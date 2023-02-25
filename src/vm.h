#pragma once

#include "frame.h"
#include "error.h"

#define DEF_NATIVE(type, ctype, ptype)                          \
    inline ctype& Py##type##_AS_C(const PyVar& obj) {           \
        check_type(obj, ptype);                                 \
        return OBJ_GET(ctype, obj);                             \
    }                                                           \
    inline PyVar Py##type(const ctype& value) { return new_object(ptype, value);} \
    inline PyVar Py##type(ctype&& value) { return new_object(ptype, std::move(value));}

class Generator: public BaseIter {
    std::unique_ptr<Frame> frame;
    int state; // 0,1,2
public:
    Generator(VM* vm, std::unique_ptr<Frame>&& frame)
        : BaseIter(vm, nullptr), frame(std::move(frame)), state(0) {}

    PyVar next();
};

class VM {
public:
    std::stack< std::unique_ptr<Frame> > callstack;
    PyVar _py_op_call;
    PyVar _py_op_yield;
    std::vector<PyVar> _all_types;
    // PyVar _ascii_str_pool[128];

    PyVar run_frame(Frame* frame);

    pkpy::NameDict _types;
    pkpy::NameDict _modules;                            // loaded modules
    std::map<StrName, Str> _lazy_modules;       // lazy loaded modules
    PyVar None, True, False, Ellipsis;

    bool use_stdio;
    std::ostream* _stdout;
    std::ostream* _stderr;
    
    PyVar builtins;         // builtins module
    PyVar _main;            // __main__ module

    int recursionlimit = 1000;

    VM(bool use_stdio){
        this->use_stdio = use_stdio;
        if(use_stdio){
            this->_stdout = &std::cout;
            this->_stderr = &std::cerr;
        }else{
            this->_stdout = new StrStream();
            this->_stderr = new StrStream();
        }

        init_builtin_types();
        // for(int i=0; i<128; i++) _ascii_str_pool[i] = new_object(tp_str, std::string(1, (char)i));
    }

    PyVar asStr(const PyVar& obj){
        PyVarOrNull f = getattr(obj, __str__, false);
        if(f != nullptr) return call(f);
        return asRepr(obj);
    }

    inline Frame* top_frame() const {
        if(callstack.empty()) UNREACHABLE();
        return callstack.top().get();
    }

    PyVar asRepr(const PyVar& obj){
        if(is_type(obj, tp_type)) return PyStr("<class '" + OBJ_GET(Str, obj->attr(__name__)) + "'>");
        return call(obj, __repr__);
    }

    const PyVar& asBool(const PyVar& obj){
        if(is_type(obj, tp_bool)) return obj;
        if(obj == None) return False;
        if(is_type(obj, tp_int)) return PyBool(PyInt_AS_C(obj) != 0);
        if(is_type(obj, tp_float)) return PyBool(PyFloat_AS_C(obj) != 0.0);
        PyVarOrNull len_fn = getattr(obj, __len__, false);
        if(len_fn != nullptr){
            PyVar ret = call(len_fn);
            return PyBool(PyInt_AS_C(ret) > 0);
        }
        return True;
    }

    PyVar asIter(const PyVar& obj){
        if(is_type(obj, tp_native_iterator)) return obj;
        PyVarOrNull iter_f = getattr(obj, __iter__, false);
        if(iter_f != nullptr) return call(iter_f);
        TypeError(OBJ_NAME(_t(obj)).escape(true) + " object is not iterable");
        return nullptr;
    }

    PyVar asList(const PyVar& iterable){
        if(is_type(iterable, tp_list)) return iterable;
        return call(_t(tp_list), pkpy::one_arg(iterable));
    }

    PyVar fast_call(StrName name, pkpy::Args&& args){
        PyObject* cls = _t(args[0]).get();
        while(cls != None.get()) {
            PyVar* val = cls->attr().try_get(name);
            if(val != nullptr) return call(*val, std::move(args));
            cls = cls->attr(__base__).get();
        }
        AttributeError(args[0], name);
        return nullptr;
    }

    inline PyVar call(const PyVar& _callable){
        return call(_callable, pkpy::no_arg(), pkpy::no_arg(), false);
    }

    template<typename ArgT>
    inline std::enable_if_t<std::is_same_v<RAW(ArgT), pkpy::Args>, PyVar>
    call(const PyVar& _callable, ArgT&& args){
        return call(_callable, std::forward<ArgT>(args), pkpy::no_arg(), false);
    }

    template<typename ArgT>
    inline std::enable_if_t<std::is_same_v<RAW(ArgT), pkpy::Args>, PyVar>
    call(const PyVar& obj, const StrName name, ArgT&& args){
        return call(getattr(obj, name), std::forward<ArgT>(args), pkpy::no_arg(), false);
    }

    inline PyVar call(const PyVar& obj, StrName name){
        return call(getattr(obj, name), pkpy::no_arg(), pkpy::no_arg(), false);
    }

    PyVar call(const PyVar& _callable, pkpy::Args args, const pkpy::Args& kwargs, bool opCall){
        if(is_type(_callable, tp_type)){
            PyVar* new_f = _callable->attr().try_get(__new__);
            PyVar obj;
            if(new_f != nullptr){
                obj = call(*new_f, std::move(args), kwargs, false);
            }else{
                obj = new_object(_callable, DummyInstance());
                PyVarOrNull init_f = getattr(obj, __init__, false);
                if (init_f != nullptr) call(init_f, std::move(args), kwargs, false);
            }
            return obj;
        }

        const PyVar* callable = &_callable;
        if(is_type(*callable, tp_bound_method)){
            auto& bm = PyBoundMethod_AS_C((*callable));
            callable = &bm.method;      // get unbound method
            args.extend_self(bm.obj);
        }
        
        if(is_type(*callable, tp_native_function)){
            const auto& f = OBJ_GET(pkpy::NativeFunc, *callable);
            if(kwargs.size() != 0) TypeError("native_function does not accept keyword arguments");
            return f(this, args);
        } else if(is_type(*callable, tp_function)){
            const pkpy::Function& fn = PyFunction_AS_C(*callable);
            NameDict_ locals = pkpy::make_shared<pkpy::NameDict>(
                fn.code->perfect_locals_capacity,
                kLocalsLoadFactor,
                fn.code->perfect_hash_seed
            );

            int i = 0;
            for(StrName name : fn.args){
                if(i < args.size()){
                    locals->set(name, std::move(args[i++]));
                    continue;
                }
                TypeError("missing positional argument " + name.str().escape(true));
            }

            locals->update(fn.kwargs);

            if(!fn.starred_arg.empty()){
                pkpy::List vargs;        // handle *args
                while(i < args.size()) vargs.push_back(std::move(args[i++]));
                locals->set(fn.starred_arg, PyTuple(std::move(vargs)));
            }else{
                for(StrName key : fn.kwargs_order){
                    if(i < args.size()){
                        locals->set(key, std::move(args[i++]));
                    }else{
                        break;
                    }
                }
                if(i < args.size()) TypeError("too many arguments");
            }
            
            for(int i=0; i<kwargs.size(); i+=2){
                const Str& key = PyStr_AS_C(kwargs[i]);
                if(!fn.kwargs.contains(key)){
                    TypeError(key.escape(true) + " is an invalid keyword argument for " + fn.name.str() + "()");
                }
                locals->set(key, kwargs[i+1]);
            }
            const PyVar& _module = fn._module != nullptr ? fn._module : top_frame()->_module;
            auto _frame = _new_frame(fn.code, _module, locals, fn._closure);
            if(fn.code->is_generator) return PyIter(Generator(this, std::move(_frame)));
            callstack.push(std::move(_frame));
            if(opCall) return _py_op_call;
            return _exec();
        }
        TypeError(OBJ_NAME(_t(*callable)).escape(true) + " object is not callable");
        return None;
    }

    // repl mode is only for setting `frame->id` to 0
    PyVarOrNull exec(Str source, Str filename, CompileMode mode, PyVar _module=nullptr){
        if(_module == nullptr) _module = _main;
        try {
            CodeObject_ code = compile(source, filename, mode);
            return _exec(code, _module);
        }catch (const pkpy::Exception& e){
            *_stderr << e.summary() << '\n';
        }catch (const std::exception& e) {
            *_stderr << "An std::exception occurred! It could be a bug.\n";
            *_stderr << e.what() << '\n';
        }
        callstack = {};
        return nullptr;
    }

    template<typename ...Args>
    inline std::unique_ptr<Frame> _new_frame(Args&&... args){
        if(callstack.size() > recursionlimit){
            _error("RecursionError", "maximum recursion depth exceeded");
        }
        return std::make_unique<Frame>(std::forward<Args>(args)...);
    }

    template<typename ...Args>
    inline PyVar _exec(Args&&... args){
        callstack.push(_new_frame(std::forward<Args>(args)...));
        return _exec();
    }

    PyVar _exec(){
        Frame* frame = top_frame();
        i64 base_id = frame->id;
        PyVar ret = nullptr;
        bool need_raise = false;

        while(true){
            if(frame->id < base_id) UNREACHABLE();
            try{
                if(need_raise){ need_raise = false; _raise(); }
                ret = run_frame(frame);
                if(ret == _py_op_yield) return _py_op_yield;
                if(ret != _py_op_call){
                    if(frame->id == base_id){      // [ frameBase<- ]
                        callstack.pop();
                        return ret;
                    }else{
                        callstack.pop();
                        frame = callstack.top().get();
                        frame->push(ret);
                    }
                }else{
                    frame = callstack.top().get();  // [ frameBase, newFrame<- ]
                }
            }catch(HandledException& e){
                continue;
            }catch(UnhandledException& e){
                PyVar obj = frame->pop();
                pkpy::Exception& _e = PyException_AS_C(obj);
                _e.st_push(frame->snapshot());
                callstack.pop();
                if(callstack.empty()) throw _e;
                frame = callstack.top().get();
                frame->push(obj);
                if(frame->id < base_id) throw ToBeRaisedException();
                need_raise = true;
            }catch(ToBeRaisedException& e){
                need_raise = true;
            }
        }
    }

    PyVar new_type_object(PyVar mod, StrName name, PyVar base){
        if(!is_type(base, tp_type)) UNREACHABLE();
        PyVar obj = pkpy::make_shared<PyObject, Py_<Type>>(tp_type, _all_types.size());
        setattr(obj, __base__, base);
        Str fullName = name.str();
        if(mod != builtins) fullName = OBJ_NAME(mod) + "." + name.str();
        setattr(obj, __name__, PyStr(fullName));
        setattr(mod, name, obj);
        _all_types.push_back(obj);
        return obj;
    }

    Type _new_type_object(StrName name, Type base=0) {
        PyVar obj = pkpy::make_shared<PyObject, Py_<Type>>(tp_type, _all_types.size());
        setattr(obj, __base__, _t(base));
        _types.set(name, obj);
        _all_types.push_back(obj);
        return OBJ_GET(Type, obj);
    }

    template<typename T>
    inline PyVar new_object(const PyVar& type, const T& _value) {
        if(!is_type(type, tp_type)) UNREACHABLE();
        return pkpy::make_shared<PyObject, Py_<RAW(T)>>(OBJ_GET(Type, type), _value);
    }
    template<typename T>
    inline PyVar new_object(const PyVar& type, T&& _value) {
        if(!is_type(type, tp_type)) UNREACHABLE();
        return pkpy::make_shared<PyObject, Py_<RAW(T)>>(OBJ_GET(Type, type), std::move(_value));
    }

    template<typename T>
    inline PyVar new_object(Type type, const T& _value) {
        return pkpy::make_shared<PyObject, Py_<RAW(T)>>(type, _value);
    }
    template<typename T>
    inline PyVar new_object(Type type, T&& _value) {
        return pkpy::make_shared<PyObject, Py_<RAW(T)>>(type, std::move(_value));
    }

    template<typename T, typename... Args>
    inline PyVar new_object(Args&&... args) {
        return new_object(T::_type(this), T(std::forward<Args>(args)...));
    }

    PyVar new_module(StrName name) {
        PyVar obj = new_object(tp_module, DummyModule());
        setattr(obj, __name__, PyStr(name.str()));
        _modules.set(name, obj);
        return obj;
    }

    PyVarOrNull getattr(const PyVar& obj, StrName name, bool throw_err=true) {
        PyVar* val;
        PyObject* cls;

        if(is_type(obj, tp_super)){
            const PyVar* root = &obj;
            int depth = 1;
            while(true){
                root = &OBJ_GET(PyVar, *root);
                if(!is_type(*root, tp_super)) break;
                depth++;
            }
            cls = _t(*root).get();
            for(int i=0; i<depth; i++) cls = cls->attr(__base__).get();

            val = (*root)->attr().try_get(name);
            if(val != nullptr) return *val;    
        }else{
            if(!obj.is_tagged() && obj->is_attr_valid()){
                val = obj->attr().try_get(name);
                if(val != nullptr) return *val;
            }
            cls = _t(obj).get();
        }

        while(cls != None.get()) {
            val = cls->attr().try_get(name);
            if(val != nullptr){
                if(is_type(*val, tp_function) || is_type(*val, tp_native_function)){
                    return PyBoundMethod({obj, *val});
                }else{
                    return *val;
                }
            }
            cls = cls->attr(__base__).get();
        }
        if(throw_err) AttributeError(obj, name);
        return nullptr;
    }

    template<typename T>
    inline void setattr(PyVar& obj, StrName name, T&& value) {
        if(obj.is_tagged()) TypeError("cannot set attribute");
        PyObject* p = obj.get();
        while(p->type == tp_super) p = static_cast<PyVar*>(p->value())->get();
        if(!p->is_attr_valid()) TypeError("cannot set attribute");
        p->attr().set(name, std::forward<T>(value));
    }

    template<int ARGC>
    void bind_method(PyVar obj, Str funcName, NativeFuncRaw fn) {
        check_type(obj, tp_type);
        setattr(obj, funcName, PyNativeFunc(pkpy::NativeFunc(fn, ARGC, true)));
    }

    template<int ARGC>
    void bind_func(PyVar obj, Str funcName, NativeFuncRaw fn) {
        setattr(obj, funcName, PyNativeFunc(pkpy::NativeFunc(fn, ARGC, false)));
    }

    template<int ARGC>
    void bind_func(Str typeName, Str funcName, NativeFuncRaw fn) {
        bind_func<ARGC>(_types[typeName], funcName, fn);     
    }

    template<int ARGC>
    void bind_method(Str typeName, Str funcName, NativeFuncRaw fn) {
        bind_method<ARGC>(_types[typeName], funcName, fn);
    }

    template<int ARGC, typename... Args>
    void bind_static_method(Args&&... args) {
        bind_func<ARGC>(std::forward<Args>(args)...);
    }

    template<int ARGC>
    void _bind_methods(std::vector<Str> typeNames, Str funcName, NativeFuncRaw fn) {
        for(auto& typeName : typeNames) bind_method<ARGC>(typeName, funcName, fn);
    }

    template<int ARGC>
    void bind_builtin_func(Str funcName, NativeFuncRaw fn) {
        bind_func<ARGC>(builtins, funcName, fn);
    }

    inline f64 num_to_float(const PyVar& obj){
        if(is_float(obj)){
            return PyFloat_AS_C(obj);
        } else if (is_int(obj)){
            return (f64)PyInt_AS_C(obj);
        }
        TypeError("expected 'int' or 'float', got " + OBJ_NAME(_t(obj)).escape(true));
        return 0;
    }

    PyVar num_negated(const PyVar& obj){
        if (is_int(obj)){
            return PyInt(-PyInt_AS_C(obj));
        }else if(is_float(obj)){
            return PyFloat(-PyFloat_AS_C(obj));
        }
        TypeError("expected 'int' or 'float', got " + OBJ_NAME(_t(obj)).escape(true));
        return nullptr;
    }

    int normalized_index(int index, int size){
        if(index < 0) index += size;
        if(index < 0 || index >= size){
            IndexError(std::to_string(index) + " not in [0, " + std::to_string(size) + ")");
        }
        return index;
    }

    Str disassemble(CodeObject_ co){
        std::vector<int> jumpTargets;
        for(auto byte : co->codes){
            if(byte.op == OP_JUMP_ABSOLUTE || byte.op == OP_SAFE_JUMP_ABSOLUTE || byte.op == OP_POP_JUMP_IF_FALSE){
                jumpTargets.push_back(byte.arg);
            }
        }
        StrStream ss;
        ss << std::string(54, '-') << '\n';
        ss << co->name << ":\n";
        int prev_line = -1;
        for(int i=0; i<co->codes.size(); i++){
            const Bytecode& byte = co->codes[i];
            if(byte.op == OP_NO_OP) continue;
            Str line = std::to_string(byte.line);
            if(byte.line == prev_line) line = "";
            else{
                if(prev_line != -1) ss << "\n";
                prev_line = byte.line;
            }

            std::string pointer;
            if(std::find(jumpTargets.begin(), jumpTargets.end(), i) != jumpTargets.end()){
                pointer = "-> ";
            }else{
                pointer = "   ";
            }
            ss << pad(line, 8) << pointer << pad(std::to_string(i), 3);
            ss << " " << pad(OP_NAMES[byte.op], 20) << " ";
            // ss << pad(byte.arg == -1 ? "" : std::to_string(byte.arg), 5);
            std::string argStr = byte.arg == -1 ? "" : std::to_string(byte.arg);
            if(byte.op == OP_LOAD_CONST){
                argStr += " (" + PyStr_AS_C(asRepr(co->consts[byte.arg])) + ")";
            }
            if(byte.op == OP_LOAD_NAME_REF || byte.op == OP_LOAD_NAME || byte.op == OP_RAISE || byte.op == OP_STORE_NAME){
                argStr += " (" + co->names[byte.arg].first.str().escape(true) + ")";
            }
            if(byte.op == OP_FAST_INDEX || byte.op == OP_FAST_INDEX_REF){
                auto& a = co->names[byte.arg & 0xFFFF];
                auto& x = co->names[(byte.arg >> 16) & 0xFFFF];
                argStr += " (" + a.first.str() + '[' + x.first.str() + "])";
            }
            ss << pad(argStr, 20);      // may overflow
            ss << co->blocks[byte.block].to_string();
            if(i != co->codes.size() - 1) ss << '\n';
        }
        StrStream consts;
        consts << "co_consts: ";
        consts << PyStr_AS_C(asRepr(PyList(co->consts)));

        StrStream names;
        names << "co_names: ";
        pkpy::List list;
        for(int i=0; i<co->names.size(); i++){
            list.push_back(PyStr(co->names[i].first.str()));
        }
        names << PyStr_AS_C(asRepr(PyList(list)));
        ss << '\n' << consts.str() << '\n' << names.str() << '\n';

        for(int i=0; i<co->consts.size(); i++){
            PyVar obj = co->consts[i];
            if(is_type(obj, tp_function)){
                const auto& f = PyFunction_AS_C(obj);
                ss << disassemble(f.code);
            }
        }
        return Str(ss.str());
    }

    // for quick access
    Type tp_object, tp_type, tp_int, tp_float, tp_bool, tp_str;
    Type tp_list, tp_tuple;
    Type tp_function, tp_native_function, tp_native_iterator, tp_bound_method;
    Type tp_slice, tp_range, tp_module, tp_ref;
    Type tp_super, tp_exception, tp_star_wrapper;

    template<typename P>
    inline PyVarRef PyRef(P&& value) {
        static_assert(std::is_base_of_v<BaseRef, RAW(P)>);
        return new_object(tp_ref, std::forward<P>(value));
    }

    inline const BaseRef* PyRef_AS_C(const PyVar& obj)
    {
        if(!is_type(obj, tp_ref)) TypeError("expected an l-value");
        return static_cast<const BaseRef*>(obj->value());
    }

    template<typename P>
    inline PyVar PyIter(P&& value) {
        static_assert(std::is_base_of_v<BaseIter, RAW(P)>);
        return new_object(tp_native_iterator, std::forward<P>(value));
    }

    inline BaseIter* PyIter_AS_C(const PyVar& obj)
    {
        check_type(obj, tp_native_iterator);
        return static_cast<BaseIter*>(obj->value());
    }

    inline const Str& PyStr_AS_C(const PyVar& obj) {
        check_type(obj, tp_str);
        return OBJ_GET(Str, obj);
    }
    inline PyVar PyStr(const Str& value) {
        // some BUGs here
        // if(value.size() == 1){
        //     char c = value.c_str()[0];
        //     if(c >= 0) return _ascii_str_pool[(int)c];
        // }
        return new_object(tp_str, value);
    }

    inline PyVar PyInt(i64 value) {
        if(((value << 2) >> 2) != value){
            _error("OverflowError", std::to_string(value) + " is out of range");
        }
        value = (value << 2) | 0b01;
        return PyVar(reinterpret_cast<int*>(value));
    }

    inline i64 PyInt_AS_C(const PyVar& obj){
        check_type(obj, tp_int);
        return obj.bits >> 2;
    }

    inline i64 _PyInt_AS_C(const PyVar& obj){
        return obj.bits >> 2;
    }

    inline PyVar PyFloat(f64 value) {
        i64 bits = __8B(value)._int;
        bits = (bits >> 2) << 2;
        bits |= 0b10;
        return PyVar(reinterpret_cast<int*>(bits));
    }

    inline f64 PyFloat_AS_C(const PyVar& obj){
        check_type(obj, tp_float);
        i64 bits = obj.bits;
        bits = (bits >> 2) << 2;
        return __8B(bits)._float;
    }

    inline f64 _PyFloat_AS_C(const PyVar& obj){
        i64 bits = obj.bits;
        bits = (bits >> 2) << 2;
        return __8B(bits)._float;
    }

    DEF_NATIVE(List, pkpy::List, tp_list)
    DEF_NATIVE(Tuple, pkpy::Tuple, tp_tuple)
    DEF_NATIVE(Function, pkpy::Function, tp_function)
    DEF_NATIVE(NativeFunc, pkpy::NativeFunc, tp_native_function)
    DEF_NATIVE(BoundMethod, pkpy::BoundMethod, tp_bound_method)
    DEF_NATIVE(Range, pkpy::Range, tp_range)
    DEF_NATIVE(Slice, pkpy::Slice, tp_slice)
    DEF_NATIVE(Exception, pkpy::Exception, tp_exception)
    DEF_NATIVE(StarWrapper, pkpy::StarWrapper, tp_star_wrapper)
    
    // there is only one True/False, so no need to copy them!
    inline bool PyBool_AS_C(const PyVar& obj){
        check_type(obj, tp_bool);
        return obj == True;
    }
    inline bool _PyBool_AS_C(const PyVar& obj){ return obj == True; }
    inline const PyVar& PyBool(bool value){return value ? True : False;}

    void init_builtin_types(){
        PyVar _tp_object = pkpy::make_shared<PyObject, Py_<Type>>(1, 0);
        PyVar _tp_type = pkpy::make_shared<PyObject, Py_<Type>>(1, 1);
        _all_types.push_back(_tp_object);
        _all_types.push_back(_tp_type);
        tp_object = 0; tp_type = 1;

        _types.set("object", _tp_object);
        _types.set("type", _tp_type);

        tp_int = _new_type_object("int");
        tp_float = _new_type_object("float");
        if(tp_int.index != kTpIntIndex || tp_float.index != kTpFloatIndex) UNREACHABLE();

        tp_bool = _new_type_object("bool");
        tp_str = _new_type_object("str");
        tp_list = _new_type_object("list");
        tp_tuple = _new_type_object("tuple");
        tp_slice = _new_type_object("slice");
        tp_range = _new_type_object("range");
        tp_module = _new_type_object("module");
        tp_ref = _new_type_object("_ref");
        tp_star_wrapper = _new_type_object("_star_wrapper");
        
        tp_function = _new_type_object("function");
        tp_native_function = _new_type_object("native_function");
        tp_native_iterator = _new_type_object("native_iterator");
        tp_bound_method = _new_type_object("bound_method");
        tp_super = _new_type_object("super");
        tp_exception = _new_type_object("Exception");

        this->None = new_object(_new_type_object("NoneType"), DUMMY_VAL);
        this->Ellipsis = new_object(_new_type_object("ellipsis"), DUMMY_VAL);
        this->True = new_object(tp_bool, true);
        this->False = new_object(tp_bool, false);
        this->builtins = new_module("builtins");
        this->_main = new_module("__main__");
        this->_py_op_call = new_object(_new_type_object("_py_op_call"), DUMMY_VAL);
        this->_py_op_yield = new_object(_new_type_object("_py_op_yield"), DUMMY_VAL);

        setattr(_t(tp_type), __base__, _t(tp_object));
        setattr(_t(tp_object), __base__, None);
        
        for(auto [k, v]: _types.items()){
            setattr(v, __name__, PyStr(k.str()));
        }

        std::vector<Str> pb_types = {"type", "object", "bool", "int", "float", "str", "list", "tuple", "range"};
        for (auto& name : pb_types) {
            setattr(builtins, name, _types[name]);
        }

        post_init();
        for(auto [k, v]: _types.items()){
            v->attr()._try_perfect_rehash();
        }
        builtins->attr()._try_perfect_rehash();
    }

    void post_init();

    i64 hash(const PyVar& obj){
        if (is_type(obj, tp_str)) return PyStr_AS_C(obj).hash();
        if (is_int(obj)) return PyInt_AS_C(obj);
        if (is_type(obj, tp_tuple)) {
            i64 x = 1000003;
            const pkpy::Tuple& items = PyTuple_AS_C(obj);
            for (int i=0; i<items.size(); i++) {
                i64 y = hash(items[i]);
                x = x ^ (y + 0x9e3779b9 + (x << 6) + (x >> 2)); // recommended by Github Copilot
            }
            return x;
        }
        if (is_type(obj, tp_type)) return obj.bits;
        if (is_type(obj, tp_bool)) return _PyBool_AS_C(obj) ? 1 : 0;
        if (is_float(obj)){
            f64 val = PyFloat_AS_C(obj);
            return (i64)std::hash<f64>()(val);
        }
        TypeError("unhashable type: " +  OBJ_NAME(_t(obj)).escape(true));
        return 0;
    }

    /***** Error Reporter *****/
private:
    void _error(StrName name, const Str& msg){
        _error(pkpy::Exception(name, msg));
    }

    void _error(pkpy::Exception e){
        if(callstack.empty()){
            e.is_re = false;
            throw e;
        }
        top_frame()->push(PyException(e));
        _raise();
    }

    void _raise(){
        bool ok = top_frame()->jump_to_exception_handler();
        if(ok) throw HandledException();
        else throw UnhandledException();
    }

public:
    void IOError(const Str& msg) { _error("IOError", msg); }
    void NotImplementedError(){ _error("NotImplementedError", ""); }
    void TypeError(const Str& msg){ _error("TypeError", msg); }
    void ZeroDivisionError(){ _error("ZeroDivisionError", "division by zero"); }
    void IndexError(const Str& msg){ _error("IndexError", msg); }
    void ValueError(const Str& msg){ _error("ValueError", msg); }
    void NameError(StrName name){ _error("NameError", "name " + name.str().escape(true) + " is not defined"); }

    void AttributeError(PyVar obj, StrName name){
        _error("AttributeError", "type " +  OBJ_NAME(_t(obj)).escape(true) + " has no attribute " + name.str().escape(true));
    }

    inline void check_type(const PyVar& obj, Type type){
        if(is_type(obj, type)) return;
        TypeError("expected " + OBJ_NAME(_t(type)).escape(true) + ", but got " + OBJ_NAME(_t(obj)).escape(true));
    }

    inline PyVar& _t(Type t){
        return _all_types[t.index];
    }

    inline PyVar& _t(const PyVar& obj){
        if(is_int(obj)) return _t(tp_int);
        if(is_float(obj)) return _t(tp_float);
        return _all_types[OBJ_GET(Type, _t(obj->type)).index];
    }

    template<typename T>
    PyVar register_class(PyVar mod){
        PyVar type = new_type_object(mod, T::_name(), _t(tp_object));
        if(OBJ_NAME(mod) != T::_mod()) UNREACHABLE();
        T::_register(this, mod, type);
        type->attr()._try_perfect_rehash();
        return type;
    }

    template<typename T>
    inline T& py_cast(const PyVar& obj){
        check_type(obj, T::_type(this));
        return OBJ_GET(T, obj);
    }

    void unpack_args(pkpy::Args& args){
        pkpy::List unpacked;
        for(int i=0; i<args.size(); i++){
            if(is_type(args[i], tp_star_wrapper)){
                auto& star = PyStarWrapper_AS_C(args[i]);
                if(!star.rvalue) UNREACHABLE();
                PyVar list = asList(star.obj);
                pkpy::List& list_c = PyList_AS_C(list);
                unpacked.insert(unpacked.end(), list_c.begin(), list_c.end());
            }else{
                unpacked.push_back(args[i]);
            }
        }
        args = std::move(unpacked);
    }

    ~VM() {
        if(!use_stdio){
            delete _stdout;
            delete _stderr;
        }
    }

    CodeObject_ compile(Str source, Str filename, CompileMode mode);
};

/***** Pointers' Impl *****/
PyVar NameRef::get(VM* vm, Frame* frame) const{
    PyVar* val;
    val = frame->f_locals().try_get(name());
    if(val != nullptr) return *val;
    val = frame->f_closure_try_get(name());
    if(val != nullptr) return *val;
    val = frame->f_globals().try_get(name());
    if(val != nullptr) return *val;
    val = vm->builtins->attr().try_get(name());
    if(val != nullptr) return *val;
    vm->NameError(name());
    return nullptr;
}

void NameRef::set(VM* vm, Frame* frame, PyVar val) const{
    switch(scope()) {
        case NAME_LOCAL: frame->f_locals().set(name(), std::move(val)); break;
        case NAME_GLOBAL:
            if(frame->f_locals().try_set(name(), std::move(val))) return;
            frame->f_globals().set(name(), std::move(val));
            break;
        default: UNREACHABLE();
    }
}

void NameRef::del(VM* vm, Frame* frame) const{
    switch(scope()) {
        case NAME_LOCAL: {
            if(frame->f_locals().contains(name())){
                frame->f_locals().erase(name());
            }else{
                vm->NameError(name());
            }
        } break;
        case NAME_GLOBAL:
        {
            if(frame->f_locals().contains(name())){
                frame->f_locals().erase(name());
            }else{
                if(frame->f_globals().contains(name())){
                    frame->f_globals().erase(name());
                }else{
                    vm->NameError(name());
                }
            }
        } break;
        default: UNREACHABLE();
    }
}

PyVar AttrRef::get(VM* vm, Frame* frame) const{
    return vm->getattr(obj, attr.name());
}

void AttrRef::set(VM* vm, Frame* frame, PyVar val) const{
    vm->setattr(obj, attr.name(), std::move(val));
}

void AttrRef::del(VM* vm, Frame* frame) const{
    if(!obj->is_attr_valid()) vm->TypeError("cannot delete attribute");
    if(!obj->attr().contains(attr.name())) vm->AttributeError(obj, attr.name());
    obj->attr().erase(attr.name());
}

PyVar IndexRef::get(VM* vm, Frame* frame) const{
    return vm->fast_call(__getitem__, pkpy::two_args(obj, index));
}

void IndexRef::set(VM* vm, Frame* frame, PyVar val) const{
    pkpy::Args args(3);
    args[0] = obj; args[1] = index; args[2] = std::move(val);
    vm->fast_call(__setitem__, std::move(args));
}

void IndexRef::del(VM* vm, Frame* frame) const{
    vm->fast_call(__delitem__, pkpy::two_args(obj, index));
}

PyVar TupleRef::get(VM* vm, Frame* frame) const{
    pkpy::Tuple args(objs.size());
    for (int i = 0; i < objs.size(); i++) {
        args[i] = vm->PyRef_AS_C(objs[i])->get(vm, frame);
    }
    return vm->PyTuple(std::move(args));
}

void TupleRef::set(VM* vm, Frame* frame, PyVar val) const{
    val = vm->asIter(val);
    BaseIter* iter = vm->PyIter_AS_C(val);
    for(int i=0; i<objs.size(); i++){
        PyVarOrNull x;
        if(is_type(objs[i], vm->tp_star_wrapper)){
            auto& star = vm->PyStarWrapper_AS_C(objs[i]);
            if(star.rvalue) vm->ValueError("can't use starred expression here");
            if(i != objs.size()-1) vm->ValueError("* can only be used at the end");
            auto ref = vm->PyRef_AS_C(star.obj);
            pkpy::List list;
            while((x = iter->next()) != nullptr) list.push_back(x);
            ref->set(vm, frame, vm->PyList(std::move(list)));
            return;
        }else{
            x = iter->next();
            if(x == nullptr) vm->ValueError("not enough values to unpack");
            vm->PyRef_AS_C(objs[i])->set(vm, frame, x);
        }
    }
    PyVarOrNull x = iter->next();
    if(x != nullptr) vm->ValueError("too many values to unpack");
}

void TupleRef::del(VM* vm, Frame* frame) const{
    for(int i=0; i<objs.size(); i++) vm->PyRef_AS_C(objs[i])->del(vm, frame);
}

/***** Frame's Impl *****/
inline void Frame::try_deref(VM* vm, PyVar& v){
    if(is_type(v, vm->tp_ref)) v = vm->PyRef_AS_C(v)->get(vm, this);
}

PyVar pkpy::NativeFunc::operator()(VM* vm, pkpy::Args& args) const{
    int args_size = args.size() - (int)method;  // remove self
    if(argc != -1 && args_size != argc) {
        vm->TypeError("expected " + std::to_string(argc) + " arguments, but got " + std::to_string(args_size));
    }
    return f(vm, args);
}

void CodeObject::optimize(VM* vm){
    std::vector<StrName> keys;
    for(auto& p: names) if(p.second == NAME_LOCAL) keys.push_back(p.first);
    uint32_t base_n = (uint32_t)(keys.size() / kLocalsLoadFactor + 0.5);
    perfect_locals_capacity = pkpy::find_next_capacity(base_n);
    perfect_hash_seed = pkpy::find_perfect_hash_seed(perfect_locals_capacity, keys);

    for(int i=1; i<codes.size(); i++){
        if(codes[i].op == OP_UNARY_NEGATIVE && codes[i-1].op == OP_LOAD_CONST){
            codes[i].op = OP_NO_OP;
            int pos = codes[i-1].arg;
            consts[pos] = vm->num_negated(consts[pos]);
        }

        if(i>=2 && codes[i].op == OP_BUILD_INDEX){
            const Bytecode& a = codes[i-1];
            const Bytecode& x = codes[i-2];
            if(codes[i].arg == 1){
                if(a.op == OP_LOAD_NAME && x.op == OP_LOAD_NAME){
                    codes[i].op = OP_FAST_INDEX;
                }else continue;
            }else{
                if(a.op == OP_LOAD_NAME_REF && x.op == OP_LOAD_NAME_REF){
                    codes[i].op = OP_FAST_INDEX_REF;
                }else continue;
            }
            codes[i].arg = (a.arg << 16) | x.arg;
            codes[i-1].op = OP_NO_OP;
            codes[i-2].op = OP_NO_OP;
        }
    }
}