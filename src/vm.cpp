#include "pocketpy/vm.h"

namespace pkpy{

    struct JsonSerializer{
        VM* vm;
        PyObject* root;
        SStream ss;

        JsonSerializer(VM* vm, PyObject* root) : vm(vm), root(root) {}

        template<typename T>
        void write_array(T& arr){
            ss << '[';
            for(int i=0; i<arr.size(); i++){
                if(i != 0) ss << ", ";
                write_object(arr[i]);
            }
            ss << ']';
        }

        void write_dict(Dict& dict){
            ss << '{';
            bool first = true;
            dict.apply([&](PyObject* k, PyObject* v){
                if(!first) ss << ", ";
                first = false;
                if(!is_non_tagged_type(k, vm->tp_str)){
                    vm->TypeError(fmt("json keys must be string, got ", obj_type_name(vm, vm->_tp(k))));
                    UNREACHABLE();
                }
                ss << _CAST(Str&, k).escape(false) << ": ";
                write_object(v);
            });
            ss << '}';
        }

        void write_object(PyObject* obj){
            Type obj_t = vm->_tp(obj);
            if(obj == vm->None){
                ss << "null";
            }else if(obj_t == vm->tp_int){
                ss << _CAST(i64, obj);
            }else if(obj_t == vm->tp_float){
                f64 val = _CAST(f64, obj);
                if(std::isinf(val) || std::isnan(val)) vm->ValueError("cannot jsonify 'nan' or 'inf'");
                ss << val;
            }else if(obj_t == vm->tp_bool){
                ss << (obj == vm->True ? "true" : "false");
            }else if(obj_t == vm->tp_str){
                _CAST(Str&, obj).escape_(ss, false);
            }else if(obj_t == vm->tp_list){
                write_array<List>(_CAST(List&, obj));
            }else if(obj_t == vm->tp_tuple){
                write_array<Tuple>(_CAST(Tuple&, obj));
            }else if(obj_t == vm->tp_dict){
                write_dict(_CAST(Dict&, obj));
            }else{
                vm->TypeError(fmt("unrecognized type ", obj_type_name(vm, obj_t).escape()));
                UNREACHABLE();
            }
        }

        Str serialize(){
            auto _lock = vm->heap.gc_scope_lock();
            write_object(root);
            return ss.str();
        }
    };

    VM::VM(bool enable_os) : heap(this), enable_os(enable_os) {
        this->vm = this;
        this->_c.error = nullptr;
        _stdout = [](VM* vm, const char* buf, int size) {
            PK_UNUSED(vm);
            std::cout.write(buf, size);
        };
        _stderr = [](VM* vm, const char* buf, int size) {
            PK_UNUSED(vm);
            std::cerr.write(buf, size);
        };
        callstack.reserve(8);
        _main = nullptr;
        _last_exception = nullptr;
        _import_handler = [](const Str& name) {
            PK_UNUSED(name);
            return Bytes();
        };
        init_builtin_types();
    }

    PyObject* VM::py_str(PyObject* obj){
        const PyTypeInfo* ti = _inst_type_info(obj);
        if(ti->m__str__) return ti->m__str__(this, obj);
        PyObject* self;
        PyObject* f = get_unbound_method(obj, __str__, &self, false);
        if(self != PY_NULL) return call_method(self, f);
        return py_repr(obj);
    }

    PyObject* VM::py_repr(PyObject* obj){
        const PyTypeInfo* ti = _inst_type_info(obj);
        if(ti->m__repr__) return ti->m__repr__(this, obj);
        return call_method(obj, __repr__);
    }

    PyObject* VM::py_json(PyObject* obj){
        auto j = JsonSerializer(this, obj);
        return VAR(j.serialize());
    }

    PyObject* VM::py_iter(PyObject* obj){
        const PyTypeInfo* ti = _inst_type_info(obj);
        if(ti->m__iter__) return ti->m__iter__(this, obj);
        PyObject* self;
        PyObject* iter_f = get_unbound_method(obj, __iter__, &self, false);
        if(self != PY_NULL) return call_method(self, iter_f);
        TypeError(OBJ_NAME(_t(obj)).escape() + " object is not iterable");
        return nullptr;
    }

    FrameId VM::top_frame(){
#if PK_DEBUG_EXTRA_CHECK
        if(callstack.empty()) FATAL_ERROR();
#endif
        return FrameId(&callstack.data(), callstack.size()-1);
    }

    void VM::_pop_frame(){
        Frame* frame = &callstack.top();
        s_data.reset(frame->_sp_base);
        callstack.pop();
    }

    PyObject* VM::find_name_in_mro(PyObject* cls, StrName name){
        PyObject* val;
        do{
            val = cls->attr().try_get(name);
            if(val != nullptr) return val;
            Type base = _all_types[PK_OBJ_GET(Type, cls)].base;
            if(base.index == -1) break;
            cls = _all_types[base].obj;
        }while(true);
        return nullptr;
    }

    bool VM::isinstance(PyObject* obj, Type base){
        Type obj_t = PK_OBJ_GET(Type, _t(obj));
        return issubclass(obj_t, base);
    }

    bool VM::issubclass(Type cls, Type base){
        do{
            if(cls == base) return true;
            Type next = _all_types[cls].base;
            if(next.index == -1) break;
            cls = next;
        }while(true);
        return false;
    }

    PyObject* VM::exec(Str source, Str filename, CompileMode mode, PyObject* _module){
        if(_module == nullptr) _module = _main;
        try {
            CodeObject_ code = compile(source, filename, mode);
#if PK_DEBUG_DIS_EXEC
            if(_module == _main) std::cout << disassemble(code) << '\n';
#endif
            return _exec(code, _module);
        }catch (const Exception& e){
            Str sum = e.summary() + "\n";
            _stderr(this, sum.data, sum.size);
        }
#if !PK_DEBUG_FULL_EXCEPTION
        catch (const std::exception& e) {
            Str msg = "An std::exception occurred! It could be a bug.\n";
            msg = msg + e.what() + "\n";
            _stderr(this, msg.data, msg.size);
        }
#endif
        callstack.clear();
        s_data.clear();
        return nullptr;
    }

    PyObject* VM::exec(Str source){
        return exec(source, "main.py", EXEC_MODE);
    }

    PyObject* VM::eval(Str source){
        return exec(source, "<eval>", EVAL_MODE);
    }

    PyObject* VM::new_type_object(PyObject* mod, StrName name, Type base, bool subclass_enabled){
        PyObject* obj = heap._new<Type>(tp_type, _all_types.size());
        const PyTypeInfo& base_info = _all_types[base];
        if(!base_info.subclass_enabled){
            TypeError(fmt("type ", base_info.name.escape(), " is not `subclass_enabled`"));
        }
        PyTypeInfo info{
            obj,
            base,
            mod,
            name.sv(),
            subclass_enabled,
        };
        if(mod != nullptr) mod->attr().set(name, obj);
        _all_types.push_back(info);
        return obj;
    }

    Type VM::_new_type_object(StrName name, Type base) {
        PyObject* obj = new_type_object(nullptr, name, base, false);
        return PK_OBJ_GET(Type, obj);
    }

    PyObject* VM::_find_type_object(const Str& type){
        PyObject* obj = builtins->attr().try_get_likely_found(type);
        if(obj == nullptr){
            for(auto& t: _all_types) if(t.name == type) return t.obj;
            throw std::runtime_error(fmt("type not found: ", type).str());
        }
        check_non_tagged_type(obj, tp_type);
        return obj;
    }


    Type VM::_type(const Str& type){
        PyObject* obj = _find_type_object(type);
        return PK_OBJ_GET(Type, obj);
    }

    PyTypeInfo* VM::_type_info(const Str& type){
        PyObject* obj = builtins->attr().try_get_likely_found(type);
        if(obj == nullptr){
            for(auto& t: _all_types) if(t.name == type) return &t;
            FATAL_ERROR();
        }
        return &_all_types[PK_OBJ_GET(Type, obj)];
    }

    PyTypeInfo* VM::_type_info(Type type){
        return &_all_types[type];
    }

    const PyTypeInfo* VM::_inst_type_info(PyObject* obj){
        if(is_int(obj)) return &_all_types[tp_int];
        if(is_float(obj)) return &_all_types[tp_float];
        return &_all_types[obj->type];
    }

    bool VM::py_eq(PyObject* lhs, PyObject* rhs){
        if(lhs == rhs) return true;
        const PyTypeInfo* ti = _inst_type_info(lhs);
        PyObject* res;
        if(ti->m__eq__){
            res = ti->m__eq__(this, lhs, rhs);
            if(res != vm->NotImplemented) return res == vm->True;
        }
        res = call_method(lhs, __eq__, rhs);
        if(res != vm->NotImplemented) return res == vm->True;

        ti = _inst_type_info(rhs);
        if(ti->m__eq__){
            res = ti->m__eq__(this, rhs, lhs);
            if(res != vm->NotImplemented) return res == vm->True;
        }
        res = call_method(rhs, __eq__, lhs);
        if(res != vm->NotImplemented) return res == vm->True;
        return false;
    }


    int VM::normalized_index(int index, int size){
        if(index < 0) index += size;
        if(index < 0 || index >= size){
            IndexError(std::to_string(index) + " not in [0, " + std::to_string(size) + ")");
        }
        return index;
    }

    PyObject* VM::py_next(PyObject* obj){
        const PyTypeInfo* ti = _inst_type_info(obj);
        if(ti->m__next__) return ti->m__next__(this, obj);
        return call_method(obj, __next__);
    }

    PyObject* VM::py_import(Str path, bool throw_err){
        if(path.empty()) vm->ValueError("empty module name");
        static auto f_join = [](const std::vector<std::string_view>& cpnts){
            SStream ss;
            for(int i=0; i<cpnts.size(); i++){
                if(i != 0) ss << ".";
                ss << cpnts[i];
            }
            return Str(ss.str());
        };

        if(path[0] == '.'){
            if(_import_context.pending.empty()){
                ImportError("relative import outside of package");
            }
            Str curr_path = _import_context.pending.back();
            bool curr_is_init = _import_context.pending_is_init.back();
            // convert relative path to absolute path
            std::vector<std::string_view> cpnts = curr_path.split('.');
            int prefix = 0;     // how many dots in the prefix
            for(int i=0; i<path.length(); i++){
                if(path[i] == '.') prefix++;
                else break;
            }
            if(prefix > cpnts.size()) ImportError("attempted relative import beyond top-level package");
            path = path.substr(prefix);     // remove prefix
            for(int i=(int)curr_is_init; i<prefix; i++) cpnts.pop_back();
            if(!path.empty()) cpnts.push_back(path.sv());
            path = f_join(cpnts);
        }

        PK_ASSERT(path.begin()[0] != '.' && path.end()[-1] != '.');

        // check existing module
        StrName name(path);
        PyObject* ext_mod = _modules.try_get(name);
        if(ext_mod != nullptr) return ext_mod;

        std::vector<std::string_view> path_cpnts = path.split('.');
        // check circular import
        if(_import_context.pending.size() > 128){
            ImportError("maximum recursion depth exceeded while importing");
        }

        // try import
        Str filename = path.replace('.', kPlatformSep) + ".py";
        Str source;
        bool is_init = false;
        auto it = _lazy_modules.find(name);
        if(it == _lazy_modules.end()){
            Bytes b = _import_handler(filename);
            if(!b){
                filename = path.replace('.', kPlatformSep).str() + kPlatformSep + "__init__.py";
                is_init = true;
                b = _import_handler(filename);
            }
            if(!b){
                if(throw_err) ImportError(fmt("module ", path.escape(), " not found"));
                else return nullptr;
            }
            source = Str(b.str());
        }else{
            source = it->second;
            _lazy_modules.erase(it);
        }
        auto _ = _import_context.scope(path, is_init);
        CodeObject_ code = compile(source, filename, EXEC_MODE);

        Str name_cpnt = path_cpnts.back();
        path_cpnts.pop_back();
        PyObject* new_mod = new_module(name_cpnt, f_join(path_cpnts));
        _exec(code, new_mod);
        return new_mod;
    }

    VM::~VM() {
        callstack.clear();
        s_data.clear();
        _all_types.clear();
        _modules.clear();
        _lazy_modules.clear();
    }

PyObject* VM::py_negate(PyObject* obj){
    const PyTypeInfo* ti = _inst_type_info(obj);
    if(ti->m__neg__) return ti->m__neg__(this, obj);
    return call_method(obj, __neg__);
}

bool VM::py_bool(PyObject* obj){
    if(obj == vm->True) return true;
    if(obj == vm->False) return false;
    if(obj == None) return false;
    if(is_int(obj)) return _CAST(i64, obj) != 0;
    if(is_float(obj)) return _CAST(f64, obj) != 0.0;
    PyObject* self;
    PyObject* len_f = get_unbound_method(obj, __len__, &self, false);
    if(self != PY_NULL){
        PyObject* ret = call_method(self, len_f);
        return CAST(i64, ret) > 0;
    }
    return true;
}

PyObject* VM::py_list(PyObject* it){
    auto _lock = heap.gc_scope_lock();
    it = py_iter(it);
    List list;
    PyObject* obj = py_next(it);
    while(obj != StopIteration){
        list.push_back(obj);
        obj = py_next(it);
    }
    return VAR(std::move(list));
}



void VM::parse_int_slice(const Slice& s, int length, int& start, int& stop, int& step){
    auto clip = [](int value, int min, int max){
        if(value < min) return min;
        if(value > max) return max;
        return value;
    };
    if(s.step == None) step = 1;
    else step = CAST(int, s.step);
    if(step == 0) ValueError("slice step cannot be zero");
    if(step > 0){
        if(s.start == None){
            start = 0;
        }else{
            start = CAST(int, s.start);
            if(start < 0) start += length;
            start = clip(start, 0, length);
        }
        if(s.stop == None){
            stop = length;
        }else{
            stop = CAST(int, s.stop);
            if(stop < 0) stop += length;
            stop = clip(stop, 0, length);
        }
    }else{
        if(s.start == None){
            start = length - 1;
        }else{
            start = CAST(int, s.start);
            if(start < 0) start += length;
            start = clip(start, -1, length - 1);
        }
        if(s.stop == None){
            stop = -1;
        }else{
            stop = CAST(int, s.stop);
            if(stop < 0) stop += length;
            stop = clip(stop, -1, length - 1);
        }
    }
}

i64 VM::py_hash(PyObject* obj){
    // https://docs.python.org/3.10/reference/datamodel.html#object.__hash__
    const PyTypeInfo* ti = _inst_type_info(obj);
    if(ti->m__hash__) return ti->m__hash__(this, obj);

    PyObject* self;
    PyObject* f = get_unbound_method(obj, __hash__, &self, false);
    if(f != nullptr){
        PyObject* ret = call_method(self, f);
        return CAST(i64, ret);
    }
    // if it is trivial `object`, return PK_BITS
    if(ti == &_all_types[tp_object]) return PK_BITS(obj);
    // otherwise, we check if it has a custom __eq__ other than object.__eq__
    bool has_custom_eq = false;
    if(ti->m__eq__) has_custom_eq = true;
    else{
        f = get_unbound_method(obj, __eq__, &self, false);
        has_custom_eq = f != _t(tp_object)->attr(__eq__);
    }
    if(has_custom_eq){
        TypeError(fmt("unhashable type: ", ti->name.escape()));
        return 0;
    }else{
        return PK_BITS(obj);
    }
}

PyObject* VM::format(Str spec, PyObject* obj){
    if(spec.empty()) return py_str(obj);
    char type;
    switch(spec.end()[-1]){
        case 'f': case 'd': case 's':
            type = spec.end()[-1];
            spec = spec.substr(0, spec.length() - 1);
            break;
        default: type = ' '; break;
    }

    char pad_c = ' ';
    for(char c: std::string_view("0-=*#@!~")){
        if(spec[0] == c){
            pad_c = c;
            spec = spec.substr(1);
            break;
        }
    }
    char align;
    if(spec[0] == '^'){
        align = '^';
        spec = spec.substr(1);
    }else if(spec[0] == '>'){
        align = '>';
        spec = spec.substr(1);
    }else if(spec[0] == '<'){
        align = '<';
        spec = spec.substr(1);
    }else{
        if(is_int(obj) || is_float(obj)) align = '>';
        else align = '<';
    }

    int dot = spec.index(".");
    int width, precision;
    try{
        if(dot >= 0){
            if(dot == 0){
                width = -1;
            }else{
                width = std::stoi(spec.substr(0, dot).str());
            }
            precision = std::stoi(spec.substr(dot+1).str());
        }else{
            width = std::stoi(spec.str());
            precision = -1;
        }
    }catch(...){
        ValueError("invalid format specifer");
        UNREACHABLE();
    }

    if(type != 'f' && dot >= 0) ValueError("precision not allowed in the format specifier");
    Str ret;
    if(type == 'f'){
        f64 val = CAST(f64, obj);
        if(precision < 0) precision = 6;
        std::stringstream ss; // float
        ss << std::fixed << std::setprecision(precision) << val;
        ret = ss.str();
    }else if(type == 'd'){
        ret = std::to_string(CAST(i64, obj));
    }else if(type == 's'){
        ret = CAST(Str&, obj);
    }else{
        ret = CAST(Str&, py_str(obj));
    }
    if(width != -1 && width > ret.length()){
        int pad = width - ret.length();
        if(align == '>' || align == '<'){
            std::string padding(pad, pad_c);
            if(align == '>') ret = padding.c_str() + ret;
            else ret = ret + padding.c_str();
        }else{  // ^
            int pad_left = pad / 2;
            int pad_right = pad - pad_left;
            std::string padding_left(pad_left, pad_c);
            std::string padding_right(pad_right, pad_c);
            ret = padding_left.c_str() + ret + padding_right.c_str();
        }
    }
    return VAR(ret);
}

PyObject* VM::new_module(Str name, Str package) {
    PyObject* obj = heap._new<DummyModule>(tp_module);
    obj->attr().set(__name__, VAR(name));
    obj->attr().set(__package__, VAR(package));
    // we do not allow override in order to avoid memory leak
    // it is because Module objects are not garbage collected
    if(_modules.contains(name)){
        throw std::runtime_error(fmt("module ", name.escape(), " already exists").str());
    }
    // convert to fullname and set it into _modules
    if(!package.empty()) name = package + "." + name;
    obj->attr().set(__path__, VAR(name));
    _modules.set(name, obj);
    return obj;
}

static std::string _opcode_argstr(VM* vm, Bytecode byte, const CodeObject* co){
    std::string argStr = byte.arg == BC_NOARG ? "" : std::to_string(byte.arg);
    switch(byte.op){
        case OP_LOAD_CONST: case OP_FORMAT_STRING: case OP_IMPORT_PATH:
            if(vm != nullptr){
                argStr += fmt(" (", CAST(Str, vm->py_repr(co->consts[byte.arg])), ")").sv();
            }
            break;
        case OP_LOAD_NAME: case OP_LOAD_GLOBAL: case OP_LOAD_NONLOCAL: case OP_STORE_GLOBAL:
        case OP_LOAD_ATTR: case OP_LOAD_METHOD: case OP_STORE_ATTR: case OP_DELETE_ATTR:
        case OP_BEGIN_CLASS: case OP_RAISE: case OP_GOTO:
        case OP_DELETE_GLOBAL: case OP_INC_GLOBAL: case OP_DEC_GLOBAL: case OP_STORE_CLASS_ATTR:
            argStr += fmt(" (", StrName(byte.arg).sv(), ")").sv();
            break;
        case OP_LOAD_FAST: case OP_STORE_FAST: case OP_DELETE_FAST: case OP_INC_FAST: case OP_DEC_FAST:
            argStr += fmt(" (", co->varnames[byte.arg].sv(), ")").sv();
            break;
        case OP_LOAD_FUNCTION:
            argStr += fmt(" (", co->func_decls[byte.arg]->code->name, ")").sv();
            break;
    }
    return argStr;
}

Str VM::disassemble(CodeObject_ co){
    auto pad = [](const Str& s, const int n){
        if(s.length() >= n) return s.substr(0, n);
        return s + std::string(n - s.length(), ' ');
    };

    std::vector<int> jumpTargets;
    for(auto byte : co->codes){
        if(byte.op == OP_JUMP_ABSOLUTE || byte.op == OP_POP_JUMP_IF_FALSE || byte.op == OP_SHORTCUT_IF_FALSE_OR_POP){
            jumpTargets.push_back(byte.arg);
        }
        if(byte.op == OP_GOTO){
            // TODO: pre-compute jump targets for OP_GOTO
            int* target = co->labels.try_get_2_likely_found(StrName(byte.arg));
            if(target != nullptr) jumpTargets.push_back(*target);
        }
    }
    SStream ss;
    int prev_line = -1;
    for(int i=0; i<co->codes.size(); i++){
        const Bytecode& byte = co->codes[i];
        Str line = std::to_string(co->lines[i]);
        if(co->lines[i] == prev_line) line = "";
        else{
            if(prev_line != -1) ss << "\n";
            prev_line = co->lines[i];
        }

        std::string pointer;
        if(std::find(jumpTargets.begin(), jumpTargets.end(), i) != jumpTargets.end()){
            pointer = "-> ";
        }else{
            pointer = "   ";
        }
        ss << pad(line, 8) << pointer << pad(std::to_string(i), 3);
        ss << " " << pad(OP_NAMES[byte.op], 25) << " ";
        // ss << pad(byte.arg == -1 ? "" : std::to_string(byte.arg), 5);
        std::string argStr = _opcode_argstr(this, byte, co.get());
        ss << argStr;
        // ss << pad(argStr, 40);      // may overflow
        // ss << co->blocks[byte.block].type;
        if(i != co->codes.size() - 1) ss << '\n';
    }

    for(auto& decl: co->func_decls){
        ss << "\n\n" << "Disassembly of " << decl->code->name << ":\n";
        ss << disassemble(decl->code);
    }
    ss << "\n";
    return Str(ss.str());
}

#if PK_DEBUG_CEVAL_STEP
void VM::_log_s_data(const char* title) {
    if(_main == nullptr) return;
    if(callstack.empty()) return;
    SStream ss;
    if(title) ss << title << " | ";
    std::map<PyObject**, int> sp_bases;
    for(Frame& f: callstack.data()){
        if(f._sp_base == nullptr) FATAL_ERROR();
        sp_bases[f._sp_base] += 1;
    }
    FrameId frame = top_frame();
    int line = frame->co->lines[frame->_ip];
    ss << frame->co->name << ":" << line << " [";
    for(PyObject** p=s_data.begin(); p!=s_data.end(); p++){
        ss << std::string(sp_bases[p], '|');
        if(sp_bases[p] > 0) ss << " ";
        PyObject* obj = *p;
        if(obj == nullptr) ss << "(nil)";
        else if(obj == PY_NULL) ss << "NULL";
        else if(is_int(obj)) ss << CAST(i64, obj);
        else if(is_float(obj)) ss << CAST(f64, obj);
        else if(is_type(obj, tp_str)) ss << CAST(Str, obj).escape();
        else if(obj == None) ss << "None";
        else if(obj == True) ss << "True";
        else if(obj == False) ss << "False";
        else if(is_type(obj, tp_function)){
            auto& f = CAST(Function&, obj);
            ss << f.decl->code->name << "(...)";
        } else if(is_type(obj, tp_type)){
            Type t = PK_OBJ_GET(Type, obj);
            ss << "<class " + _all_types[t].name.escape() + ">";
        } else if(is_type(obj, tp_list)){
            auto& t = CAST(List&, obj);
            ss << "list(size=" << t.size() << ")";
        } else if(is_type(obj, tp_tuple)){
            auto& t = CAST(Tuple&, obj);
            ss << "tuple(size=" << t.size() << ")";
        } else ss << "(" << obj_type_name(this, obj->type) << ")";
        ss << ", ";
    }
    std::string output = ss.str();
    if(!s_data.empty()) {
        output.pop_back(); output.pop_back();
    }
    output.push_back(']');
    Bytecode byte = frame->co->codes[frame->_ip];
    std::cout << output << " " << OP_NAMES[byte.op] << " " << _opcode_argstr(nullptr, byte, frame->co) << std::endl;
}
#endif

void VM::init_builtin_types(){
    _all_types.push_back({heap._new<Type>(Type(1), Type(0)), -1, nullptr, "object", true});
    _all_types.push_back({heap._new<Type>(Type(1), Type(1)), 0, nullptr, "type", false});
    tp_object = 0; tp_type = 1;

    tp_int = _new_type_object("int");
    tp_float = _new_type_object("float");
    if(tp_int.index != kTpIntIndex || tp_float.index != kTpFloatIndex) FATAL_ERROR();

    tp_bool = _new_type_object("bool");
    tp_str = _new_type_object("str");
    tp_list = _new_type_object("list");
    tp_tuple = _new_type_object("tuple");
    tp_slice = _new_type_object("slice");
    tp_range = _new_type_object("range");
    tp_module = _new_type_object("module");
    tp_function = _new_type_object("function");
    tp_native_func = _new_type_object("native_func");
    tp_bound_method = _new_type_object("bound_method");
    tp_super = _new_type_object("super");
    tp_exception = _new_type_object("Exception");
    tp_bytes = _new_type_object("bytes");
    tp_mappingproxy = _new_type_object("mappingproxy");
    tp_dict = _new_type_object("dict");
    tp_property = _new_type_object("property");
    tp_star_wrapper = _new_type_object("_star_wrapper");

    this->None = heap._new<Dummy>(_new_type_object("NoneType"));
    this->NotImplemented = heap._new<Dummy>(_new_type_object("NotImplementedType"));
    this->Ellipsis = heap._new<Dummy>(_new_type_object("ellipsis"));
    this->True = heap._new<Dummy>(tp_bool);
    this->False = heap._new<Dummy>(tp_bool);
    this->StopIteration = heap._new<Dummy>(_new_type_object("StopIterationType"));

    this->builtins = new_module("builtins");
    
    // setup public types
    builtins->attr().set("type", _t(tp_type));
    builtins->attr().set("object", _t(tp_object));
    builtins->attr().set("bool", _t(tp_bool));
    builtins->attr().set("int", _t(tp_int));
    builtins->attr().set("float", _t(tp_float));
    builtins->attr().set("str", _t(tp_str));
    builtins->attr().set("list", _t(tp_list));
    builtins->attr().set("tuple", _t(tp_tuple));
    builtins->attr().set("range", _t(tp_range));
    builtins->attr().set("bytes", _t(tp_bytes));
    builtins->attr().set("dict", _t(tp_dict));
    builtins->attr().set("property", _t(tp_property));
    builtins->attr().set("StopIteration", StopIteration);
    builtins->attr().set("NotImplemented", NotImplemented);
    builtins->attr().set("slice", _t(tp_slice));

    post_init();
    this->_main = new_module("__main__");
}

// `heap.gc_scope_lock();` needed before calling this function
void VM::_unpack_as_list(ArgsView args, List& list){
    for(PyObject* obj: args){
        if(is_non_tagged_type(obj, tp_star_wrapper)){
            const StarWrapper& w = _CAST(StarWrapper&, obj);
            // maybe this check should be done in the compile time
            if(w.level != 1) TypeError("expected level 1 star wrapper");
            PyObject* _0 = py_iter(w.obj);
            PyObject* _1 = py_next(_0);
            while(_1 != StopIteration){
                list.push_back(_1);
                _1 = py_next(_0);
            }
        }else{
            list.push_back(obj);
        }
    }
}

// `heap.gc_scope_lock();` needed before calling this function
void VM::_unpack_as_dict(ArgsView args, Dict& dict){
    for(PyObject* obj: args){
        if(is_non_tagged_type(obj, tp_star_wrapper)){
            const StarWrapper& w = _CAST(StarWrapper&, obj);
            // maybe this check should be done in the compile time
            if(w.level != 2) TypeError("expected level 2 star wrapper");
            const Dict& other = CAST(Dict&, w.obj);
            dict.update(other);
        }else{
            const Tuple& t = CAST(Tuple&, obj);
            if(t.size() != 2) TypeError("expected tuple of length 2");
            dict.set(t[0], t[1]);
        }
    }
}


void VM::_prepare_py_call(PyObject** buffer, ArgsView args, ArgsView kwargs, const FuncDecl_& decl){
    const CodeObject* co = decl->code.get();
    int co_nlocals = co->varnames.size();
    int decl_argc = decl->args.size();

    if(args.size() < decl_argc){
        vm->TypeError(fmt(
            co->name, "() takes ", decl_argc, " positional arguments but ", args.size(), " were given"
        ));
        UNREACHABLE();
    }

    int i = 0;
    // prepare args
    for(int index: decl->args) buffer[index] = args[i++];
    // set extra varnames to PY_NULL
    for(int j=i; j<co_nlocals; j++) buffer[j] = PY_NULL;
    // prepare kwdefaults
    for(auto& kv: decl->kwargs) buffer[kv.key] = kv.value;
    
    // handle *args
    if(decl->starred_arg != -1){
        ArgsView vargs(args.begin() + i, args.end());
        buffer[decl->starred_arg] = VAR(vargs.to_tuple());
        i += vargs.size();
    }else{
        // kwdefaults override
        for(auto& kv: decl->kwargs){
            if(i >= args.size()) break;
            buffer[kv.key] = args[i++];
        }
        if(i < args.size()) TypeError(fmt("too many arguments", " (", decl->code->name, ')'));
    }
    
    PyObject* vkwargs;
    if(decl->starred_kwarg != -1){
        vkwargs = VAR(Dict(this));
        buffer[decl->starred_kwarg] = vkwargs;
    }else{
        vkwargs = nullptr;
    }

    for(int j=0; j<kwargs.size(); j+=2){
        StrName key(CAST(int, kwargs[j]));
        int index = co->varnames_inv.try_get_likely_found(key);
        if(index < 0){
            if(vkwargs == nullptr){
                TypeError(fmt(key.escape(), " is an invalid keyword argument for ", co->name, "()"));
            }else{
                Dict& dict = _CAST(Dict&, vkwargs);
                dict.set(VAR(key.sv()), kwargs[j+1]);
            }
        }else{
            buffer[index] = kwargs[j+1];
        }
    }
}

PyObject* VM::vectorcall(int ARGC, int KWARGC, bool op_call){
    PyObject** p1 = s_data._sp - KWARGC*2;
    PyObject** p0 = p1 - ARGC - 2;
    // [callable, <self>, args..., kwargs...]
    //      ^p0                    ^p1      ^_sp
    PyObject* callable = p1[-(ARGC + 2)];
    bool method_call = p1[-(ARGC + 1)] != PY_NULL;

    // handle boundmethod, do a patch
    if(is_non_tagged_type(callable, tp_bound_method)){
        if(method_call) FATAL_ERROR();
        auto& bm = CAST(BoundMethod&, callable);
        callable = bm.func;      // get unbound method
        p1[-(ARGC + 2)] = bm.func;
        p1[-(ARGC + 1)] = bm.self;
        method_call = true;
        // [unbound, self, args..., kwargs...]
    }

    ArgsView args(p1 - ARGC - int(method_call), p1);
    ArgsView kwargs(p1, s_data._sp);

    PyObject** _base = args.begin();
    PyObject* buffer[PK_MAX_CO_VARNAMES];

    if(is_non_tagged_type(callable, tp_native_func)){
        const auto& f = PK_OBJ_GET(NativeFunc, callable);
        PyObject* ret;
        if(f.decl != nullptr){
            int co_nlocals = f.decl->code->varnames.size();
            _prepare_py_call(buffer, args, kwargs, f.decl);
            // copy buffer back to stack
            s_data.reset(_base + co_nlocals);
            for(int j=0; j<co_nlocals; j++) _base[j] = buffer[j];
            ret = f.call(vm, ArgsView(s_data._sp - co_nlocals, s_data._sp));
        }else{
            if(KWARGC != 0) TypeError("old-style native_func does not accept keyword arguments");
            f.check_size(this, args);
            ret = f.call(this, args);
        }
        s_data.reset(p0);
        return ret;
    }

    if(is_non_tagged_type(callable, tp_function)){
        /*****************_py_call*****************/
        // callable must be a `function` object
        if(s_data.is_overflow()) StackOverflowError();

        const Function& fn = PK_OBJ_GET(Function, callable);
        const FuncDecl_& decl = fn.decl;
        const CodeObject* co = decl->code.get();
        int co_nlocals = co->varnames.size();
        if(decl->is_simple){
            if(args.size() != decl->args.size()){
                TypeError(fmt(
                    co->name, "() takes ", decl->args.size(), " positional arguments but ", args.size(), " were given"
                ));
                UNREACHABLE();
            }
            if(!kwargs.empty()){
                TypeError(fmt(co->name, "() takes no keyword arguments"));
                UNREACHABLE();
            }
            s_data.reset(_base + co_nlocals);
            int i = 0;
            // prepare args
            for(int index: decl->args) _base[index] = args[i++];
            // set extra varnames to PY_NULL
            for(int j=i; j<co_nlocals; j++) _base[j] = PY_NULL;
            goto __FAST_CALL;
        }

        _prepare_py_call(buffer, args, kwargs, decl);
        
        if(co->is_generator){
            s_data.reset(p0);
            return _py_generator(
                Frame(&s_data, nullptr, co, fn._module, callable),
                ArgsView(buffer, buffer + co_nlocals)
            );
        }

        // copy buffer back to stack
        s_data.reset(_base + co_nlocals);
        for(int j=0; j<co_nlocals; j++) _base[j] = buffer[j];

__FAST_CALL:
        callstack.emplace(&s_data, p0, co, fn._module, callable, FastLocals(co, args.begin()));
        if(op_call) return PY_OP_CALL;
        return _run_top_frame();
        /*****************_py_call*****************/
    }

    if(is_non_tagged_type(callable, tp_type)){
        if(method_call) FATAL_ERROR();
        // [type, NULL, args..., kwargs...]
        PyObject* new_f = find_name_in_mro(callable, __new__);
        PyObject* obj;
#if PK_DEBUG_EXTRA_CHECK
        PK_ASSERT(new_f != nullptr);
#endif
        if(new_f == cached_object__new__) {
            // fast path for object.__new__
            Type t = PK_OBJ_GET(Type, callable);
            obj= vm->heap.gcnew<DummyInstance>(t);
        }else{
            PUSH(new_f);
            PUSH(PY_NULL);
            PUSH(callable);    // cls
            for(PyObject* o: args) PUSH(o);
            for(PyObject* o: kwargs) PUSH(o);
            // if obj is not an instance of callable, the behavior is undefined
            obj = vectorcall(ARGC+1, KWARGC);
        }

        // __init__
        PyObject* self;
        callable = get_unbound_method(obj, __init__, &self, false);
        if (self != PY_NULL) {
            // replace `NULL` with `self`
            p1[-(ARGC + 2)] = callable;
            p1[-(ARGC + 1)] = self;
            // [init_f, self, args..., kwargs...]
            vectorcall(ARGC, KWARGC);
            // We just discard the return value of `__init__`
            // in cpython it raises a TypeError if the return value is not None
        }else{
            // manually reset the stack
            s_data.reset(p0);
        }
        return obj;
    }

    // handle `__call__` overload
    PyObject* self;
    PyObject* call_f = get_unbound_method(callable, __call__, &self, false);
    if(self != PY_NULL){
        p1[-(ARGC + 2)] = call_f;
        p1[-(ARGC + 1)] = self;
        // [call_f, self, args..., kwargs...]
        return vectorcall(ARGC, KWARGC, false);
    }
    TypeError(OBJ_NAME(_t(callable)).escape() + " object is not callable");
    return nullptr;
}

void VM::delattr(PyObject *_0, StrName _name){
    if(is_tagged(_0) || !_0->is_attr_valid()) TypeError("cannot delete attribute");
    if(!_0->attr().del(_name)) AttributeError(_0, _name);
}

// https://docs.python.org/3/howto/descriptor.html#invocation-from-an-instance
PyObject* VM::getattr(PyObject* obj, StrName name, bool throw_err){
    PyObject* objtype;
    // handle super() proxy
    if(is_non_tagged_type(obj, tp_super)){
        const Super& super = PK_OBJ_GET(Super, obj);
        obj = super.first;
        objtype = _t(super.second);
    }else{
        objtype = _t(obj);
    }
    PyObject* cls_var = find_name_in_mro(objtype, name);
    if(cls_var != nullptr){
        // handle descriptor
        if(is_non_tagged_type(cls_var, tp_property)){
            const Property& prop = _CAST(Property&, cls_var);
            return call(prop.getter, obj);
        }
    }
    // handle instance __dict__
    if(!is_tagged(obj) && obj->is_attr_valid()){
        PyObject* val = obj->attr().try_get_likely_found(name);
        if(val != nullptr) return val;
    }
    if(cls_var != nullptr){
        // bound method is non-data descriptor
        if(is_non_tagged_type(cls_var, tp_function) || is_non_tagged_type(cls_var, tp_native_func)){
            return VAR(BoundMethod(obj, cls_var));
        }
        return cls_var;
    }
    
    if(is_non_tagged_type(obj, tp_module)){
        Str path = CAST(Str&, obj->attr(__path__));
        path = path + "." + name.sv();
        PyObject* mod = py_import(path, false);
        if(mod != nullptr){
            obj->attr().set(name, mod);
            return mod;
        }
    }

    if(throw_err) AttributeError(obj, name);
    return nullptr;
}

// used by OP_LOAD_METHOD
// try to load a unbound method (fallback to `getattr` if not found)
PyObject* VM::get_unbound_method(PyObject* obj, StrName name, PyObject** self, bool throw_err, bool fallback){
    *self = PY_NULL;
    PyObject* objtype;
    // handle super() proxy
    if(is_non_tagged_type(obj, tp_super)){
        const Super& super = PK_OBJ_GET(Super, obj);
        obj = super.first;
        objtype = _t(super.second);
    }else{
        objtype = _t(obj);
    }
    PyObject* cls_var = find_name_in_mro(objtype, name);

    if(fallback){
        if(cls_var != nullptr){
            // handle descriptor
            if(is_non_tagged_type(cls_var, tp_property)){
                const Property& prop = _CAST(Property&, cls_var);
                return call(prop.getter, obj);
            }
        }
        // handle instance __dict__
        if(!is_tagged(obj) && obj->is_attr_valid()){
            PyObject* val = obj->attr().try_get(name);
            if(val != nullptr) return val;
        }
    }

    if(cls_var != nullptr){
        if(is_non_tagged_type(cls_var, tp_function) || is_non_tagged_type(cls_var, tp_native_func)){
            *self = obj;
        }
        return cls_var;
    }
    if(throw_err) AttributeError(obj, name);
    return nullptr;
}

void VM::setattr(PyObject* obj, StrName name, PyObject* value){
    PyObject* objtype;
    // handle super() proxy
    if(is_non_tagged_type(obj, tp_super)){
        Super& super = PK_OBJ_GET(Super, obj);
        obj = super.first;
        objtype = _t(super.second);
    }else{
        objtype = _t(obj);
    }
    PyObject* cls_var = find_name_in_mro(objtype, name);
    if(cls_var != nullptr){
        // handle descriptor
        if(is_non_tagged_type(cls_var, tp_property)){
            const Property& prop = _CAST(Property&, cls_var);
            if(prop.setter != vm->None){
                call(prop.setter, obj, value);
            }else{
                TypeError(fmt("readonly attribute: ", name.escape()));
            }
            return;
        }
    }
    // handle instance __dict__
    if(is_tagged(obj) || !obj->is_attr_valid()) TypeError("cannot set attribute");
    obj->attr().set(name, value);
}

PyObject* VM::bind(PyObject* obj, const char* sig, NativeFuncC fn, UserData userdata){
    return bind(obj, sig, nullptr, fn, userdata);
}

PyObject* VM::bind(PyObject* obj, const char* sig, const char* docstring, NativeFuncC fn, UserData userdata){
    CodeObject_ co;
    try{
        // fn(a, b, *c, d=1) -> None
        co = compile("def " + Str(sig) + " : pass", "<bind>", EXEC_MODE);
    }catch(Exception&){
        throw std::runtime_error("invalid signature: " + std::string(sig));
    }
    if(co->func_decls.size() != 1){
        throw std::runtime_error("expected 1 function declaration");
    }
    FuncDecl_ decl = co->func_decls[0];
    decl->signature = Str(sig);
    if(docstring != nullptr){
        decl->docstring = Str(docstring).strip();
    }
    PyObject* f_obj = VAR(NativeFunc(fn, decl));
    PK_OBJ_GET(NativeFunc, f_obj).set_userdata(userdata);
    if(obj != nullptr) obj->attr().set(decl->code->name, f_obj);
    return f_obj;
}

PyObject* VM::bind_property(PyObject* obj, Str name, NativeFuncC fget, NativeFuncC fset){
    PyObject* _0 = heap.gcnew<NativeFunc>(tp_native_func, fget, 1, false);
    PyObject* _1 = vm->None;
    if(fset != nullptr) _1 = heap.gcnew<NativeFunc>(tp_native_func, fset, 2, false);
    Str signature = name;
    int pos = name.index(":");
    if(pos > 0) name = name.substr(0, pos).strip();
    PyObject* prop = VAR(Property(_0, _1, signature));
    obj->attr().set(name, prop);
    return prop;
}

void VM::_error(Exception e){
    if(callstack.empty()){
        e.is_re = false;
        throw e;
    }
    PUSH(VAR(std::move(e)));
    _raise();
}

void VM::_raise(bool re_raise){
    Frame* frame = top_frame().get();
    Exception& e = PK_OBJ_GET(Exception, s_data.top());
    if(!re_raise){
        e._ip_on_error = frame->_ip;
        e._code_on_error = (void*)frame->co;
    }
    bool ok = frame->jump_to_exception_handler();

    int actual_ip = frame->_ip;
    if(e._ip_on_error >= 0 && e._code_on_error == (void*)frame->co) actual_ip = e._ip_on_error;
    int current_line = frame->co->lines[actual_ip];         // current line
    auto current_f_name = frame->co->name.sv();             // current function name
    if(frame->_callable == nullptr) current_f_name = "";    // not in a function
    e.st_push(frame->co->src, current_line, nullptr, current_f_name);

    if(ok) throw HandledException();
    else throw UnhandledException();
}

void ManagedHeap::mark() {
    for(PyObject* obj: _no_gc) PK_OBJ_MARK(obj);
    for(auto& frame : vm->callstack.data()) frame._gc_mark();
    for(PyObject* obj: vm->s_data) PK_OBJ_MARK(obj);
    if(_gc_marker_ex) _gc_marker_ex(vm);
    if(vm->_last_exception) PK_OBJ_MARK(vm->_last_exception);
    if(vm->_curr_class) PK_OBJ_MARK(vm->_curr_class);
    if(vm->_c.error != nullptr) PK_OBJ_MARK(vm->_c.error);
}

Str obj_type_name(VM *vm, Type type){
    return vm->_all_types[type].name;
}


void VM::bind__hash__(Type type, i64 (*f)(VM*, PyObject*)){
    PyObject* obj = _t(type);
    _all_types[type].m__hash__ = f;
    PyObject* nf = bind_method<0>(obj, "__hash__", [](VM* vm, ArgsView args){
        i64 ret = lambda_get_userdata<i64(*)(VM*, PyObject*)>(args.begin())(vm, args[0]);
        return VAR(ret);
    });
    PK_OBJ_GET(NativeFunc, nf).set_userdata(f);
}

void VM::bind__len__(Type type, i64 (*f)(VM*, PyObject*)){
    PyObject* obj = _t(type);
    _all_types[type].m__len__ = f;
    PyObject* nf = bind_method<0>(obj, "__len__", [](VM* vm, ArgsView args){
        i64 ret = lambda_get_userdata<i64(*)(VM*, PyObject*)>(args.begin())(vm, args[0]);
        return VAR(ret);
    });
    PK_OBJ_GET(NativeFunc, nf).set_userdata(f);
}

void Dict::_probe_0(PyObject *key, bool &ok, int &i) const{
    ok = false;
    i64 hash = vm->py_hash(key);
    i = hash & _mask;
    // std::cout << CAST(Str, vm->py_repr(key)) << " " << hash << " " << i << std::endl;
    for(int j=0; j<_capacity; j++) {
        if(_items[i].first != nullptr){
            if(vm->py_eq(_items[i].first, key)) { ok = true; break; }
        }else{
            if(_items[i].second == nullptr) break;
        }
        // https://github.com/python/cpython/blob/3.8/Objects/dictobject.c#L166
        i = ((5*i) + 1) & _mask;
        // std::cout << CAST(Str, vm->py_repr(key)) << " next: " << i << std::endl;
    }
}

void Dict::_probe_1(PyObject *key, bool &ok, int &i) const{
    ok = false;
    i = vm->py_hash(key) & _mask;
    while(_items[i].first != nullptr) {
        if(vm->py_eq(_items[i].first, key)) { ok = true; break; }
        // https://github.com/python/cpython/blob/3.8/Objects/dictobject.c#L166
        i = ((5*i) + 1) & _mask;
    }
}

// void CodeObjectSerializer::write_object(VM *vm, PyObject *obj){
//     if(is_int(obj)) write_int(_CAST(i64, obj));
//     else if(is_float(obj)) write_float(_CAST(f64, obj));
//     else if(is_type(obj, vm->tp_str)) write_str(_CAST(Str&, obj));
//     else if(is_type(obj, vm->tp_bool)) write_bool(_CAST(bool, obj));
//     else if(obj == vm->None) write_none();
//     else if(obj == vm->Ellipsis) write_ellipsis();
//     else{
//         throw std::runtime_error(fmt(OBJ_NAME(vm->_t(obj)).escape(), " is not serializable"));
//     }
// }

void NativeFunc::check_size(VM* vm, ArgsView args) const{
    if(args.size() != argc && argc != -1) {
        vm->TypeError(fmt("expected ", argc, " arguments, got ", args.size()));
    }
}

PyObject* NativeFunc::call(VM *vm, ArgsView args) const {
    return f(vm, args);
}

}   // namespace pkpy