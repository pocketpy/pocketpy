#include "pocketpy/interpreter/vm.hpp"

#include <iostream>
#include <cmath>
#include <stdexcept>

const static char* OP_NAMES[] = {
#define OPCODE(name) #name,
#include "pocketpy/opcodes.h"
#undef OPCODE
};

namespace pkpy {

struct JsonSerializer {
    VM* vm;
    PyVar root;
    SStream ss;

    JsonSerializer(VM* vm, PyVar root) : vm(vm), root(root) {}

    template <typename T>
    void write_array(T& arr) {
        ss << '[';
        for(int i = 0; i < arr.size(); i++) {
            if(i != 0) ss << ", ";
            write_object(arr[i]);
        }
        ss << ']';
    }

    void write_dict(Dict& dict) {
        ss << '{';
        bool first = true;
        dict.apply([&](PyVar k, PyVar v) {
            if(!first) ss << ", ";
            first = false;
            if(!is_type(k, VM::tp_str)) {
                vm->TypeError(_S("json keys must be string, got ", _type_name(vm, vm->_tp(k))));
            }
            ss << _CAST(Str&, k).escape(false) << ": ";
            write_object(v);
        });
        ss << '}';
    }

    void write_object(PyVar obj) {
        Type obj_t = vm->_tp(obj);
        if(obj == vm->None) {
            ss << "null";
        } else if(obj_t == vm->tp_int) {
            ss << _CAST(i64, obj);
        } else if(obj_t == vm->tp_float) {
            f64 val = _CAST(f64, obj);
            if(std::isinf(val) || std::isnan(val)) vm->ValueError("cannot jsonify 'nan' or 'inf'");
            ss << val;
        } else if(obj_t == vm->tp_bool) {
            ss << (obj == vm->True ? "true" : "false");
        } else if(obj_t == vm->tp_str) {
            _CAST(Str&, obj).escape_(ss, false);
        } else if(obj_t == vm->tp_list) {
            write_array<List>(_CAST(List&, obj));
        } else if(obj_t == vm->tp_tuple) {
            write_array<Tuple>(_CAST(Tuple&, obj));
        } else if(obj_t == vm->tp_dict) {
            write_dict(_CAST(Dict&, obj));
        } else {
            vm->TypeError(_S("unrecognized type ", _type_name(vm, obj_t).escape()));
        }
    }

    Str serialize() {
        auto _lock = vm->heap.gc_scope_lock();
        write_object(root);
        return ss.str();
    }
};

VM::VM(bool enable_os) : heap(this), enable_os(enable_os) {
    this->vm = this;
    this->__c.error = nullptr;
    _ceval_on_step = nullptr;
    _stdout = [](const char* buf, int size) {
        std::cout.write(buf, size);
    };
    _stderr = [](const char* buf, int size) {
        std::cerr.write(buf, size);
    };
    builtins = nullptr;
    _main = nullptr;
    __last_exception = nullptr;
    _import_handler = [](const char* name, int* out_size) -> unsigned char* {
        return nullptr;
    };
    __init_builtin_types();
}

Str VM::py_str(PyVar obj) {
    const PyTypeInfo* ti = _tp_info(obj);
    if(ti->m__str__) return ti->m__str__(this, obj);
    PyVar self;
    PyVar f = get_unbound_method(obj, __str__, &self, false);
    if(self != PY_NULL) {
        PyVar retval = call_method(self, f);
        if(!is_type(retval, tp_str)) { throw std::runtime_error("object.__str__ must return str"); }
        return PK_OBJ_GET(Str, retval);
    }
    return py_repr(obj);
}

Str VM::py_repr(PyVar obj) {
    const PyTypeInfo* ti = _tp_info(obj);
    if(ti->m__repr__) return ti->m__repr__(this, obj);
    PyVar retval = call_method(obj, __repr__);
    if(!is_type(retval, tp_str)) { throw std::runtime_error("object.__repr__ must return str"); }
    return PK_OBJ_GET(Str, retval);
}

Str VM::py_json(PyVar obj) {
    auto j = JsonSerializer(this, obj);
    return j.serialize();
}

PyVar VM::py_iter(PyVar obj) {
    const PyTypeInfo* ti = _tp_info(obj);
    if(ti->m__iter__) return ti->m__iter__(this, obj);
    PyVar self;
    PyVar iter_f = get_unbound_method(obj, __iter__, &self, false);
    if(self != PY_NULL) return call_method(self, iter_f);
    TypeError(_type_name(vm, _tp(obj)).escape() + " object is not iterable");
    return nullptr;
}

ArgsView VM::cast_array_view(PyVar obj) {
    if(is_type(obj, VM::tp_list)) {
        List& list = PK_OBJ_GET(List, obj);
        return ArgsView(list.begin(), list.end());
    } else if(is_type(obj, VM::tp_tuple)) {
        Tuple& tuple = PK_OBJ_GET(Tuple, obj);
        return ArgsView(tuple.begin(), tuple.end());
    }
    TypeError(_S("expected list or tuple, got ", _type_name(this, _tp(obj)).escape()));
}

void VM::set_main_argv(int argc, char** argv) {
    PyVar mod = vm->_modules["sys"];
    List argv_(argc);
    for(int i = 0; i < argc; i++)
        argv_[i] = VAR(std::string_view(argv[i]));
    mod->attr().set("argv", VAR(std::move(argv_)));
}

PyVar* VM::find_name_in_mro(Type cls, StrName name) {
    PyVar* val;
    do {
        val = _t(cls)->attr().try_get_2(name);
        if(val != nullptr) return val;
        cls = _all_types[cls].base;
        if(!cls) break;
    } while(true);
    return nullptr;
}

bool VM::isinstance(PyVar obj, Type base) { return issubclass(_tp(obj), base); }

bool VM::issubclass(Type cls, Type base) {
    do {
        if(cls == base) return true;
        Type next = _all_types[cls].base;
        if(!next) break;
        cls = next;
    } while(true);
    return false;
}

PyVar VM::exec(std::string_view source, Str filename, CompileMode mode, PyObject* _module) {
    if(_module == nullptr) _module = _main;
    try {
#if PK_DEBUG_PRECOMPILED_EXEC == 1
        Str precompiled = vm->precompile(source, filename, mode);
        source = precompiled.sv();
#endif
        CodeObject_ code = compile(source, filename, mode);
        return _exec(code, _module);
    } catch(TopLevelException e) { stderr_write(e.summary() + "\n"); } catch(const std::exception& e) {
        Str msg = "An std::exception occurred! It could be a bug.\n";
        msg = msg + e.what() + "\n";
        stderr_write(msg);
    } catch(NeedMoreLines) { throw; } catch(...) {
        Str msg = "An unknown exception occurred! It could be a bug. Please report it to @blueloveTH on GitHub.\n";
        stderr_write(msg);
    }
    callstack.clear();
    s_data.clear();
    return nullptr;
}

PyVar VM::exec(std::string_view source) { return exec(source, "main.py", EXEC_MODE); }

PyVar VM::eval(std::string_view source) { return exec(source, "<eval>", EVAL_MODE); }

PyObject* VM::new_type_object(PyObject* mod, StrName name, Type base, bool subclass_enabled, PyTypeInfo::Vt vt) {
    PyObject* obj = heap._new<Type>(tp_type, Type(_all_types.size()));
    const PyTypeInfo& base_info = _all_types[base];
    if(!base_info.subclass_enabled) {
        Str error = _S("type ", base_info.name.escape(), " is not `subclass_enabled`");
        throw std::runtime_error(error.c_str());
    }
    if(base_info.vt) {
        if(vt) {
            Str error = _S("type ", base_info.name.escape(), " has a custom vtable, cannot override");
            throw std::runtime_error(error.c_str());
        } else {
            // promote base vt to its subclass
            vt = base_info.vt;
        }
    }
    _all_types.emplace_back(obj, base, mod, name, subclass_enabled, vt);
    return obj;
}

bool VM::py_eq(PyVar lhs, PyVar rhs) {
    if(is_int(lhs) && is_int(rhs)) return lhs.as<i64>() == rhs.as<i64>();
    const PyTypeInfo* ti = _tp_info(lhs);
    PyVar res;
    if(ti->m__eq__) {
        res = ti->m__eq__(this, lhs, rhs);
        if(!is_not_implemented(res)) return res == vm->True;
    }
    res = call_method(lhs, __eq__, rhs);
    if(!is_not_implemented(res)) return res == vm->True;

    ti = _tp_info(rhs);
    if(ti->m__eq__) {
        res = ti->m__eq__(this, rhs, lhs);
        if(!is_not_implemented(res)) return res == vm->True;
    }
    res = call_method(rhs, __eq__, lhs);
    if(!is_not_implemented(res)) return res == vm->True;
    return false;
}

PyVar VM::py_op(std::string_view name) {
    PyVar func;
    auto it = __cached_op_funcs.find(name);
    if(it == __cached_op_funcs.end()) {
        func = py_import("operator")->attr(StrName::get(name));
        __cached_op_funcs[name] = func;
    } else {
        func = it->second;
    }
    return func;
}

i64 VM::normalized_index(i64 index, int size) {
    if(index < 0) index += size;
    if(index < 0 || index >= size) { IndexError(std::to_string(index) + " not in [0, " + std::to_string(size) + ")"); }
    return index;
}

PyVar VM::_py_next(const PyTypeInfo* ti, PyVar obj) {
    if(ti->op__next__) {
        unsigned n = ti->op__next__(this, obj);
        return __pack_next_retval(n);
    }
    return call_method(obj, __next__);
}

PyVar VM::py_next(PyVar obj) {
    const PyTypeInfo* ti = _tp_info(obj);
    return _py_next(ti, obj);
}

bool VM::py_callable(PyVar obj) {
    Type cls = vm->_tp(obj);
    switch(cls.index) {
        case VM::tp_function.index: return true;
        case VM::tp_native_func.index: return true;
        case VM::tp_bound_method.index: return true;
        case VM::tp_type.index: return true;
    }
    return vm->find_name_in_mro(cls, __call__) != nullptr;
}

PyVar VM::__minmax_reduce(bool (VM::*op)(PyVar, PyVar), PyVar args, PyVar key) {
    auto _lock = heap.gc_scope_lock();
    const Tuple& args_tuple = PK_OBJ_GET(Tuple, args);  // from *args, it must be a tuple
    if(key == vm->None && args_tuple.size() == 2) {
        // fast path
        PyVar a = args_tuple[0];
        PyVar b = args_tuple[1];
        return (this->*op)(a, b) ? a : b;
    }

    if(args_tuple.size() == 0) TypeError("expected at least 1 argument, got 0");

    ArgsView view(nullptr, nullptr);
    if(args_tuple.size() == 1) {
        view = cast_array_view(args_tuple[0]);
    } else {
        view = ArgsView(args_tuple);
    }

    if(view.empty()) ValueError("arg is an empty sequence");
    PyVar res = view[0];

    if(key == vm->None) {
        for(int i = 1; i < view.size(); i++) {
            if((this->*op)(view[i], res)) res = view[i];
        }
    } else {
        auto _lock = heap.gc_scope_lock();
        for(int i = 1; i < view.size(); i++) {
            PyVar a = call(key, view[i]);
            PyVar b = call(key, res);
            if((this->*op)(a, b)) res = view[i];
        }
    }
    return res;
}

PyObject* VM::py_import(Str path, bool throw_err) {
    if(path.empty()) vm->ValueError("empty module name");
    static auto f_join = [](const vector<std::string_view>& cpnts) {
        SStream ss;
        for(int i = 0; i < cpnts.size(); i++) {
            if(i != 0) ss << ".";
            ss << cpnts[i];
        }
        return ss.str();
    };

    if(path[0] == '.') {
        if(__import_context.pending.empty()) { ImportError("relative import outside of package"); }
        Str curr_path = __import_context.pending.back();
        bool curr_is_init = __import_context.pending_is_init.back();
        // convert relative path to absolute path
        vector<std::string_view> cpnts = curr_path.split('.');
        int prefix = 0;  // how many dots in the prefix
        for(int i = 0; i < path.length(); i++) {
            if(path[i] == '.')
                prefix++;
            else
                break;
        }
        if(prefix > cpnts.size()) ImportError("attempted relative import beyond top-level package");
        path = path.substr(prefix);  // remove prefix
        for(int i = (int)curr_is_init; i < prefix; i++)
            cpnts.pop_back();
        if(!path.empty()) cpnts.push_back(path.sv());
        path = f_join(cpnts);
    }

    assert(path.begin()[0] != '.' && path.end()[-1] != '.');

    // check existing module
    StrName name(path);
    PyVar ext_mod = _modules.try_get(name);
    if(ext_mod != nullptr) return ext_mod.get();

    vector<std::string_view> path_cpnts = path.split('.');
    // check circular import
    if(__import_context.pending.size() > 128) { ImportError("maximum recursion depth exceeded while importing"); }

    // try import
    Str filename = path.replace('.', PK_PLATFORM_SEP) + ".py";
    Str source;
    bool is_init = false;
    auto it = _lazy_modules.find(name);
    if(it == _lazy_modules.end()) {
        int out_size;
        unsigned char* out = _import_handler(filename.c_str(), &out_size);
        if(out == nullptr) {
            filename = path.replace('.', PK_PLATFORM_SEP).str() + PK_PLATFORM_SEP + "__init__.py";
            is_init = true;
            out = _import_handler(filename.c_str(), &out_size);
        }
        if(out == nullptr) {
            if(throw_err)
                ImportError(_S("module ", path.escape(), " not found"));
            else
                return nullptr;
        }
        assert(out_size >= 0);
        source = Str(std::string_view((char*)out, out_size));
        std::free(out);
    } else {
        source = it->second;
        _lazy_modules.erase(it);
    }
    auto _ = __import_context.scope(path, is_init);
    CodeObject_ code = compile(source, filename, EXEC_MODE);

    Str name_cpnt = path_cpnts.back();
    path_cpnts.pop_back();
    PyObject* new_mod = new_module(name_cpnt, f_join(path_cpnts));
    _exec(code, new_mod);
    return new_mod;
}

VM::~VM() {
    // clear managed heap
    for(PyObject* obj: heap.gen)
        heap._delete(obj);
    for(PyObject* obj: heap._no_gc)
        heap._delete(obj);
    // clear everything
    callstack.clear();
    s_data.clear();
    _all_types.clear();
    _modules.clear();
    _lazy_modules.clear();
}

PyVar VM::py_negate(PyVar obj) {
    const PyTypeInfo* ti = _tp_info(obj);
    if(ti->m__neg__) return ti->m__neg__(this, obj);
    return call_method(obj, __neg__);
}

bool VM::__py_bool_non_trivial(PyVar obj) {
    if(obj == None) return false;
    if(is_int(obj)) return _CAST(i64, obj) != 0;
    if(is_float(obj)) return _CAST(f64, obj) != 0.0;
    PyVar self;
    PyVar len_f = get_unbound_method(obj, __len__, &self, false);
    if(self != PY_NULL) {
        PyVar ret = call_method(self, len_f);
        return CAST(i64, ret) != 0;
    }
    return true;
}

void VM::__obj_gc_mark(PyObject* obj) {
    if(obj->gc_marked) return;
    obj->gc_marked = true;
    const PyTypeInfo* ti = _tp_info(obj->type);
    if(ti->vt._gc_mark) ti->vt._gc_mark(obj->_value_ptr(), this);
    if(obj->is_attr_valid()) {
        obj->attr().apply([this](StrName _, PyVar obj) {
            if(obj.is_ptr) vm->__obj_gc_mark((obj).get());
        });
    }
}

void VM::__stack_gc_mark(PyVar* begin, PyVar* end) {
    for(PyVar* it = begin; it != end; it++) {
        if(it->is_ptr) {
            __obj_gc_mark(it->get());
        } else {
            if(it->type == tp_stack_memory) {
                // [sm:3, _0, _1, _2, sm:-3]
                int count = it->as<StackMemory>().count;
                if(count > 0) it += count;
            }
        }
    }
}

void* VM::__stack_alloc(int size) {
    int count = size / sizeof(PyVar) + 1;
    s_data.emplace(tp_stack_memory, StackMemory(count));
    void* out = s_data._sp;
    s_data._sp += count;
    s_data.emplace(tp_stack_memory, StackMemory(-count));
    return out;
}

List VM::py_list(PyVar it) {
    auto _lock = heap.gc_scope_lock();
    it = py_iter(it);
    List list;
    const PyTypeInfo* info = _tp_info(it);
    PyVar obj = _py_next(info, it);
    while(obj != StopIteration) {
        list.push_back(obj);
        obj = _py_next(info, it);
    }
    return list;
}

void VM::parse_int_slice(const Slice& s, int length, int& start, int& stop, int& step) {
    auto clip = [](int value, int min, int max) {
        if(value < min) return min;
        if(value > max) return max;
        return value;
    };
    if(s.step == None)
        step = 1;
    else
        step = CAST(int, s.step);
    if(step == 0) ValueError("slice step cannot be zero");
    if(step > 0) {
        if(s.start == None) {
            start = 0;
        } else {
            start = CAST(int, s.start);
            if(start < 0) start += length;
            start = clip(start, 0, length);
        }
        if(s.stop == None) {
            stop = length;
        } else {
            stop = CAST(int, s.stop);
            if(stop < 0) stop += length;
            stop = clip(stop, 0, length);
        }
    } else {
        if(s.start == None) {
            start = length - 1;
        } else {
            start = CAST(int, s.start);
            if(start < 0) start += length;
            start = clip(start, -1, length - 1);
        }
        if(s.stop == None) {
            stop = -1;
        } else {
            stop = CAST(int, s.stop);
            if(stop < 0) stop += length;
            stop = clip(stop, -1, length - 1);
        }
    }
}

i64 VM::py_hash(PyVar obj) {
    // https://docs.python.org/3.10/reference/datamodel.html#object.__hash__
    const PyTypeInfo* ti = _tp_info(obj);
    if(ti->m__hash__) return ti->m__hash__(this, obj);

    PyVar self;
    PyVar f = get_unbound_method(obj, __hash__, &self, false);
    if(f != nullptr) {
        PyVar ret = call_method(self, f);
        return CAST(i64, ret);
    }
    // if it is trivial `object`, return PK_BITS
    if(ti == &_all_types[tp_object]) return obj.hash();
    // otherwise, we check if it has a custom __eq__ other than object.__eq__
    bool has_custom_eq = false;
    if(ti->m__eq__)
        has_custom_eq = true;
    else {
        f = get_unbound_method(obj, __eq__, &self, false);
        has_custom_eq = f != _t(tp_object)->attr(__eq__);
    }
    if(has_custom_eq) {
        TypeError(_S("unhashable type: ", ti->name.escape()));
    } else {
        return obj.hash();
    }
}

PyVar VM::__py_exec_internal(const CodeObject_& code, PyVar globals, PyVar locals) {
    Frame* frame = &vm->callstack.top();

    // fast path
    if(globals == vm->None && locals == vm->None) {
        return vm->_exec(code.get(), frame->_module, frame->_callable, frame->_locals);
    }

    auto _lock = heap.gc_scope_lock();  // for safety

    PyObject* globals_obj = nullptr;
    Dict* globals_dict = nullptr;

    NameDict_ locals_closure = nullptr;
    Dict* locals_dict = nullptr;

    if(globals == vm->None) {
        globals_obj = frame->_module;
    } else {
        if(is_type(globals, VM::tp_mappingproxy)) {
            globals_obj = PK_OBJ_GET(MappingProxy, globals).obj;
        } else {
            check_compatible_type(globals, VM::tp_dict);
            // make a temporary object and copy globals into it
            globals_obj = new_object<DummyInstance>(VM::tp_object).get();
            globals_obj->_attr = new NameDict();
            globals_dict = &PK_OBJ_GET(Dict, globals);
            globals_dict->apply([&](PyVar k, PyVar v) {
                globals_obj->attr().set(CAST(Str&, k), v);
            });
        }
    }

    PyVar retval = nullptr;

    if(locals == vm->None) {
        retval = vm->_exec(code, globals_obj);  // only globals
    } else {
        check_compatible_type(locals, VM::tp_dict);
        locals_dict = &PK_OBJ_GET(Dict, locals);
        locals_closure = std::make_shared<NameDict>();
        locals_dict->apply([&](PyVar k, PyVar v) {
            locals_closure->set(CAST(Str&, k), v);
        });
        PyObject* _callable =
            heap.gcnew<Function>(tp_function, __dynamic_func_decl, globals_obj, nullptr, locals_closure);
        retval = vm->_exec(code.get(), globals_obj, _callable, vm->s_data._sp);
    }

    if(globals_dict) {
        globals_dict->clear();
        globals_obj->attr().apply([&](StrName k, PyVar v) {
            globals_dict->set(vm, VAR(k.sv()), v);
        });
    }

    if(locals_dict) {
        locals_dict->clear();
        locals_closure->apply([&](StrName k, PyVar v) {
            locals_dict->set(vm, VAR(k.sv()), v);
        });
    }
    return retval;
}

void VM::py_exec(std::string_view source, PyVar globals, PyVar locals) {
    CodeObject_ code = vm->compile(source, "<exec>", EXEC_MODE, true);
    __py_exec_internal(code, globals, locals);
}

PyVar VM::py_eval(std::string_view source, PyVar globals, PyVar locals) {
    CodeObject_ code = vm->compile(source, "<eval>", EVAL_MODE, true);
    return __py_exec_internal(code, globals, locals);
}

PyVar VM::__format_object(PyVar obj, Str spec) {
    if(spec.empty()) return VAR(py_str(obj));
    char type;
    switch(spec.end()[-1]) {
        case 'f':
        case 'd':
        case 's':
            type = spec.end()[-1];
            spec = spec.substr(0, spec.length() - 1);
            break;
        default: type = ' '; break;
    }

    char pad_c = ' ';
    for(char c: std::string_view("0-=*#@!~")) {
        if(spec[0] == c) {
            pad_c = c;
            spec = spec.substr(1);
            break;
        }
    }
    char align;
    if(spec[0] == '^') {
        align = '^';
        spec = spec.substr(1);
    } else if(spec[0] == '>') {
        align = '>';
        spec = spec.substr(1);
    } else if(spec[0] == '<') {
        align = '<';
        spec = spec.substr(1);
    } else {
        if(is_int(obj) || is_float(obj))
            align = '>';
        else
            align = '<';
    }

    int dot = spec.index(".");
    int width, precision;
    try {
        if(dot >= 0) {
            if(dot == 0) {
                width = -1;
            } else {
                width = std::stoi(spec.substr(0, dot).str());
            }
            precision = std::stoi(spec.substr(dot + 1).str());
        } else {
            width = std::stoi(spec.str());
            precision = -1;
        }
    } catch(...) { ValueError("invalid format specifer"); }

    if(type != 'f' && dot >= 0) ValueError("precision not allowed in the format specifier");
    Str ret;
    if(type == 'f') {
        f64 val = CAST(f64, obj);
        if(precision < 0) precision = 6;
        SStream ss;
        ss.setprecision(precision);
        ss << val;
        ret = ss.str();
    } else if(type == 'd') {
        ret = std::to_string(CAST(i64, obj));
    } else if(type == 's') {
        ret = CAST(Str&, obj);
    } else {
        ret = py_str(obj);
    }
    if(width != -1 && width > ret.length()) {
        int pad = width - ret.length();
        if(align == '>' || align == '<') {
            std::string padding(pad, pad_c);
            if(align == '>')
                ret = padding.c_str() + ret;
            else
                ret = ret + padding.c_str();
        } else {  // ^
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
    // convert to fullname
    if(!package.empty()) name = package + "." + name;
    obj->attr().set(__path__, VAR(name));

    // we do not allow override in order to avoid memory leak
    // it is because Module objects are not garbage collected
    if(_modules.contains(name)) { throw std::runtime_error(_S("module ", name.escape(), " already exists").str()); }
    // set it into _modules
    _modules.set(name, obj);
    return obj;
}

static std::string _opcode_argstr(VM* vm, int i, Bytecode byte, const CodeObject* co) {
    SStream ss;
    if(byte.is_forward_jump()) {
        std::string argStr = std::to_string((int16_t)byte.arg);
        ss << (i64)(int16_t)byte.arg;
        ss << " (to " << (i64)((int16_t)byte.arg + i) << ")";
        return ss.str().str();
    }
    ss << (i64)byte.arg;
    switch(byte.op) {
        case OP_LOAD_CONST:
        case OP_FORMAT_STRING:
        case OP_IMPORT_PATH:
            if(vm != nullptr) ss << " (" << vm->py_repr(co->consts[byte.arg]) << ")";
            break;
        case OP_LOAD_NAME:
        case OP_LOAD_GLOBAL:
        case OP_LOAD_NONLOCAL:
        case OP_STORE_GLOBAL:
        case OP_LOAD_ATTR:
        case OP_LOAD_METHOD:
        case OP_STORE_ATTR:
        case OP_DELETE_ATTR:
        case OP_BEGIN_CLASS:
        case OP_GOTO:
        case OP_DELETE_GLOBAL:
        case OP_INC_GLOBAL:
        case OP_DEC_GLOBAL:
        case OP_STORE_CLASS_ATTR:
        case OP_FOR_ITER_STORE_GLOBAL: ss << " (" << StrName(byte.arg).sv() << ")"; break;
        case OP_LOAD_FAST:
        case OP_STORE_FAST:
        case OP_DELETE_FAST:
        case OP_INC_FAST:
        case OP_DEC_FAST:
        case OP_FOR_ITER_STORE_FAST:
        case OP_LOAD_SUBSCR_FAST:
        case OP_STORE_SUBSCR_FAST: ss << " (" << co->varnames[byte.arg].sv() << ")"; break;
        case OP_LOAD_FUNCTION: ss << " (" << co->func_decls[byte.arg]->code->name << ")"; break;
    }
    return ss.str().str();
}

Str VM::disassemble(CodeObject_ co) {
    auto pad = [](const Str& s, const int n) {
        if(s.length() >= n) return s.substr(0, n);
        return s + std::string(n - s.length(), ' ');
    };

    vector<int> jumpTargets;
    for(int i = 0; i < co->codes.size(); i++) {
        Bytecode byte = co->codes[i];
        if(byte.is_forward_jump()) { jumpTargets.push_back((int16_t)byte.arg + i); }
    }
    SStream ss;
    int prev_line = -1;
    for(int i = 0; i < co->codes.size(); i++) {
        const Bytecode& byte = co->codes[i];
        Str line = std::to_string(co->lines[i].lineno);
        if(co->lines[i].lineno == prev_line)
            line = "";
        else {
            if(prev_line != -1) ss << "\n";
            prev_line = co->lines[i].lineno;
        }

        std::string pointer;
        if(jumpTargets.contains(i)) {
            pointer = "-> ";
        } else {
            pointer = "   ";
        }
        ss << pad(line, 8) << pointer << pad(std::to_string(i), 3);
        std::string bc_name(OP_NAMES[byte.op]);
        if(co->lines[i].is_virtual) bc_name += '*';
        ss << " " << pad(bc_name, 25) << " ";
        std::string argStr = _opcode_argstr(this, i, byte, co.get());
        ss << argStr;
        if(i != co->codes.size() - 1) ss << '\n';
    }

    for(auto& decl: co->func_decls) {
        ss << "\n\n"
           << "Disassembly of " << decl->code->name << ":\n";
        ss << disassemble(decl->code);
    }
    ss << "\n";
    return Str(ss.str());
}

#if PK_DEBUG_CEVAL_STEP
void VM::__log_s_data(const char* title) {
    if(_main == nullptr) return;
    if(callstack.empty()) return;
    SStream ss;
    if(title) ss << title << " | ";
    std::map<PyVar*, int> sp_bases;
    callstack.apply([&](Frame& f) {
        assert(f._sp_base != nullptr);
        sp_bases[f._sp_base] += 1;
    });
    Frame* frame = &callstack.top();
    int line = frame->curr_lineno();
    ss << frame->co->name << ":" << line << " [";
    for(PyVar* p = s_data.begin(); p != s_data.end(); p++) {
        ss << std::string(sp_bases[p], '|');
        if(sp_bases[p] > 0) ss << " ";
        if(*p == PY_NULL)
            ss << "NULL";
        else {
            switch(p->type) {
                case tp_none_type: ss << "None"; break;
                case tp_int: ss << _CAST(i64, *p); break;
                case tp_float: ss << _CAST(f64, *p); break;
                case tp_bool: ss << ((*p == True) ? "True" : "False"); break;
                case tp_str: ss << _CAST(Str, *p).escape(); break;
                case tp_function: ss << p->obj_get<Function>().decl->code->name << "()"; break;
                case tp_type: ss << "<class " + _type_name(this, p->obj_get<Type>()).escape() + ">"; break;
                case tp_list: ss << "list(size=" << p->obj_get<List>().size() << ")"; break;
                case tp_tuple: ss << "tuple(size=" << p->obj_get<Tuple>().size() << ")"; break;
                case tp_stack_memory: {
                    int count = p->obj_get<StackMemory>().count;
                    ss << "M[" << count << "]";
                    if(count > 0) p += count;
                    break;
                }
                default: ss << "(" << _type_name(this, p->type) << ")"; break;
            }
        }
        ss << ", ";
    }
    std::string output = ss.str().str();
    if(!s_data.empty()) {
        output.pop_back();
        output.pop_back();
    }
    output.push_back(']');
    Bytecode byte = *frame->_ip;
    std::cout << output << " " << OP_NAMES[byte.op] << " " << _opcode_argstr(nullptr, frame->ip(), byte, frame->co)
              << std::endl;
}
#endif

void VM::__init_builtin_types() {
    _all_types.emplace_back(nullptr, Type(), nullptr, "", false);  // 0 is not used
    _all_types.emplace_back(heap._new<Type>(tp_type, tp_object), Type(), nullptr, "object", true);
    _all_types.emplace_back(heap._new<Type>(tp_type, tp_type), tp_object, nullptr, "type", false);

    auto validate = [](Type type, PyObject* ret) {
        Type ret_t = ret->as<Type>();
        if(ret_t != type) exit(-3);
    };

    validate(tp_int, new_type_object(nullptr, "int", tp_object, false));
    validate(tp_float, new_type_object(nullptr, "float", tp_object, false));
    validate(tp_bool, new_type_object(nullptr, "bool", tp_object, false));

    validate(tp_str, new_type_object<Str>(nullptr, "str", tp_object, false));
    validate(tp_list, new_type_object<List>(nullptr, "list", tp_object, false));
    validate(tp_tuple, new_type_object<Tuple>(nullptr, "tuple", tp_object, false));

    validate(tp_slice, new_type_object<Slice>(nullptr, "slice", tp_object, false));
    validate(tp_range, new_type_object<Range>(nullptr, "range", tp_object, false));
    validate(tp_module, new_type_object<DummyModule>(nullptr, "module", tp_object, false));
    validate(tp_function, new_type_object<Function>(nullptr, "function", tp_object, false));
    validate(tp_native_func, new_type_object<NativeFunc>(nullptr, "native_func", tp_object, false));
    validate(tp_bound_method, new_type_object<BoundMethod>(nullptr, "bound_method", tp_object, false));

    validate(tp_super, new_type_object<Super>(nullptr, "super", tp_object, false));
    validate(tp_exception, new_type_object<Exception>(nullptr, "Exception", tp_object, true));
    validate(tp_bytes, new_type_object<Bytes>(nullptr, "bytes", tp_object, false));
    validate(tp_mappingproxy, new_type_object<MappingProxy>(nullptr, "mappingproxy", tp_object, false));
    validate(tp_dict, new_type_object<Dict>(nullptr, "dict", tp_object, true));
    validate(tp_property, new_type_object<Property>(nullptr, "property", tp_object, false));
    validate(tp_star_wrapper, new_type_object<StarWrapper>(nullptr, "_star_wrapper", tp_object, false));

    validate(tp_staticmethod, new_type_object<StaticMethod>(nullptr, "staticmethod", tp_object, false));
    validate(tp_classmethod, new_type_object<ClassMethod>(nullptr, "classmethod", tp_object, false));

    validate(tp_none_type, new_type_object(nullptr, "NoneType", tp_object, false));
    validate(tp_not_implemented, new_type_object(nullptr, "NotImplementedType", tp_object, false));
    validate(tp_ellipsis, new_type_object(nullptr, "ellipsis", tp_object, false));
    validate(tp_stack_memory, new_type_object<StackMemory>(nullptr, "_stack_memory", tp_object, false));

    // SyntaxError and IndentationError must be created here
    PyObject* SyntaxError = new_type_object(nullptr, "SyntaxError", tp_exception, true);
    PyObject* IndentationError = new_type_object(nullptr, "IndentationError", SyntaxError->as<Type>(), true);
    this->StopIteration = new_type_object(nullptr, "StopIteration", tp_exception, true);

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
    builtins->attr().set("Exception", _t(tp_exception));
    builtins->attr().set("SyntaxError", SyntaxError);
    builtins->attr().set("IndentationError", IndentationError);

    __post_init_builtin_types();
    this->_main = new_module("__main__");
}

void VM::__unpack_as_list(ArgsView args, List& list) {
    auto _lock = heap.gc_scope_lock();
    for(PyVar obj: args) {
        if(is_type(obj, tp_star_wrapper)) {
            const StarWrapper& w = _CAST(StarWrapper&, obj);
            // maybe this check should be done in the compile time
            if(w.level != 1) TypeError("expected level 1 star wrapper");
            PyVar _0 = py_iter(w.obj);
            const PyTypeInfo* info = _tp_info(_0);
            PyVar _1 = _py_next(info, _0);
            while(_1 != StopIteration) {
                list.push_back(_1);
                _1 = _py_next(info, _0);
            }
        } else {
            list.push_back(obj);
        }
    }
}

void VM::__unpack_as_dict(ArgsView args, Dict& dict) {
    auto _lock = heap.gc_scope_lock();
    for(PyVar obj: args) {
        if(is_type(obj, tp_star_wrapper)) {
            const StarWrapper& w = _CAST(StarWrapper&, obj);
            // maybe this check should be done in the compile time
            if(w.level != 2) TypeError("expected level 2 star wrapper");
            const Dict& other = CAST(Dict&, w.obj);
            dict.update(this, other);
        } else {
            const Tuple& t = CAST(Tuple&, obj);
            if(t.size() != 2) TypeError("expected tuple of length 2");
            dict.set(this, t[0], t[1]);
        }
    }
}

void VM::__prepare_py_call(PyVar* buffer, ArgsView args, ArgsView kwargs, const FuncDecl_& decl) {
    const CodeObject* co = decl->code.get();
    int decl_argc = decl->args.size();

    if(args.size() < decl_argc) {
        vm->TypeError(_S(co->name, "() takes ", decl_argc, " positional arguments but ", args.size(), " were given"));
    }

    int i = 0;
    // prepare args
    std::memset(buffer, 0, co->nlocals * sizeof(PyVar));
    for(int index: decl->args)
        buffer[index] = args[i++];
    // prepare kwdefaults
    for(auto& kv: decl->kwargs)
        buffer[kv.index] = kv.value;

    // handle *args
    if(decl->starred_arg != -1) {
        ArgsView vargs(args.begin() + i, args.end());
        buffer[decl->starred_arg] = VAR(vargs.to_tuple());
        i += vargs.size();
    } else {
        // kwdefaults override
        for(auto& kv: decl->kwargs) {
            if(i >= args.size()) break;
            buffer[kv.index] = args[i++];
        }
        if(i < args.size()) TypeError(_S("too many arguments", " (", decl->code->name, ')'));
    }

    PyVar vkwargs;
    if(decl->starred_kwarg != -1) {
        vkwargs = VAR(Dict());
        buffer[decl->starred_kwarg] = vkwargs;
    } else {
        vkwargs = nullptr;
    }

    for(int j = 0; j < kwargs.size(); j += 2) {
        StrName key(_CAST(uint16_t, kwargs[j]));
        int index = decl->kw_to_index.try_get_likely_found(key);
        // if key is an explicit key, set as local variable
        if(index >= 0) {
            buffer[index] = kwargs[j + 1];
        } else {
            // otherwise, set as **kwargs if possible
            if(vkwargs == nullptr) {
                TypeError(_S(key.escape(), " is an invalid keyword argument for ", co->name, "()"));
            } else {
                Dict& dict = _CAST(Dict&, vkwargs);
                dict.set(this, VAR(key.sv()), kwargs[j + 1]);
            }
        }
    }
}

PyVar VM::vectorcall(int ARGC, int KWARGC, bool op_call) {
    PyVar* p1 = s_data._sp - KWARGC * 2;
    PyVar* p0 = p1 - ARGC - 2;
    // [callable, <self>, args..., kwargs...]
    //      ^p0                    ^p1      ^_sp
    PyVar callable = p1[-ARGC - 2];
    Type callable_t = _tp(callable);

    // handle boundmethod, do a patch
    if(callable_t == tp_bound_method) {
        assert(p0[1] == PY_NULL);
        BoundMethod& bm = PK_OBJ_GET(BoundMethod, callable);
        callable = bm.func;  // get unbound method
        callable_t = _tp(callable);
        p1[-(ARGC + 2)] = bm.func;
        p1[-(ARGC + 1)] = bm.self;
        // [unbound, self, args..., kwargs...]
    }

    ArgsView args(p0[1] == PY_NULL ? (p0 + 2) : (p0 + 1), p1);
    ArgsView kwargs(p1, s_data._sp);

    PyVar* _base = args.begin();

    if(callable_t == tp_function) {
        /*****************_py_call*****************/
        // check stack overflow
        if(s_data.is_overflow()) StackOverflowError();

        const Function& fn = PK_OBJ_GET(Function, callable);
        const CodeObject* co = fn.decl->code.get();

        switch(fn.decl->type) {
            case FuncType::NORMAL:
                __prepare_py_call(__vectorcall_buffer, args, kwargs, fn.decl);
                // copy buffer back to stack
                s_data.reset(_base + co->nlocals);
                for(int j = 0; j < co->nlocals; j++)
                    _base[j] = __vectorcall_buffer[j];
                break;
            case FuncType::SIMPLE:
                if(args.size() != fn.decl->args.size())
                    TypeError(_S(co->name,
                                 "() takes ",
                                 fn.decl->args.size(),
                                 " positional arguments but ",
                                 args.size(),
                                 " were given"));
                if(!kwargs.empty()) TypeError(_S(co->name, "() takes no keyword arguments"));
                // [callable, <self>, args..., local_vars...]
                //      ^p0                    ^p1      ^_sp
                s_data.reset(_base + co->nlocals);
                // initialize local variables to PY_NULL
                std::memset(p1, 0, (char*)s_data._sp - (char*)p1);
                break;
            case FuncType::EMPTY:
                if(args.size() != fn.decl->args.size())
                    TypeError(_S(co->name,
                                 "() takes ",
                                 fn.decl->args.size(),
                                 " positional arguments but ",
                                 args.size(),
                                 " were given"));
                if(!kwargs.empty()) TypeError(_S(co->name, "() takes no keyword arguments"));
                s_data.reset(p0);
                return None;
            case FuncType::GENERATOR:
                __prepare_py_call(__vectorcall_buffer, args, kwargs, fn.decl);
                s_data.reset(p0);
                callstack.emplace(nullptr, co, fn._module, callable.get(), nullptr);
                return __py_generator(callstack.popx(),
                                      ArgsView(__vectorcall_buffer, __vectorcall_buffer + co->nlocals));
            default: PK_UNREACHABLE()
        };

        // simple or normal
        callstack.emplace(p0, co, fn._module, callable.get(), args.begin());
        if(op_call) return PY_OP_CALL;
        return __run_top_frame();
        /*****************_py_call*****************/
    }

    if(callable_t == tp_native_func) {
        const auto& f = PK_OBJ_GET(NativeFunc, callable);
        PyVar ret;
        if(f.decl != nullptr) {
            int co_nlocals = f.decl->code->nlocals;
            __prepare_py_call(__vectorcall_buffer, args, kwargs, f.decl);
            // copy buffer back to stack
            s_data.reset(_base + co_nlocals);
            for(int j = 0; j < co_nlocals; j++)
                _base[j] = __vectorcall_buffer[j];
            ret = f.call(vm, ArgsView(s_data._sp - co_nlocals, s_data._sp));
        } else {
            if(f.argc != -1) {
                if(KWARGC != 0)
                    TypeError(
                        "old-style native_func does not accept keyword arguments. If you want to skip this check, specify `argc` to -1");
                if(args.size() != f.argc) { vm->TypeError(_S("expected ", f.argc, " arguments, got ", args.size())); }
            }
            ret = f.call(this, args);
        }
        s_data.reset(p0);
        return ret;
    }

    if(callable_t == tp_type) {
        // [type, NULL, args..., kwargs...]
        PyVar new_f = *find_name_in_mro(PK_OBJ_GET(Type, callable), __new__);
        PyVar obj;
        assert(new_f != nullptr && p0[1] == PY_NULL);
        if(new_f == __cached_object_new) {
            // fast path for object.__new__
            obj = vm->new_object<DummyInstance>(PK_OBJ_GET(Type, callable));
        } else {
            PUSH(new_f);
            PUSH(PY_NULL);
            PUSH(callable);  // cls
            for(PyVar o: args)
                PUSH(o);
            for(PyVar o: kwargs)
                PUSH(o);
            // if obj is not an instance of `cls`, the behavior is undefined
            obj = vectorcall(ARGC + 1, KWARGC);
        }

        // __init__
        PyVar self;
        callable = get_unbound_method(obj, __init__, &self, false);
        if(callable != nullptr) {
            callable_t = _tp(callable);
            // replace `NULL` with `self`
            p1[-(ARGC + 2)] = callable;
            p1[-(ARGC + 1)] = self;
            // [init_f, self, args..., kwargs...]
            vectorcall(ARGC, KWARGC);
            // We just discard the return value of `__init__`
            // in cpython it raises a TypeError if the return value is not None
        } else {
            // manually reset the stack
            s_data.reset(p0);
        }
        return obj;
    }

    // handle `__call__` overload
    PyVar self;
    PyVar call_f = get_unbound_method(callable, __call__, &self, false);
    if(self != PY_NULL) {
        p1[-(ARGC + 2)] = call_f;
        p1[-(ARGC + 1)] = self;
        // [call_f, self, args..., kwargs...]
        return vectorcall(ARGC, KWARGC, op_call);
    }
    TypeError(_type_name(vm, callable_t).escape() + " object is not callable");
}

void VM::delattr(PyVar _0, StrName _name) {
    const PyTypeInfo* ti = _tp_info(_0);
    if(ti->m__delattr__ && ti->m__delattr__(this, _0, _name)) return;
    if(is_tagged(_0) || !_0->is_attr_valid()) TypeError("cannot delete attribute");
    if(!_0->attr().del(_name)) AttributeError(_0, _name);
}

// https://docs.python.org/3/howto/descriptor.html#invocation-from-an-instance
PyVar VM::getattr(PyVar obj, StrName name, bool throw_err) {
    Type objtype(0);
    // handle super() proxy
    if(is_type(obj, tp_super)) {
        const Super& super = PK_OBJ_GET(Super, obj);
        obj = super.first;
        objtype = super.second;
    } else {
        objtype = _tp(obj);
    }
    PyVar* cls_var = find_name_in_mro(objtype, name);
    if(cls_var != nullptr) {
        // handle descriptor
        if(is_type(*cls_var, tp_property)) {
            const Property& prop = PK_OBJ_GET(Property, *cls_var);
            return call(prop.getter, obj);
        }
    }
    // handle instance __dict__
    if(!is_tagged(obj) && obj->is_attr_valid()) {
        PyVar* val;
        if(obj.type == tp_type) {
            val = find_name_in_mro(PK_OBJ_GET(Type, obj), name);
            if(val != nullptr) {
                if(is_tagged(*val)) return *val;
                if(val->type == tp_staticmethod) return PK_OBJ_GET(StaticMethod, *val).func;
                if(val->type == tp_classmethod) return VAR(BoundMethod(obj, PK_OBJ_GET(ClassMethod, *val).func));
                return *val;
            }
        } else {
            val = obj->attr().try_get_2_likely_found(name);
            if(val != nullptr) return *val;
        }
    }
    if(cls_var != nullptr) {
        // bound method is non-data descriptor
        if(!is_tagged(*cls_var)) {
            switch(cls_var->type.index) {
                case tp_function.index: return VAR(BoundMethod(obj, *cls_var));
                case tp_native_func.index: return VAR(BoundMethod(obj, *cls_var));
                case tp_staticmethod.index: return PK_OBJ_GET(StaticMethod, *cls_var).func;
                case tp_classmethod.index: return VAR(BoundMethod(_t(objtype), PK_OBJ_GET(ClassMethod, *cls_var).func));
            }
        }
        return *cls_var;
    }

    const PyTypeInfo* ti = &_all_types[objtype];
    if(ti->m__getattr__) {
        PyVar ret = ti->m__getattr__(this, obj, name);
        if(ret) return ret;
    }

    if(throw_err) AttributeError(obj, name);
    return nullptr;
}

// used by OP_LOAD_METHOD
// try to load a unbound method (fallback to `getattr` if not found)
PyVar VM::get_unbound_method(PyVar obj, StrName name, PyVar* self, bool throw_err, bool fallback) {
    self->set_null();
    Type objtype(0);
    // handle super() proxy
    if(is_type(obj, tp_super)) {
        const Super& super = PK_OBJ_GET(Super, obj);
        obj = super.first;
        objtype = super.second;
    } else {
        objtype = _tp(obj);
    }
    PyVar* cls_var = find_name_in_mro(objtype, name);

    if(fallback) {
        if(cls_var != nullptr) {
            // handle descriptor
            if(is_type(*cls_var, tp_property)) {
                const Property& prop = PK_OBJ_GET(Property, *cls_var);
                return call(prop.getter, obj);
            }
        }
        // handle instance __dict__
        if(!is_tagged(obj) && obj->is_attr_valid()) {
            PyVar* val;
            if(obj.type == tp_type) {
                val = find_name_in_mro(PK_OBJ_GET(Type, obj), name);
                if(val != nullptr) {
                    if(is_tagged(*val)) return *val;
                    if(val->type == tp_staticmethod) return PK_OBJ_GET(StaticMethod, *val).func;
                    if(val->type == tp_classmethod) return VAR(BoundMethod(obj, PK_OBJ_GET(ClassMethod, *val).func));
                    return *val;
                }
            } else {
                val = obj->attr().try_get_2_likely_found(name);
                if(val != nullptr) return *val;
            }
        }
    }

    if(cls_var != nullptr) {
        if(!is_tagged(*cls_var)) {
            switch(cls_var->type.index) {
                case tp_function.index: *self = obj; break;
                case tp_native_func.index: *self = obj; break;
                case tp_staticmethod.index: self->set_null(); return PK_OBJ_GET(StaticMethod, *cls_var).func;
                case tp_classmethod.index: *self = _t(objtype); return PK_OBJ_GET(ClassMethod, *cls_var).func;
            }
        }
        return *cls_var;
    }

    const PyTypeInfo* ti = &_all_types[objtype];
    if(fallback && ti->m__getattr__) {
        PyVar ret = ti->m__getattr__(this, obj, name);
        if(ret) return ret;
    }

    if(throw_err) AttributeError(obj, name);
    return nullptr;
}

void VM::setattr(PyVar obj, StrName name, PyVar value) {
    Type objtype(0);
    // handle super() proxy
    if(is_type(obj, tp_super)) {
        Super& super = PK_OBJ_GET(Super, obj);
        obj = super.first;
        objtype = super.second;
    } else {
        objtype = _tp(obj);
    }
    PyVar* cls_var = find_name_in_mro(objtype, name);
    if(cls_var != nullptr) {
        // handle descriptor
        if(is_type(*cls_var, tp_property)) {
            const Property& prop = _CAST(Property&, *cls_var);
            if(prop.setter != vm->None) {
                call(prop.setter, obj, value);
            } else {
                TypeError(_S("readonly attribute: ", name.escape()));
            }
            return;
        }
    }

    const PyTypeInfo* ti = &_all_types[objtype];
    if(ti->m__setattr__) {
        ti->m__setattr__(this, obj, name, value);
        return;
    }

    // handle instance __dict__
    if(is_tagged(obj) || !obj->is_attr_valid()) TypeError("cannot set attribute");
    obj->attr().set(name, value);
}

PyObject* VM::bind_func(PyObject* obj, StrName name, int argc, NativeFuncC fn, any userdata, BindType bt) {
    PyObject* nf = heap.gcnew<NativeFunc>(tp_native_func, fn, argc, std::move(userdata));
    switch(bt) {
        case BindType::DEFAULT: break;
        case BindType::STATICMETHOD: nf = heap.gcnew<StaticMethod>(tp_staticmethod, nf); break;
        case BindType::CLASSMETHOD: nf = heap.gcnew<ClassMethod>(tp_classmethod, nf); break;
    }
    if(obj != nullptr) obj->attr().set(name, nf);
    return nf;
}

PyObject* VM::bind(PyObject* obj, const char* sig, NativeFuncC fn, any userdata, BindType bt) {
    return bind(obj, sig, nullptr, fn, std::move(userdata), bt);
}

PyObject* VM::bind(PyObject* obj, const char* sig, const char* docstring, NativeFuncC fn, any userdata, BindType bt) {
    CodeObject_ co;
    try {
        // fn(a, b, *c, d=1) -> None
        co = compile(_S("def ", sig, " : pass"), "<bind>", EXEC_MODE);
    } catch(TopLevelException) { throw std::runtime_error("invalid signature: " + std::string(sig)); }
    if(co->func_decls.size() != 1) { throw std::runtime_error("expected 1 function declaration"); }
    FuncDecl_ decl = co->func_decls[0];
    decl->docstring = docstring;
    PyObject* f_obj = heap.gcnew<NativeFunc>(tp_native_func, fn, decl, std::move(userdata));

    switch(bt) {
        case BindType::STATICMETHOD: f_obj = heap.gcnew<StaticMethod>(tp_staticmethod, f_obj); break;
        case BindType::CLASSMETHOD: f_obj = heap.gcnew<ClassMethod>(tp_classmethod, f_obj); break;
        case BindType::DEFAULT: break;
    }
    if(obj != nullptr) obj->attr().set(decl->code->name, f_obj);
    return f_obj;
}

PyObject* VM::bind_property(PyObject* obj, const char* name, NativeFuncC fget, NativeFuncC fset) {
    assert(is_type(obj, tp_type));
    std::string_view name_sv(name);
    int pos = name_sv.find(':');
    if(pos > 0) name_sv = name_sv.substr(0, pos);
    PyVar _0 = new_object<NativeFunc>(tp_native_func, fget, 1);
    PyVar _1 = vm->None;
    if(fset != nullptr) _1 = new_object<NativeFunc>(tp_native_func, fset, 2);
    PyObject* prop = heap.gcnew<Property>(tp_property, _0, _1);
    obj->attr().set(StrName(name_sv), prop);
    return prop;
}

void VM::__builtin_error(StrName type) { _error(call(builtins->attr(type))); }

void VM::__builtin_error(StrName type, PyVar arg) { _error(call(builtins->attr(type), arg)); }

void VM::__builtin_error(StrName type, const Str& msg) { __builtin_error(type, VAR(msg)); }

void VM::BinaryOptError(const char* op, PyVar _0, PyVar _1) {
    StrName name_0 = _type_name(vm, _tp(_0));
    StrName name_1 = _type_name(vm, _tp(_1));
    TypeError(_S("unsupported operand type(s) for ", op, ": ", name_0.escape(), " and ", name_1.escape()));
}

void VM::AttributeError(PyVar obj, StrName name) {
    if(isinstance(obj, vm->tp_type)) {
        __builtin_error(
            "AttributeError",
            _S("type object ", _type_name(vm, PK_OBJ_GET(Type, obj)).escape(), " has no attribute ", name.escape()));
    } else {
        __builtin_error("AttributeError",
                        _S(_type_name(vm, _tp(obj)).escape(), " object has no attribute ", name.escape()));
    }
}

void VM::_error(PyVar e_obj) {
    assert(isinstance(e_obj, tp_exception));
    Exception& e = PK_OBJ_GET(Exception, e_obj);
    if(callstack.empty()) {
        e.is_re = false;
        __last_exception = e_obj.get();
        throw TopLevelException(this, &e);
    }
    PUSH(e_obj);
    __raise_exc();
}

void VM::__raise_exc(bool re_raise) {
    Frame* frame = &callstack.top();
    Exception& e = PK_OBJ_GET(Exception, s_data.top());
    if(!re_raise) {
        e._ip_on_error = frame->ip();
        e._code_on_error = (void*)frame->co;
    }
    int next_ip = frame->prepare_jump_exception_handler(&s_data);

    int actual_ip = frame->ip();
    if(e._ip_on_error >= 0 && e._code_on_error == (void*)frame->co) actual_ip = e._ip_on_error;
    int current_line = frame->co->lines[actual_ip].lineno;  // current line
    auto current_f_name = frame->co->name.sv();             // current function name
    if(frame->_callable == nullptr) current_f_name = "";    // not in a function
    e.st_push(frame->co->src, current_line, nullptr, current_f_name);

    if(next_ip >= 0) {
        throw InternalException(InternalExceptionType::Handled, next_ip);
    } else {
        throw InternalException(InternalExceptionType::Unhandled);
    }
}

StrName _type_name(VM* vm, Type type) { return vm->_all_types[type].name; }

void VM::bind__getitem__(Type type, PyVar (*f)(VM*, PyVar, PyVar)) {
    _all_types[type].m__getitem__ = f;
    bind_func(
        type,
        __getitem__,
        2,
        [](VM* vm, ArgsView args) {
            return lambda_get_userdata<decltype(f)>(args.begin())(vm, args[0], args[1]);
        },
        f);
}

void VM::bind__setitem__(Type type, void (*f)(VM*, PyVar, PyVar, PyVar)) {
    _all_types[type].m__setitem__ = f;
    bind_func(
        type,
        __setitem__,
        3,
        [](VM* vm, ArgsView args) {
            lambda_get_userdata<decltype(f)>(args.begin())(vm, args[0], args[1], args[2]);
            return vm->None;
        },
        f);
}

void VM::bind__delitem__(Type type, void (*f)(VM*, PyVar, PyVar)) {
    _all_types[type].m__delitem__ = f;
    bind_func(
        type,
        __delitem__,
        2,
        [](VM* vm, ArgsView args) {
            lambda_get_userdata<decltype(f)>(args.begin())(vm, args[0], args[1]);
            return vm->None;
        },
        f);
}

PyVar VM::__pack_next_retval(unsigned n) {
    if(n == 0) return StopIteration;
    if(n == 1) return s_data.popx();
    PyVar retval = VAR(s_data.view(n).to_tuple());
    s_data._sp -= n;
    return retval;
}

void VM::bind__next__(Type type, unsigned (*f)(VM*, PyVar)) {
    _all_types[type].op__next__ = f;
    bind_func(
        type,
        __next__,
        1,
        [](VM* vm, ArgsView args) {
            int n = lambda_get_userdata<decltype(f)>(args.begin())(vm, args[0]);
            return vm->__pack_next_retval(n);
        },
        f);
}

void VM::bind__next__(Type type, PyVar (*f)(VM*, PyVar)) {
    bind_func(
        type,
        __next__,
        1,
        [](VM* vm, ArgsView args) {
            auto f = lambda_get_userdata<PyVar (*)(VM*, PyVar)>(args.begin());
            return f(vm, args[0]);
        },
        f);
}

#define BIND_UNARY_SPECIAL(name)                                                                                       \
    void VM::bind##name(Type type, PyVar (*f)(VM*, PyVar)) {                                                           \
        _all_types[type].m##name = f;                                                                                  \
        bind_func(                                                                                                     \
            type,                                                                                                      \
            name,                                                                                                      \
            1,                                                                                                         \
            [](VM* vm, ArgsView args) {                                                                                \
                return lambda_get_userdata<PyVar (*)(VM*, PyVar)>(args.begin())(vm, args[0]);                          \
            },                                                                                                         \
            f);                                                                                                        \
    }
BIND_UNARY_SPECIAL(__iter__)
BIND_UNARY_SPECIAL(__neg__)
BIND_UNARY_SPECIAL(__invert__)
#undef BIND_UNARY_SPECIAL

void VM::bind__str__(Type type, Str (*f)(VM*, PyVar)) {
    _all_types[type].m__str__ = f;
    bind_func(
        type,
        __str__,
        1,
        [](VM* vm, ArgsView args) {
            Str s = lambda_get_userdata<decltype(f)>(args.begin())(vm, args[0]);
            return VAR(s);
        },
        f);
}

void VM::bind__repr__(Type type, Str (*f)(VM*, PyVar)) {
    _all_types[type].m__repr__ = f;
    bind_func(
        type,
        __repr__,
        1,
        [](VM* vm, ArgsView args) {
            Str s = lambda_get_userdata<decltype(f)>(args.begin())(vm, args[0]);
            return VAR(s);
        },
        f);
}

void VM::bind__hash__(Type type, i64 (*f)(VM*, PyVar)) {
    _all_types[type].m__hash__ = f;
    bind_func(
        type,
        __hash__,
        1,
        [](VM* vm, ArgsView args) {
            i64 ret = lambda_get_userdata<decltype(f)>(args.begin())(vm, args[0]);
            return VAR(ret);
        },
        f);
}

void VM::bind__len__(Type type, i64 (*f)(VM*, PyVar)) {
    _all_types[type].m__len__ = f;
    bind_func(
        type,
        __len__,
        1,
        [](VM* vm, ArgsView args) {
            i64 ret = lambda_get_userdata<decltype(f)>(args.begin())(vm, args[0]);
            return VAR(ret);
        },
        f);
}

#define BIND_BINARY_SPECIAL(name)                                                                                      \
    void VM::bind##name(Type type, BinaryFuncC f) {                                                                    \
        _all_types[type].m##name = f;                                                                                  \
        bind_func(                                                                                                     \
            type,                                                                                                      \
            name,                                                                                                      \
            2,                                                                                                         \
            [](VM* vm, ArgsView args) {                                                                                \
                return lambda_get_userdata<BinaryFuncC>(args.begin())(vm, args[0], args[1]);                           \
            },                                                                                                         \
            f);                                                                                                        \
    }

BIND_BINARY_SPECIAL(__eq__)
BIND_BINARY_SPECIAL(__lt__)
BIND_BINARY_SPECIAL(__le__)
BIND_BINARY_SPECIAL(__gt__)
BIND_BINARY_SPECIAL(__ge__)
BIND_BINARY_SPECIAL(__contains__)

BIND_BINARY_SPECIAL(__add__)
BIND_BINARY_SPECIAL(__sub__)
BIND_BINARY_SPECIAL(__mul__)
BIND_BINARY_SPECIAL(__truediv__)
BIND_BINARY_SPECIAL(__floordiv__)
BIND_BINARY_SPECIAL(__mod__)
BIND_BINARY_SPECIAL(__pow__)
BIND_BINARY_SPECIAL(__matmul__)

BIND_BINARY_SPECIAL(__lshift__)
BIND_BINARY_SPECIAL(__rshift__)
BIND_BINARY_SPECIAL(__and__)
BIND_BINARY_SPECIAL(__or__)
BIND_BINARY_SPECIAL(__xor__)

#undef BIND_BINARY_SPECIAL

void Dict::_probe_0(VM* vm, PyVar key, bool& ok, int& i) const {
    ok = false;
    i64 hash = vm->py_hash(key);
    i = hash & _mask;
    for(int j = 0; j < _capacity; j++) {
        if(_items[i].first != nullptr) {
            if(vm->py_eq(_items[i].first, key)) {
                ok = true;
                break;
            }
        } else {
            if(_items[i].second == nullptr) break;
        }
        // https://github.com/python/cpython/blob/3.8/Objects/dictobject.c#L166
        i = ((5 * i) + 1) & _mask;
    }
}

void Dict::_probe_1(VM* vm, PyVar key, bool& ok, int& i) const {
    ok = false;
    i = vm->py_hash(key) & _mask;
    while(_items[i].first != nullptr) {
        if(vm->py_eq(_items[i].first, key)) {
            ok = true;
            break;
        }
        // https://github.com/python/cpython/blob/3.8/Objects/dictobject.c#L166
        i = ((5 * i) + 1) & _mask;
    }
}

#if PK_ENABLE_PROFILER
void NextBreakpoint::_step(VM* vm) {
    int curr_callstack_size = vm->callstack.size();
    int curr_lineno = vm->callstack.top().curr_lineno();
    if(should_step_into) {
        if(curr_callstack_size != callstack_size || curr_lineno != lineno) { vm->__breakpoint(); }
    } else {
        if(curr_callstack_size == callstack_size) {
            if(curr_lineno != lineno) vm->__breakpoint();
        } else if(curr_callstack_size < callstack_size) {
            // returning
            vm->__breakpoint();
        }
    }
}
#endif

void VM::__pop_frame() {
    s_data.reset(callstack.top()._sp_base);
    callstack.pop();

#if PK_ENABLE_PROFILER
    if(!_next_breakpoint.empty() && callstack.size() < _next_breakpoint.callstack_size) {
        _next_breakpoint = NextBreakpoint();
    }
#endif
}

void VM::__breakpoint() {
#if PK_ENABLE_PROFILER
    _next_breakpoint = NextBreakpoint();

    bool show_where = false;
    bool show_headers = true;

    while(true) {
        vector<LinkedFrame*> frames;
        LinkedFrame* lf = callstack._tail;
        while(lf != nullptr) {
            frames.push_back(lf);
            lf = lf->f_back;
            if(frames.size() >= 4) break;
        }

        if(show_headers) {
            for(int i = frames.size() - 1; i >= 0; i--) {
                if(!show_where && i != 0) continue;

                SStream ss;
                Frame* frame = &frames[i]->frame;
                int lineno = frame->curr_lineno();
                ss << "File \"" << frame->co->src->filename << "\", line " << lineno;
                if(frame->_callable) {
                    ss << ", in ";
                    ss << frame->_callable->as<Function>().decl->code->name;
                }
                ss << '\n';
                ss << "-> " << frame->co->src->get_line(lineno) << '\n';
                stdout_write(ss.str());
            }
            show_headers = false;
        }

        vm->stdout_write("(Pdb) ");
        Frame* frame_0 = &frames[0]->frame;

        std::string line;
        if(!std::getline(std::cin, line)) {
            stdout_write("--KeyboardInterrupt--\n");
            continue;
        }

        if(line == "h" || line == "help") {
            stdout_write("h, help: show this help message\n");
            stdout_write("q, quit: exit the debugger\n");
            stdout_write("n, next: execute next line\n");
            stdout_write("s, step: step into\n");
            stdout_write("w, where: show current stack frame\n");
            stdout_write("c, continue: continue execution\n");
            stdout_write("a, args: show local variables\n");
            stdout_write("p, print <expr>: evaluate expression\n");
            stdout_write("l, list: show lines around current line\n");
            stderr_write("ll, longlist: show all lines\n");
            stdout_write("!: execute statement\n");
            continue;
        }
        if(line == "q" || line == "quit") { vm->RuntimeError("pdb quit"); }
        if(line == "n" || line == "next") {
            vm->_next_breakpoint = NextBreakpoint(vm->callstack.size(), frame_0->curr_lineno(), false);
            break;
        }
        if(line == "s" || line == "step") {
            vm->_next_breakpoint = NextBreakpoint(vm->callstack.size(), frame_0->curr_lineno(), true);
            break;
        }
        if(line == "w" || line == "where") {
            show_where = !show_where;
            show_headers = true;
            continue;
        }
        if(line == "c" || line == "continue") break;
        if(line == "a" || line == "args") {
            int i = 0;
            for(PyVar obj: frame_0->_locals) {
                if(obj == PY_NULL) continue;
                StrName name = frame_0->co->varnames[i++];
                stdout_write(_S(name.sv(), " = ", vm->py_repr(obj), '\n'));
            }
            continue;
        }

        bool is_list = line == "l" || line == "list";
        bool is_longlist = line == "ll" || line == "longlist";

        if(is_list || is_longlist) {
            if(frame_0->co->src->is_precompiled) continue;
            int lineno = frame_0->curr_lineno();
            int start, end;

            if(is_list) {
                int max_line = frame_0->co->src->line_starts.size() + 1;
                start = (std::max)(1, lineno - 5);
                end = (std::min)(max_line, lineno + 5);
            } else {
                start = frame_0->co->start_line;
                end = frame_0->co->end_line;
                if(start == -1 || end == -1) continue;
            }

            SStream ss;
            int max_width = std::to_string(end).size();
            for(int i = start; i <= end; i++) {
                int spaces = max_width - std::to_string(i).size();
                ss << std::string(spaces, ' ') << std::to_string(i);
                if(i == lineno)
                    ss << "  -> ";
                else
                    ss << "     ";
                ss << frame_0->co->src->get_line(i) << '\n';
            }
            stdout_write(ss.str());
            continue;
        }

        int space = line.find_first_of(' ');
        if(space != -1) {
            std::string cmd = line.substr(0, space);
            std::string arg = line.substr(space + 1);
            if(arg.empty()) continue;  // ignore empty command
            if(cmd == "p" || cmd == "print") {
                CodeObject_ code = compile(arg, "<stdin>", EVAL_MODE, true);
                PyVar retval = vm->_exec(code.get(), frame_0->_module, frame_0->_callable, frame_0->_locals);
                stdout_write(vm->py_repr(retval));
                stdout_write("\n");
            } else if(cmd == "!") {
                CodeObject_ code = compile(arg, "<stdin>", EXEC_MODE, true);
                vm->_exec(code.get(), frame_0->_module, frame_0->_callable, frame_0->_locals);
            }
            continue;
        }
    }
#endif
}

/**************************************************************************/
void Function::_gc_mark(VM* vm) const {
    decl->_gc_mark(vm);
    if(_closure) {
        _closure->apply([=](StrName _, PyVar obj) {
            vm->obj_gc_mark(obj);
        });
    }
}

void NativeFunc::_gc_mark(VM* vm) const {
    if(decl) decl->_gc_mark(vm);
}

void FuncDecl::_gc_mark(VM* vm) const {
    code->_gc_mark(vm);
    for(int i = 0; i < kwargs.size(); i++)
        vm->obj_gc_mark(kwargs[i].value);
}

void List::_gc_mark(VM* vm) const {
    for(PyVar obj: *this)
        vm->obj_gc_mark(obj);
}

void Tuple::_gc_mark(VM* vm) const {
    for(PyVar obj: *this)
        vm->obj_gc_mark(obj);
}

void MappingProxy::_gc_mark(VM* vm) const { vm->__obj_gc_mark(obj); }

void BoundMethod::_gc_mark(VM* vm) const {
    vm->obj_gc_mark(func);
    vm->obj_gc_mark(self);
}

void StarWrapper::_gc_mark(VM* vm) const { vm->obj_gc_mark(obj); }

void StaticMethod::_gc_mark(VM* vm) const { vm->obj_gc_mark(func); }

void ClassMethod::_gc_mark(VM* vm) const { vm->obj_gc_mark(func); }

void Property::_gc_mark(VM* vm) const {
    vm->obj_gc_mark(getter);
    vm->obj_gc_mark(setter);
}

void Slice::_gc_mark(VM* vm) const {
    vm->obj_gc_mark(start);
    vm->obj_gc_mark(stop);
    vm->obj_gc_mark(step);
}

void Super::_gc_mark(VM* vm) const { vm->obj_gc_mark(first); }

void Frame::_gc_mark(VM* vm) const {
    vm->obj_gc_mark(_module);
    co->_gc_mark(vm);
    // Frame could be stored in a generator, so mark _callable for safety
    vm->obj_gc_mark(_callable);
}

void ManagedHeap::mark() {
    for(PyObject* obj: _no_gc)
        vm->__obj_gc_mark(obj);
    vm->callstack.apply([this](Frame& frame) {
        frame._gc_mark(vm);
    });
    for(auto [_, co]: vm->__cached_codes)
        co->_gc_mark(vm);
    vm->obj_gc_mark(vm->__last_exception);
    vm->obj_gc_mark(vm->__curr_class);
    vm->obj_gc_mark(vm->__c.error);
    vm->__stack_gc_mark(vm->s_data.begin(), vm->s_data.end());
    if(_gc_marker_ex) _gc_marker_ex(vm);
}

void ManagedHeap::_delete(PyObject* obj) {
    const PyTypeInfo* ti = vm->_tp_info(obj->type);
    if(ti->vt._dtor) ti->vt._dtor(obj->_value_ptr());
    delete obj->_attr;  // delete __dict__ if exists
    PoolObject_dealloc(obj);
}

void Dict::_gc_mark(VM* vm) const {
    apply([vm](PyVar k, PyVar v) {
        vm->obj_gc_mark(k);
        vm->obj_gc_mark(v);
    });
}

void CodeObject::_gc_mark(VM* vm) const {
    for(PyVar v: consts)
        vm->obj_gc_mark(v);
    for(auto& decl: func_decls)
        decl->_gc_mark(vm);
}

}  // namespace pkpy
