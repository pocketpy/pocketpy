#pragma once

#include "codeobject.h"
#include "common.h"
#include "frame.h"
#include "error.h"
#include "gc.h"
#include "memory.h"
#include "obj.h"
#include "str.h"

namespace pkpy{

Str _read_file_cwd(const Str& name, bool* ok);

#define DEF_NATIVE_2(ctype, ptype)                                      \
    template<> inline ctype py_cast<ctype>(VM* vm, PyObject* obj) {     \
        vm->check_type(obj, vm->ptype);                                 \
        return OBJ_GET(ctype, obj);                                     \
    }                                                                   \
    template<> inline ctype _py_cast<ctype>(VM* vm, PyObject* obj) {    \
        return OBJ_GET(ctype, obj);                                     \
    }                                                                   \
    template<> inline ctype& py_cast<ctype&>(VM* vm, PyObject* obj) {   \
        vm->check_type(obj, vm->ptype);                                 \
        return OBJ_GET(ctype, obj);                                     \
    }                                                                   \
    template<> inline ctype& _py_cast<ctype&>(VM* vm, PyObject* obj) {  \
        return OBJ_GET(ctype, obj);                                     \
    }                                                                   \
    inline PyObject* py_var(VM* vm, const ctype& value) { return vm->heap.gcnew(vm->ptype, value);}     \
    inline PyObject* py_var(VM* vm, ctype&& value) { return vm->heap.gcnew(vm->ptype, std::move(value));}


class Generator final: public BaseIter {
    Frame frame;
    int state; // 0,1,2
public:
    template<typename... Args>
    Generator(VM* vm, Frame&& frame)
        : BaseIter(vm), frame(std::move(frame)), state(0) {}

    PyObject* next() override;
    void _gc_mark() const override;
};

struct PyTypeInfo{
    PyObject* obj;
    Type base;
    Str name;
};

struct FrameId{
    std::vector<pkpy::Frame>* data;
    int index;
    FrameId(std::vector<pkpy::Frame>* data, int index) : data(data), index(index) {}
    Frame* operator->() const { return &data->operator[](index); }
};

class VM {
    VM* vm;     // self reference for simplify code
public:
    ManagedHeap heap;
    stack< Frame > callstack;
    std::vector<PyTypeInfo> _all_types;

    NameDict _modules;                                  // loaded modules
    std::map<StrName, Str> _lazy_modules;               // lazy loaded modules

    PyObject* _py_op_call;
    PyObject* _py_op_yield;
    PyObject* _py_null;
    PyObject* None;
    PyObject* True;
    PyObject* False;
    PyObject* Ellipsis;
    PyObject* builtins;         // builtins module
    PyObject* _main;            // __main__ module

    std::stringstream _stdout_buffer;
    std::stringstream _stderr_buffer;
    std::ostream* _stdout;
    std::ostream* _stderr;
    int recursionlimit = 1000;

    // for quick access
    Type tp_object, tp_type, tp_int, tp_float, tp_bool, tp_str;
    Type tp_list, tp_tuple;
    Type tp_function, tp_native_function, tp_iterator, tp_bound_method;
    Type tp_slice, tp_range, tp_module;
    Type tp_super, tp_exception, tp_star_wrapper;

    VM(bool use_stdio) : heap(this){
        this->vm = this;
        this->_stdout = use_stdio ? &std::cout : &_stdout_buffer;
        this->_stderr = use_stdio ? &std::cerr : &_stderr_buffer;
        callstack.data().reserve(8);
        init_builtin_types();
    }

    bool is_stdio_used() const { return _stdout == &std::cout; }

    FrameId top_frame() {
#if DEBUG_EXTRA_CHECK
        if(callstack.empty()) FATAL_ERROR();
#endif
        return FrameId(&callstack.data(), callstack.size()-1);
    }

    PyObject* asStr(PyObject* obj){
        PyObject* self;
        PyObject* f = get_unbound_method(obj, __str__, &self, false);
        if(self != _py_null) return call(f, Args{self});
        return asRepr(obj);
    }

    PyObject* asIter(PyObject* obj){
        if(is_type(obj, tp_iterator)) return obj;
        PyObject* self;
        PyObject* iter_f = get_unbound_method(obj, __iter__, &self, false);
        if(self != _py_null) return call(iter_f, Args{self});
        TypeError(OBJ_NAME(_t(obj)).escape() + " object is not iterable");
        return nullptr;
    }

    PyObject* asList(PyObject* iterable){
        if(is_type(iterable, tp_list)) return iterable;
        return call(_t(tp_list), Args{iterable});
    }

    PyObject* find_name_in_mro(PyObject* cls, StrName name){
        PyObject* val;
        do{
            val = cls->attr().try_get(name);
            if(val != nullptr) return val;
            Type cls_t = OBJ_GET(Type, cls);
            Type base = _all_types[cls_t].base;
            if(base.index == -1) break;
            cls = _all_types[base].obj;
        }while(true);
        return nullptr;
    }

    bool isinstance(PyObject* obj, Type cls_t){
        Type obj_t = OBJ_GET(Type, _t(obj));
        do{
            if(obj_t == cls_t) return true;
            Type base = _all_types[obj_t].base;
            if(base.index == -1) break;
            obj_t = base;
        }while(true);
        return false;
    }

    PyObject* fast_call(StrName name, Args&& args){
        PyObject* val = find_name_in_mro(_t(args[0]), name);
        if(val != nullptr) return call(val, std::move(args));
        AttributeError(args[0], name);
        return nullptr;
    }

    template<typename ArgT>
    std::enable_if_t<std::is_same_v<std::decay_t<ArgT>, Args>, PyObject*>
    call(PyObject* callable, ArgT&& args){
        return call(callable, std::forward<ArgT>(args), no_arg(), false);
    }

    PyObject* exec(Str source, Str filename, CompileMode mode, PyObject* _module=nullptr){
        if(_module == nullptr) _module = _main;
        try {
            CodeObject_ code = compile(source, filename, mode);
#if DEBUG_DIS_EXEC
            if(_module == _main) std::cout << disassemble(code) << '\n';
#endif
            return _exec(code, _module);
        }catch (const Exception& e){
            *_stderr << e.summary() << '\n';

        }
#if !DEBUG_FULL_EXCEPTION
        catch (const std::exception& e) {
            *_stderr << "An std::exception occurred! It could be a bug.\n";
            *_stderr << e.what() << '\n';
        }
#endif
        callstack.clear();
        return nullptr;
    }

    template<typename ...Args>
    void _push_new_frame(Args&&... args){
        if(callstack.size() > recursionlimit){
            _error("RecursionError", "maximum recursion depth exceeded");
        }
        callstack.emplace(std::forward<Args>(args)...);
    }

    void _push_new_frame(Frame&& frame){
        if(callstack.size() > recursionlimit){
            _error("RecursionError", "maximum recursion depth exceeded");
        }
        callstack.emplace(std::move(frame));
    }

    template<typename ...Args>
    PyObject* _exec(Args&&... args){
        _push_new_frame(std::forward<Args>(args)...);
        return _run_top_frame();
    }

    PyObject* property(NativeFuncRaw fget){
        PyObject* p = builtins->attr("property");
        PyObject* method = heap.gcnew(tp_native_function, NativeFunc(fget, 1, false));
        return call(p, Args{method});
    }

    PyObject* new_type_object(PyObject* mod, StrName name, Type base){
        PyObject* obj = heap._new<Type>(tp_type, _all_types.size());
        PyTypeInfo info{
            obj,
            base,
            (mod!=nullptr && mod!=builtins) ? Str(OBJ_NAME(mod)+"."+name.sv()): name.sv()
        };
        if(mod != nullptr) mod->attr().set(name, obj);
        _all_types.push_back(info);
        return obj;
    }

    Type _new_type_object(StrName name, Type base=0) {
        PyObject* obj = new_type_object(nullptr, name, base);
        return OBJ_GET(Type, obj);
    }

    PyObject* _find_type(const Str& type){
        PyObject* obj = builtins->attr().try_get(type);
        if(obj == nullptr){
            for(auto& t: _all_types) if(t.name == type) return t.obj;
            throw std::runtime_error(fmt("type not found: ", type));
        }
        return obj;
    }

    template<int ARGC>
    void bind_func(Str type, Str name, NativeFuncRaw fn) {
        bind_func<ARGC>(_find_type(type), name, fn);
    }

    template<int ARGC>
    void bind_method(Str type, Str name, NativeFuncRaw fn) {
        bind_method<ARGC>(_find_type(type), name, fn);
    }

    template<int ARGC, typename... Args>
    void bind_static_method(Args&&... args) {
        bind_func<ARGC>(std::forward<Args>(args)...);
    }

    template<int ARGC>
    void _bind_methods(std::vector<Str> types, Str name, NativeFuncRaw fn) {
        for(auto& type: types) bind_method<ARGC>(type, name, fn);
    }

    template<int ARGC>
    void bind_builtin_func(Str name, NativeFuncRaw fn) {
        bind_func<ARGC>(builtins, name, fn);
    }

    int normalized_index(int index, int size){
        if(index < 0) index += size;
        if(index < 0 || index >= size){
            IndexError(std::to_string(index) + " not in [0, " + std::to_string(size) + ")");
        }
        return index;
    }

    template<typename P>
    PyObject* PyIter(P&& value) {
        static_assert(std::is_base_of_v<BaseIter, std::decay_t<P>>);
        return heap.gcnew<P>(tp_iterator, std::forward<P>(value));
    }

    BaseIter* PyIter_AS_C(PyObject* obj)
    {
        check_type(obj, tp_iterator);
        return static_cast<BaseIter*>(obj->value());
    }
    
    /***** Error Reporter *****/
    void _error(StrName name, const Str& msg){
        _error(Exception(name, msg));
    }

    void _raise(){
        bool ok = top_frame()->jump_to_exception_handler();
        if(ok) throw HandledException();
        else throw UnhandledException();
    }

    void IOError(const Str& msg) { _error("IOError", msg); }
    void NotImplementedError(){ _error("NotImplementedError", ""); }
    void TypeError(const Str& msg){ _error("TypeError", msg); }
    void ZeroDivisionError(){ _error("ZeroDivisionError", "division by zero"); }
    void IndexError(const Str& msg){ _error("IndexError", msg); }
    void ValueError(const Str& msg){ _error("ValueError", msg); }
    void NameError(StrName name){ _error("NameError", fmt("name ", name.escape() + " is not defined")); }

    void AttributeError(PyObject* obj, StrName name){
        // OBJ_NAME calls getattr, which may lead to a infinite recursion
        _error("AttributeError", fmt("type ", OBJ_NAME(_t(obj)).escape(), " has no attribute ", name.escape()));
    }

    void AttributeError(Str msg){ _error("AttributeError", msg); }

    void check_type(PyObject* obj, Type type){
        if(is_type(obj, type)) return;
        TypeError("expected " + OBJ_NAME(_t(type)).escape() + ", but got " + OBJ_NAME(_t(obj)).escape());
    }

    PyObject* _t(Type t){
        return _all_types[t.index].obj;
    }

    PyObject* _t(PyObject* obj){
        if(is_int(obj)) return _t(tp_int);
        if(is_float(obj)) return _t(tp_float);
        return _all_types[OBJ_GET(Type, _t(obj->type)).index].obj;
    }

    ~VM() {
        callstack.clear();
        _all_types.clear();
        _modules.clear();
        _lazy_modules.clear();
    }

    CodeObject_ compile(Str source, Str filename, CompileMode mode);
    PyObject* num_negated(PyObject* obj);
    f64 num_to_float(PyObject* obj);
    bool asBool(PyObject* obj);
    i64 hash(PyObject* obj);
    PyObject* asRepr(PyObject* obj);
    PyObject* new_module(StrName name);
    Str disassemble(CodeObject_ co);
    void init_builtin_types();
    PyObject* call(PyObject* callable, Args args, const Args& kwargs, bool opCall);
    PyObject* _py_call(PyObject* callable, ArgsView args, ArgsView kwargs);
    void unpack_args(Args& args);
    PyObject* getattr(PyObject* obj, StrName name, bool throw_err=true);
    PyObject* get_unbound_method(PyObject* obj, StrName name, PyObject** self, bool throw_err=true, bool fallback=false);
    template<typename T>
    void setattr(PyObject* obj, StrName name, T&& value);
    template<int ARGC>
    void bind_method(PyObject*, Str, NativeFuncRaw);
    template<int ARGC>
    void bind_func(PyObject*, Str, NativeFuncRaw);
    void _error(Exception);
    PyObject* _run_top_frame();
    void post_init();
};

inline PyObject* NativeFunc::operator()(VM* vm, Args& args) const{
    int args_size = args.size() - (int)method;  // remove self
    if(argc != -1 && args_size != argc) {
        vm->TypeError(fmt("expected ", argc, " arguments, but got ", args_size));
    }
    return f(vm, args);
}

inline void CodeObject::optimize(VM* vm){
    // uint32_t base_n = (uint32_t)(names.size() / kLocalsLoadFactor + 0.5);
    // perfect_locals_capacity = std::max(find_next_capacity(base_n), NameDict::__Capacity);
    // perfect_hash_seed = find_perfect_hash_seed(perfect_locals_capacity, names);
}

DEF_NATIVE_2(Str, tp_str)
DEF_NATIVE_2(List, tp_list)
DEF_NATIVE_2(Tuple, tp_tuple)
DEF_NATIVE_2(Function, tp_function)
DEF_NATIVE_2(NativeFunc, tp_native_function)
DEF_NATIVE_2(BoundMethod, tp_bound_method)
DEF_NATIVE_2(Range, tp_range)
DEF_NATIVE_2(Slice, tp_slice)
DEF_NATIVE_2(Exception, tp_exception)
DEF_NATIVE_2(StarWrapper, tp_star_wrapper)

#define PY_CAST_INT(T)                                  \
template<> inline T py_cast<T>(VM* vm, PyObject* obj){  \
    vm->check_type(obj, vm->tp_int);                    \
    return (T)(BITS(obj) >> 2);                         \
}                                                       \
template<> inline T _py_cast<T>(VM* vm, PyObject* obj){ \
    return (T)(BITS(obj) >> 2);                         \
}

PY_CAST_INT(char)
PY_CAST_INT(short)
PY_CAST_INT(int)
PY_CAST_INT(long)
PY_CAST_INT(long long)
PY_CAST_INT(unsigned char)
PY_CAST_INT(unsigned short)
PY_CAST_INT(unsigned int)
PY_CAST_INT(unsigned long)
PY_CAST_INT(unsigned long long)


template<> inline float py_cast<float>(VM* vm, PyObject* obj){
    vm->check_type(obj, vm->tp_float);
    i64 bits = BITS(obj);
    bits = (bits >> 2) << 2;
    return BitsCvt(bits)._float;
}
template<> inline float _py_cast<float>(VM* vm, PyObject* obj){
    i64 bits = BITS(obj);
    bits = (bits >> 2) << 2;
    return BitsCvt(bits)._float;
}
template<> inline double py_cast<double>(VM* vm, PyObject* obj){
    vm->check_type(obj, vm->tp_float);
    i64 bits = BITS(obj);
    bits = (bits >> 2) << 2;
    return BitsCvt(bits)._float;
}
template<> inline double _py_cast<double>(VM* vm, PyObject* obj){
    i64 bits = BITS(obj);
    bits = (bits >> 2) << 2;
    return BitsCvt(bits)._float;
}


#define PY_VAR_INT(T)                                       \
    inline PyObject* py_var(VM* vm, T _val){                \
        i64 val = static_cast<i64>(_val);                   \
        if(((val << 2) >> 2) != val){                       \
            vm->_error("OverflowError", std::to_string(val) + " is out of range");  \
        }                                                                           \
        val = (val << 2) | 0b01;                                                    \
        return reinterpret_cast<PyObject*>(val);                                    \
    }

PY_VAR_INT(char)
PY_VAR_INT(short)
PY_VAR_INT(int)
PY_VAR_INT(long)
PY_VAR_INT(long long)
PY_VAR_INT(unsigned char)
PY_VAR_INT(unsigned short)
PY_VAR_INT(unsigned int)
PY_VAR_INT(unsigned long)
PY_VAR_INT(unsigned long long)

#define PY_VAR_FLOAT(T)                             \
    inline PyObject* py_var(VM* vm, T _val){        \
        f64 val = static_cast<f64>(_val);           \
        i64 bits = BitsCvt(val)._int;               \
        bits = (bits >> 2) << 2;                    \
        bits |= 0b10;                               \
        return reinterpret_cast<PyObject*>(bits);   \
    }

PY_VAR_FLOAT(float)
PY_VAR_FLOAT(double)

inline PyObject* py_var(VM* vm, bool val){
    return val ? vm->True : vm->False;
}

template<> inline bool py_cast<bool>(VM* vm, PyObject* obj){
    vm->check_type(obj, vm->tp_bool);
    return obj == vm->True;
}
template<> inline bool _py_cast<bool>(VM* vm, PyObject* obj){
    return obj == vm->True;
}

inline PyObject* py_var(VM* vm, const char val[]){
    return VAR(Str(val));
}

inline PyObject* py_var(VM* vm, std::string val){
    return VAR(Str(std::move(val)));
}

inline PyObject* py_var(VM* vm, std::string_view val){
    return VAR(Str(val));
}

template<typename T>
void _check_py_class(VM* vm, PyObject* obj){
    vm->check_type(obj, T::_type(vm));
}

inline PyObject* VM::num_negated(PyObject* obj){
    if (is_int(obj)){
        return VAR(-CAST(i64, obj));
    }else if(is_float(obj)){
        return VAR(-CAST(f64, obj));
    }
    TypeError("expected 'int' or 'float', got " + OBJ_NAME(_t(obj)).escape());
    return nullptr;
}

inline f64 VM::num_to_float(PyObject* obj){
    if(is_float(obj)){
        return CAST(f64, obj);
    } else if (is_int(obj)){
        return (f64)CAST(i64, obj);
    }
    TypeError("expected 'int' or 'float', got " + OBJ_NAME(_t(obj)).escape());
    return 0;
}

inline bool VM::asBool(PyObject* obj){
    if(is_type(obj, tp_bool)) return obj == True;
    if(obj == None) return false;
    if(is_type(obj, tp_int)) return CAST(i64, obj) != 0;
    if(is_type(obj, tp_float)) return CAST(f64, obj) != 0.0;
    PyObject* self;
    PyObject* len_f = get_unbound_method(obj, __len__, &self, false);
    if(self != _py_null){
        PyObject* ret = call(len_f, Args{self});
        return CAST(i64, ret) > 0;
    }
    return true;
}

inline i64 VM::hash(PyObject* obj){
    if (is_type(obj, tp_str)) return CAST(Str&, obj).hash();
    if (is_int(obj)) return CAST(i64, obj);
    if (is_type(obj, tp_tuple)) {
        i64 x = 1000003;
        const Tuple& items = CAST(Tuple&, obj);
        for (int i=0; i<items.size(); i++) {
            i64 y = hash(items[i]);
            // recommended by Github Copilot
            x = x ^ (y + 0x9e3779b9 + (x << 6) + (x >> 2));
        }
        return x;
    }
    if (is_type(obj, tp_type)) return BITS(obj);
    if (is_type(obj, tp_bool)) return _CAST(bool, obj) ? 1 : 0;
    if (is_float(obj)){
        f64 val = CAST(f64, obj);
        return (i64)std::hash<f64>()(val);
    }
    TypeError("unhashable type: " +  OBJ_NAME(_t(obj)).escape());
    return 0;
}

inline PyObject* VM::asRepr(PyObject* obj){
    // TODO: fastcall does not take care of super() proxy!
    return fast_call(__repr__, Args{obj});
}

inline PyObject* VM::new_module(StrName name) {
    PyObject* obj = heap._new<DummyModule>(tp_module, DummyModule());
    obj->attr().set(__name__, VAR(name.sv()));
    // we do not allow override in order to avoid memory leak
    // it is because Module objects are not garbage collected
    if(_modules.contains(name)) FATAL_ERROR();
    _modules.set(name, obj);
    return obj;
}

inline Str VM::disassemble(CodeObject_ co){
    auto pad = [](const Str& s, const int n){
        if(s.length() >= n) return s.substr(0, n);
        return s + std::string(n - s.length(), ' ');
    };

    std::vector<int> jumpTargets;
    for(auto byte : co->codes){
        if(byte.op == OP_JUMP_ABSOLUTE || byte.op == OP_POP_JUMP_IF_FALSE){
            jumpTargets.push_back(byte.arg);
        }
    }
    std::stringstream ss;
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
        ss << " " << pad(OP_NAMES[byte.op], 20) << " ";
        // ss << pad(byte.arg == -1 ? "" : std::to_string(byte.arg), 5);
        std::string argStr = byte.arg == -1 ? "" : std::to_string(byte.arg);
        switch(byte.op){
            case OP_LOAD_CONST:
                argStr += fmt(" (", CAST(Str, asRepr(co->consts[byte.arg])), ")");
                break;
            case OP_LOAD_NAME: case OP_LOAD_GLOBAL: case OP_LOAD_NONLOCAL: case OP_STORE_GLOBAL:
            case OP_LOAD_ATTR: case OP_LOAD_METHOD: case OP_STORE_ATTR: case OP_DELETE_ATTR:
            case OP_IMPORT_NAME: case OP_BEGIN_CLASS:
            case OP_DELETE_GLOBAL:
                argStr += fmt(" (", StrName(byte.arg).sv(), ")");
                break;
            case OP_LOAD_FAST: case OP_STORE_FAST: case OP_DELETE_FAST:
                argStr += fmt(" (", co->varnames[byte.arg].sv(), ")");
                break;
            case OP_BINARY_OP:
                argStr += fmt(" (", BINARY_SPECIAL_METHODS[byte.arg], ")");
                break;
            case OP_LOAD_FUNCTION:
                argStr += fmt(" (", co->func_decls[byte.arg]->code->name, ")");
                break;
        }
        ss << pad(argStr, 40);      // may overflow
        ss << co->blocks[byte.block].type;
        if(i != co->codes.size() - 1) ss << '\n';
    }

    for(auto& decl: co->func_decls){
        ss << "\n\n" << "Disassembly of " << decl->code->name << ":\n";
        ss << disassemble(decl->code);
    }
    ss << "\n";
    return Str(ss.str());
}

inline void VM::init_builtin_types(){
    _all_types.push_back({heap._new<Type>(Type(1), Type(0)), -1, "object"});
    _all_types.push_back({heap._new<Type>(Type(1), Type(1)), 0, "type"});
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
    tp_star_wrapper = _new_type_object("_star_wrapper");
    tp_function = _new_type_object("function");
    tp_native_function = _new_type_object("native_function");
    tp_iterator = _new_type_object("iterator");
    tp_bound_method = _new_type_object("bound_method");
    tp_super = _new_type_object("super");
    tp_exception = _new_type_object("Exception");

    this->None = heap._new<Dummy>(_new_type_object("NoneType"), {});
    this->Ellipsis = heap._new<Dummy>(_new_type_object("ellipsis"), {});
    this->True = heap._new<Dummy>(tp_bool, {});
    this->False = heap._new<Dummy>(tp_bool, {});
    this->_py_null = heap._new<Dummy>(_new_type_object("_py_null"), {});
    this->_py_op_call = heap._new<Dummy>(_new_type_object("_py_op_call"), {});
    this->_py_op_yield = heap._new<Dummy>(_new_type_object("_py_op_yield"), {});

    this->builtins = new_module("builtins");
    this->_main = new_module("__main__");
    
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

    post_init();
    for(int i=0; i<_all_types.size(); i++){
        _all_types[i].obj->attr()._try_perfect_rehash();
    }
    for(auto [k, v]: _modules.items()) v->attr()._try_perfect_rehash();
}

inline PyObject* VM::_py_call(PyObject* callable, ArgsView args, ArgsView kwargs){
    // callable is a `function` object
    const Function& fn = CAST(Function&, callable);
    const CodeObject* co = fn.decl->code.get();
    FastLocals locals(co);

    int i = 0;
    if(args.size() < fn.decl->args.size()){
        vm->TypeError(fmt("expected ", fn.decl->args.size(), " positional arguments, but got ", args.size()));
    }

    // prepare args
    for(int index: fn.decl->args) locals[index] = args[i++];
    // prepare kwdefaults
    for(auto& kv: fn.decl->kwargs) locals[kv.key] = kv.value;
    
    // handle *args
    if(fn.decl->starred_arg != -1){
        List vargs;        // handle *args
        while(i < args.size()) vargs.push_back(args[i++]);
        locals[fn.decl->starred_arg] = VAR(Tuple(std::move(vargs)));
    }else{
        // kwdefaults override
        for(auto& kv: fn.decl->kwargs){
            if(i < args.size()){
                locals[kv.key] = args[i++];
            }else{
                break;
            }
        }
        if(i < args.size()) TypeError("too many arguments");
    }
    
    for(int i=0; i<kwargs.size(); i+=2){
        StrName key = CAST(int, kwargs[i]);
        // try_set has nullptr check
        // TODO: optimize this
        bool ok = locals.try_set(key, kwargs[i+1]);
        if(!ok){
            TypeError(fmt(key.escape(), " is an invalid keyword argument for ", co->name, "()"));
        }
    }
    PyObject* _module = fn._module != nullptr ? fn._module : top_frame()->_module;
    if(co->is_generator){
        return PyIter(Generator(this, Frame(co, _module, std::move(locals), fn._closure)));
    }
    _push_new_frame(co, _module, std::move(locals), fn._closure);
    return nullptr;
}

// TODO: callable/args here may be garbage collected accidentally
inline PyObject* VM::call(PyObject* callable, Args args, const Args& kwargs, bool opCall){
    if(is_type(callable, tp_bound_method)){
        auto& bm = CAST(BoundMethod&, callable);
        callable = bm.method;      // get unbound method
        args.extend_self(bm.obj);
    }
    
    if(is_type(callable, tp_native_function)){
        const auto& f = OBJ_GET(NativeFunc, callable);
        if(kwargs.size() != 0) TypeError("native_function does not accept keyword arguments");
        return f(this, args);
    } else if(is_type(callable, tp_function)){
        // ret is nullptr or a generator
        PyObject* ret = _py_call(callable, args, kwargs);
        if(ret != nullptr) return ret;
        if(opCall) return _py_op_call;
        return _run_top_frame();
    }

    if(is_type(callable, tp_type)){
        // TODO: use get_unbound_method here
        PyObject* new_f = callable->attr().try_get(__new__);
        PyObject* obj;
        if(new_f != nullptr){
            obj = call(new_f, std::move(args), kwargs, false);
        }else{
            obj = heap.gcnew<DummyInstance>(OBJ_GET(Type, callable), {});
            PyObject* self;
            PyObject* init_f = get_unbound_method(obj, __init__, &self, false);
            args.extend_self(self);
            if (self != _py_null) call(init_f, std::move(args), kwargs, false);
        }
        return obj;
    }

    PyObject* self;
    PyObject* call_f = get_unbound_method(callable, __call__, &self, false);
    if(self != _py_null){
        args.extend_self(self);
        return call(call_f, std::move(args), kwargs, false);
    }
    TypeError(OBJ_NAME(_t(callable)).escape() + " object is not callable");
    return None;
}

inline void VM::unpack_args(Args& args){
    List unpacked;
    for(int i=0; i<args.size(); i++){
        if(is_type(args[i], tp_star_wrapper)){
            auto& star = _CAST(StarWrapper&, args[i]);
            List& list = CAST(List&, asList(star.obj));
            unpacked.extend(list);
        }else{
            unpacked.push_back(args[i]);
        }
    }
    args = Args(std::move(unpacked));
}

// https://docs.python.org/3/howto/descriptor.html#invocation-from-an-instance
inline PyObject* VM::getattr(PyObject* obj, StrName name, bool throw_err){
    PyObject* objtype = _t(obj);
    // handle super() proxy
    if(is_type(obj, tp_super)){
        const Super& super = OBJ_GET(Super, obj);
        obj = super.first;
        objtype = _t(super.second);
    }
    PyObject* cls_var = find_name_in_mro(objtype, name);
    if(cls_var != nullptr){
        // handle descriptor
        PyObject* descr_get = _t(cls_var)->attr().try_get(__get__);
        if(descr_get != nullptr) return call(descr_get, Args{cls_var, obj});
    }
    // handle instance __dict__
    if(!is_tagged(obj) && obj->is_attr_valid()){
        PyObject* val = obj->attr().try_get(name);
        if(val != nullptr) return val;
    }
    if(cls_var != nullptr){
        // bound method is non-data descriptor
        if(is_type(cls_var, tp_function) || is_type(cls_var, tp_native_function)){
            return VAR(BoundMethod(obj, cls_var));
        }
        return cls_var;
    }
    if(throw_err) AttributeError(obj, name);
    return nullptr;
}

// used by OP_LOAD_METHOD
// try to load a unbound method (fallback to `getattr` if not found)
inline PyObject* VM::get_unbound_method(PyObject* obj, StrName name, PyObject** self, bool throw_err, bool fallback){
    *self = _py_null;
    PyObject* objtype = _t(obj);
    // handle super() proxy
    if(is_type(obj, tp_super)){
        const Super& super = OBJ_GET(Super, obj);
        obj = super.first;
        objtype = _t(super.second);
    }
    PyObject* cls_var = find_name_in_mro(objtype, name);

    if(fallback){
        if(cls_var != nullptr){
            // handle descriptor
            PyObject* descr_get = _t(cls_var)->attr().try_get(__get__);
            if(descr_get != nullptr) return call(descr_get, Args{cls_var, obj});
        }
        // handle instance __dict__
        if(!is_tagged(obj) && obj->is_attr_valid()){
            PyObject* val = obj->attr().try_get(name);
            if(val != nullptr) return val;
        }
    }

    if(cls_var != nullptr){
        if(is_type(cls_var, tp_function) || is_type(cls_var, tp_native_function)){
            *self = obj;
        }
        return cls_var;
    }
    if(throw_err) AttributeError(obj, name);
    return nullptr;
}

template<typename T>
inline void VM::setattr(PyObject* obj, StrName name, T&& value){
    static_assert(std::is_same_v<std::decay_t<T>, PyObject*>);
    PyObject* objtype = _t(obj);
    // handle super() proxy
    if(is_type(obj, tp_super)){
        Super& super = OBJ_GET(Super, obj);
        obj = super.first;
        objtype = _t(super.second);
    }
    PyObject* cls_var = find_name_in_mro(objtype, name);
    if(cls_var != nullptr){
        // handle descriptor
        PyObject* cls_var_t = _t(cls_var);
        if(cls_var_t->attr().contains(__get__)){
            PyObject* descr_set = cls_var_t->attr().try_get(__set__);
            if(descr_set != nullptr){
                call(descr_set, Args{cls_var, obj, std::forward<T>(value)});
            }else{
                TypeError(fmt("readonly attribute: ", name.escape()));
            }
            return;
        }
    }
    // handle instance __dict__
    if(is_tagged(obj) || !obj->is_attr_valid()) TypeError("cannot set attribute");
    obj->attr().set(name, std::forward<T>(value));
}

template<int ARGC>
void VM::bind_method(PyObject* obj, Str name, NativeFuncRaw fn) {
    check_type(obj, tp_type);
    obj->attr().set(name, VAR(NativeFunc(fn, ARGC, true)));
}

template<int ARGC>
void VM::bind_func(PyObject* obj, Str name, NativeFuncRaw fn) {
    obj->attr().set(name, VAR(NativeFunc(fn, ARGC, false)));
}

inline void VM::_error(Exception e){
    if(callstack.empty()){
        e.is_re = false;
        throw e;
    }
    Frame* frame = &callstack.top();
    frame->_s.push(VAR(e));
    _raise();
}

inline void ManagedHeap::mark() {
    for(PyObject* obj: _no_gc) OBJ_MARK(obj);
    for(auto& frame : vm->callstack.data()) frame._gc_mark();
}

inline Str obj_type_name(VM *vm, Type type){
    return vm->_all_types[type].name;
}

}   // namespace pkpy