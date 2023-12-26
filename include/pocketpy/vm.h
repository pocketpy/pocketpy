#pragma once

#include "codeobject.h"
#include "common.h"
#include "frame.h"
#include "error.h"
#include "gc.h"
#include "memory.h"
#include "obj.h"
#include "str.h"
#include "tuplelist.h"
#include "dict.h"

namespace pkpy{

/* Stack manipulation macros */
// https://github.com/python/cpython/blob/3.9/Python/ceval.c#L1123
#define TOP()             (s_data.top())
#define SECOND()          (s_data.second())
#define THIRD()           (s_data.third())
#define PEEK(n)           (s_data.peek(n))
#define STACK_SHRINK(n)   (s_data.shrink(n))
#define PUSH(v)           (s_data.push(v))
#define POP()             (s_data.pop())
#define POPX()            (s_data.popx())
#define STACK_VIEW(n)     (s_data.view(n))

#define DEF_NATIVE_2(ctype, ptype)                                      \
    template<> inline ctype py_cast<ctype>(VM* vm, PyObject* obj) {     \
        vm->check_non_tagged_type(obj, vm->ptype);                      \
        return PK_OBJ_GET(ctype, obj);                                     \
    }                                                                   \
    template<> inline ctype _py_cast<ctype>(VM* vm, PyObject* obj) {    \
        PK_UNUSED(vm);                                                  \
        return PK_OBJ_GET(ctype, obj);                                     \
    }                                                                   \
    template<> inline ctype& py_cast<ctype&>(VM* vm, PyObject* obj) {   \
        vm->check_non_tagged_type(obj, vm->ptype);                      \
        return PK_OBJ_GET(ctype, obj);                                     \
    }                                                                   \
    template<> inline ctype& _py_cast<ctype&>(VM* vm, PyObject* obj) {  \
        PK_UNUSED(vm);                                                  \
        return PK_OBJ_GET(ctype, obj);                                     \
    }                                                                   \
    inline PyObject* py_var(VM* vm, const ctype& value) { return vm->heap.gcnew<ctype>(vm->ptype, value);}     \
    inline PyObject* py_var(VM* vm, ctype&& value) { return vm->heap.gcnew<ctype>(vm->ptype, std::move(value));}


typedef PyObject* (*BinaryFuncC)(VM*, PyObject*, PyObject*);

struct PyTypeInfo{
    PyObject* obj;      // never be garbage collected
    Type base;
    PyObject* mod;      // never be garbage collected
    Str name;
    bool subclass_enabled;

    std::vector<StrName> annotated_fields;

    // cached special methods
    // unary operators
    PyObject* (*m__repr__)(VM* vm, PyObject*) = nullptr;
    PyObject* (*m__str__)(VM* vm, PyObject*) = nullptr;
    i64 (*m__hash__)(VM* vm, PyObject*) = nullptr;
    i64 (*m__len__)(VM* vm, PyObject*) = nullptr;
    PyObject* (*m__iter__)(VM* vm, PyObject*) = nullptr;
    PyObject* (*m__next__)(VM* vm, PyObject*) = nullptr;
    PyObject* (*m__neg__)(VM* vm, PyObject*) = nullptr;
    PyObject* (*m__bool__)(VM* vm, PyObject*) = nullptr;
    PyObject* (*m__invert__)(VM* vm, PyObject*) = nullptr;

    BinaryFuncC m__eq__ = nullptr;
    BinaryFuncC m__lt__ = nullptr;
    BinaryFuncC m__le__ = nullptr;
    BinaryFuncC m__gt__ = nullptr;
    BinaryFuncC m__ge__ = nullptr;
    BinaryFuncC m__contains__ = nullptr;

    // binary operators
    BinaryFuncC m__add__ = nullptr;
    BinaryFuncC m__sub__ = nullptr;
    BinaryFuncC m__mul__ = nullptr;
    BinaryFuncC m__truediv__ = nullptr;
    BinaryFuncC m__floordiv__ = nullptr;
    BinaryFuncC m__mod__ = nullptr;
    BinaryFuncC m__pow__ = nullptr;
    BinaryFuncC m__matmul__ = nullptr;

    BinaryFuncC m__lshift__ = nullptr;
    BinaryFuncC m__rshift__ = nullptr;
    BinaryFuncC m__and__ = nullptr;
    BinaryFuncC m__or__ = nullptr;
    BinaryFuncC m__xor__ = nullptr;

    // indexer
    PyObject* (*m__getitem__)(VM* vm, PyObject*, PyObject*) = nullptr;
    void (*m__setitem__)(VM* vm, PyObject*, PyObject*, PyObject*) = nullptr;
    void (*m__delitem__)(VM* vm, PyObject*, PyObject*) = nullptr;
};

struct FrameId{
    std::vector<pkpy::Frame>* data;
    int index;
    FrameId(std::vector<pkpy::Frame>* data, int index) : data(data), index(index) {}
    Frame* operator->() const { return &data->operator[](index); }
    Frame* get() const { return &data->operator[](index); }
};

typedef void(*PrintFunc)(const char*, int);

class VM {
    PK_ALWAYS_PASS_BY_POINTER(VM)
    
    VM* vm;     // self reference for simplify code
public:
    ManagedHeap heap;
    ValueStack s_data;
    stack< Frame > callstack;
    std::vector<PyTypeInfo> _all_types;
    
    NameDict _modules;                                 // loaded modules
    std::map<StrName, Str> _lazy_modules;              // lazy loaded modules

    struct{
        PyObject* error;
        stack<ArgsView> s_view;
    } _c;

    PyObject* None;
    PyObject* True;
    PyObject* False;
    PyObject* NotImplemented;   // unused
    PyObject* Ellipsis;
    PyObject* builtins;         // builtins module
    PyObject* StopIteration;
    PyObject* _main;            // __main__ module

    PyObject* _last_exception;  // last exception
    PyObject* _curr_class;      // current class being defined

    // cached code objects for FSTRING_EVAL
    std::map<std::string_view, CodeObject_> _cached_codes;

#if PK_ENABLE_CEVAL_CALLBACK
    void (*_ceval_on_step)(VM*, Frame*, Bytecode bc) = nullptr;
#endif

    PrintFunc _stdout;
    PrintFunc _stderr;
    unsigned char* (*_import_handler)(const char*, int, int*);

    // for quick access
    static constexpr Type tp_object=0, tp_type=1;
    static constexpr Type tp_int=kTpIntIndex, tp_float=kTpFloatIndex, tp_bool=4, tp_str=5;
    static constexpr Type tp_list=6, tp_tuple=7;
    static constexpr Type tp_slice=8, tp_range=9, tp_module=10;
    static constexpr Type tp_function=11, tp_native_func=12, tp_bound_method=13;
    
    static constexpr Type tp_super=14, tp_exception=15, tp_bytes=16, tp_mappingproxy=17;
    static constexpr Type tp_dict=18, tp_property=19, tp_star_wrapper=20;

    PyObject* cached_object__new__;

    const bool enable_os;

    VM(bool enable_os=true);

    FrameId top_frame();
    void _pop_frame();

    PyObject* py_str(PyObject* obj);
    PyObject* py_repr(PyObject* obj);
    PyObject* py_json(PyObject* obj);
    PyObject* py_iter(PyObject* obj);

    PyObject* find_name_in_mro(PyObject* cls, StrName name);
    bool isinstance(PyObject* obj, Type base);
    bool issubclass(Type cls, Type base);
    PyObject* exec(Str source, Str filename, CompileMode mode, PyObject* _module=nullptr);
    PyObject* exec(Str source);
    PyObject* eval(Str source);

    template<typename ...Args>
    PyObject* _exec(Args&&... args){
        callstack.emplace(&s_data, s_data._sp, std::forward<Args>(args)...);
        return _run_top_frame();
    }

    void _push_varargs(){ }
    void _push_varargs(PyObject* _0){ PUSH(_0); }
    void _push_varargs(PyObject* _0, PyObject* _1){ PUSH(_0); PUSH(_1); }
    void _push_varargs(PyObject* _0, PyObject* _1, PyObject* _2){ PUSH(_0); PUSH(_1); PUSH(_2); }
    void _push_varargs(PyObject* _0, PyObject* _1, PyObject* _2, PyObject* _3){ PUSH(_0); PUSH(_1); PUSH(_2); PUSH(_3); }

    void stdout_write(const Str& s){
        _stdout(s.data, s.size);
    }

    template<typename... Args>
    PyObject* call(PyObject* callable, Args&&... args){
        PUSH(callable);
        PUSH(PY_NULL);
        _push_varargs(args...);
        return vectorcall(sizeof...(args));
    }

    template<typename... Args>
    PyObject* call_method(PyObject* self, PyObject* callable, Args&&... args){
        PUSH(callable);
        PUSH(self);
        _push_varargs(args...);
        return vectorcall(sizeof...(args));
    }

    template<typename... Args>
    PyObject* call_method(PyObject* self, StrName name, Args&&... args){
        PyObject* callable = get_unbound_method(self, name, &self);
        return call_method(self, callable, args...);
    }

    PyObject* new_type_object(PyObject* mod, StrName name, Type base, bool subclass_enabled=true);
    Type _new_type_object(StrName name, Type base=0);
    PyObject* _find_type_object(const Str& type);

    Type _type(const Str& type);
    PyTypeInfo* _type_info(const Str& type);
    PyTypeInfo* _type_info(Type type);
    const PyTypeInfo* _inst_type_info(PyObject* obj);

#define BIND_UNARY_SPECIAL(name)                                                        \
    void bind##name(Type type, PyObject* (*f)(VM*, PyObject*)){                         \
        _all_types[type].m##name = f;                                                   \
        PyObject* nf = bind_method<0>(_t(type), #name, [](VM* vm, ArgsView args){       \
            return lambda_get_userdata<PyObject*(*)(VM*, PyObject*)>(args.begin())(vm, args[0]);\
        });                                                                             \
        PK_OBJ_GET(NativeFunc, nf).set_userdata(f);                                        \
    }

    BIND_UNARY_SPECIAL(__repr__)
    BIND_UNARY_SPECIAL(__str__)
    BIND_UNARY_SPECIAL(__iter__)
    BIND_UNARY_SPECIAL(__next__)
    BIND_UNARY_SPECIAL(__neg__)
    BIND_UNARY_SPECIAL(__bool__)
    BIND_UNARY_SPECIAL(__invert__)

    void bind__hash__(Type type, i64 (*f)(VM* vm, PyObject*));
    void bind__len__(Type type, i64 (*f)(VM* vm, PyObject*));
#undef BIND_UNARY_SPECIAL


#define BIND_BINARY_SPECIAL(name)                                                       \
    void bind##name(Type type, BinaryFuncC f){                                          \
        PyObject* obj = _t(type);                                                       \
        _all_types[type].m##name = f;                                                   \
        PyObject* nf = bind_method<1>(obj, #name, [](VM* vm, ArgsView args){            \
            return lambda_get_userdata<BinaryFuncC>(args.begin())(vm, args[0], args[1]); \
        });                                                                             \
        PK_OBJ_GET(NativeFunc, nf).set_userdata(f);                                        \
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

    void bind__getitem__(Type type, PyObject* (*f)(VM*, PyObject*, PyObject*)){
        PyObject* obj = _t(type);
        _all_types[type].m__getitem__ = f;
        PyObject* nf = bind_method<1>(obj, "__getitem__", [](VM* vm, ArgsView args){
            return lambda_get_userdata<PyObject*(*)(VM*, PyObject*, PyObject*)>(args.begin())(vm, args[0], args[1]);
        });
        PK_OBJ_GET(NativeFunc, nf).set_userdata(f);
    }

    void bind__setitem__(Type type, void (*f)(VM*, PyObject*, PyObject*, PyObject*)){
        PyObject* obj = _t(type);
        _all_types[type].m__setitem__ = f;
        PyObject* nf = bind_method<2>(obj, "__setitem__", [](VM* vm, ArgsView args){
            lambda_get_userdata<void(*)(VM* vm, PyObject*, PyObject*, PyObject*)>(args.begin())(vm, args[0], args[1], args[2]);
            return vm->None;
        });
        PK_OBJ_GET(NativeFunc, nf).set_userdata(f);
    }

    void bind__delitem__(Type type, void (*f)(VM*, PyObject*, PyObject*)){
        PyObject* obj = _t(type);
        _all_types[type].m__delitem__ = f;
        PyObject* nf = bind_method<1>(obj, "__delitem__", [](VM* vm, ArgsView args){
            lambda_get_userdata<void(*)(VM*, PyObject*, PyObject*)>(args.begin())(vm, args[0], args[1]);
            return vm->None;
        });
        PK_OBJ_GET(NativeFunc, nf).set_userdata(f);
    }

    bool py_eq(PyObject* lhs, PyObject* rhs);
    // new in v1.2.9
    bool py_lt(PyObject* lhs, PyObject* rhs);
    bool py_le(PyObject* lhs, PyObject* rhs);
    bool py_gt(PyObject* lhs, PyObject* rhs);
    bool py_ge(PyObject* lhs, PyObject* rhs);
    bool py_ne(PyObject* lhs, PyObject* rhs) { return !py_eq(lhs, rhs); }

    template<int ARGC>
    PyObject* bind_func(Str type, Str name, NativeFuncC fn) {
        return bind_func<ARGC>(_find_type_object(type), name, fn);
    }

    template<int ARGC>
    PyObject* bind_method(Str type, Str name, NativeFuncC fn) {
        return bind_method<ARGC>(_find_type_object(type), name, fn);
    }

    template<int ARGC, typename __T>
    PyObject* bind_constructor(__T&& type, NativeFuncC fn) {
        static_assert(ARGC==-1 || ARGC>=1);
        return bind_func<ARGC>(std::forward<__T>(type), "__new__", fn);
    }

    template<typename T, typename __T>
    PyObject* bind_default_constructor(__T&& type) {
        return bind_constructor<1>(std::forward<__T>(type), [](VM* vm, ArgsView args){
            Type t = PK_OBJ_GET(Type, args[0]);
            return vm->heap.gcnew<T>(t, T());
        });
    }

    template<typename T, typename __T>
    PyObject* bind_notimplemented_constructor(__T&& type) {
        return bind_constructor<-1>(std::forward<__T>(type), [](VM* vm, ArgsView args){
            PK_UNUSED(args);
            vm->NotImplementedError();
            return vm->None;
        });
    }

    template<int ARGC>
    PyObject* bind_builtin_func(Str name, NativeFuncC fn) {
        return bind_func<ARGC>(builtins, name, fn);
    }

    int normalized_index(int index, int size);
    PyObject* py_next(PyObject* obj);
    
    /***** Error Reporter *****/
    void _error(StrName name, const Str& msg){
        _error(Exception(name, msg));
    }

    void _raise(bool re_raise=false);

    void StackOverflowError() { _error("StackOverflowError", ""); }
    void IOError(const Str& msg) { _error("IOError", msg); }
    void NotImplementedError(){ _error("NotImplementedError", ""); }
    void TypeError(const Str& msg){ _error("TypeError", msg); }
    void IndexError(const Str& msg){ _error("IndexError", msg); }
    void ValueError(const Str& msg){ _error("ValueError", msg); }
    void RuntimeError(const Str& msg){ _error("RuntimeError", msg); }
    void ZeroDivisionError(const Str& msg){ _error("ZeroDivisionError", msg); }
    void ZeroDivisionError(){ _error("ZeroDivisionError", "division by zero"); }
    void NameError(StrName name){ _error("NameError", fmt("name ", name.escape() + " is not defined")); }
    void UnboundLocalError(StrName name){ _error("UnboundLocalError", fmt("local variable ", name.escape() + " referenced before assignment")); }
    void KeyError(PyObject* obj){ _error("KeyError", PK_OBJ_GET(Str, py_repr(obj))); }
    void BinaryOptError(const char* op) { TypeError(fmt("unsupported operand type(s) for ", op)); }
    void ImportError(const Str& msg){ _error("ImportError", msg); }

    void AttributeError(PyObject* obj, StrName name){
        // OBJ_NAME calls getattr, which may lead to a infinite recursion
        if(isinstance(obj, vm->tp_type)){
            _error("AttributeError", fmt("type object ", OBJ_NAME(obj).escape(), " has no attribute ", name.escape()));
        }else{
            _error("AttributeError", fmt(OBJ_NAME(_t(obj)).escape(), " object has no attribute ", name.escape()));
        }
    }

    void AttributeError(Str msg){ _error("AttributeError", msg); }

    void check_type(PyObject* obj, Type type){
        if(is_type(obj, type)) return;
        TypeError("expected " + OBJ_NAME(_t(type)).escape() + ", got " + OBJ_NAME(_t(obj)).escape());
    }

    void check_args_size(int size, int min_size, int max_size){
        if(size >= min_size && size <= max_size) return;
        TypeError(fmt("expected ", min_size, "-", max_size, " arguments, got ", size));
    }

    void check_non_tagged_type(PyObject* obj, Type type){
        if(is_non_tagged_type(obj, type)) return;
        TypeError("expected " + OBJ_NAME(_t(type)).escape() + ", got " + OBJ_NAME(_t(obj)).escape());
    }

    PyObject* _t(Type t){
        return _all_types[t.index].obj;
    }

    Type _tp(PyObject* obj){
        if(is_int(obj)) return tp_int;
        if(is_float(obj)) return tp_float;
        return obj->type;
    }

    PyObject* _t(PyObject* obj){
        return _all_types[_tp(obj).index].obj;
    }

    struct ImportContext{
        std::vector<Str> pending;
        std::vector<bool> pending_is_init;   // a.k.a __init__.py
        struct Temp{
            ImportContext* ctx;
            Temp(ImportContext* ctx, Str name, bool is_init) : ctx(ctx){
                ctx->pending.push_back(name);
                ctx->pending_is_init.push_back(is_init);
            }
            ~Temp(){
                ctx->pending.pop_back();
                ctx->pending_is_init.pop_back();
            }
        };

        Temp scope(Str name, bool is_init){
            return {this, name, is_init};
        }
    };

    ImportContext _import_context;
    PyObject* py_import(Str path, bool throw_err=true);
    ~VM();

#if PK_DEBUG_CEVAL_STEP
    void _log_s_data(const char* title = nullptr);
#endif
    void _unpack_as_list(ArgsView args, List& list);
    void _unpack_as_dict(ArgsView args, Dict& dict);
    PyObject* vectorcall(int ARGC, int KWARGC=0, bool op_call=false);
    CodeObject_ compile(Str source, Str filename, CompileMode mode, bool unknown_global_scope=false);
    PyObject* py_negate(PyObject* obj);
    bool py_bool(PyObject* obj);
    i64 py_hash(PyObject* obj);
    PyObject* py_list(PyObject*);
    PyObject* new_module(Str name, Str package="");
    Str disassemble(CodeObject_ co);
    void init_builtin_types();
    PyObject* getattr(PyObject* obj, StrName name, bool throw_err=true);
    void delattr(PyObject* obj, StrName name);
    PyObject* get_unbound_method(PyObject* obj, StrName name, PyObject** self, bool throw_err=true, bool fallback=false);
    void parse_int_slice(const Slice& s, int length, int& start, int& stop, int& step);
    PyObject* _format_string(Str, PyObject*);
    void setattr(PyObject* obj, StrName name, PyObject* value);
    template<int ARGC>
    PyObject* bind_method(PyObject*, Str, NativeFuncC);
    template<int ARGC>
    PyObject* bind_func(PyObject*, Str, NativeFuncC);
    void _error(Exception);
    PyObject* _run_top_frame();
    void post_init();
    PyObject* _py_generator(Frame&& frame, ArgsView buffer);
    void _prepare_py_call(PyObject**, ArgsView, ArgsView, const FuncDecl_&);
    // new style binding api
    PyObject* bind(PyObject*, const char*, const char*, NativeFuncC, UserData userdata={});
    PyObject* bind(PyObject*, const char*, NativeFuncC, UserData userdata={});
    PyObject* bind_property(PyObject*, Str, NativeFuncC fget, NativeFuncC fset=nullptr);
};

DEF_NATIVE_2(Str, tp_str)
DEF_NATIVE_2(List, tp_list)
DEF_NATIVE_2(Tuple, tp_tuple)
DEF_NATIVE_2(Function, tp_function)
DEF_NATIVE_2(NativeFunc, tp_native_func)
DEF_NATIVE_2(BoundMethod, tp_bound_method)
DEF_NATIVE_2(Range, tp_range)
DEF_NATIVE_2(Slice, tp_slice)
DEF_NATIVE_2(Exception, tp_exception)
DEF_NATIVE_2(Bytes, tp_bytes)
DEF_NATIVE_2(MappingProxy, tp_mappingproxy)
DEF_NATIVE_2(Dict, tp_dict)
DEF_NATIVE_2(Property, tp_property)
DEF_NATIVE_2(StarWrapper, tp_star_wrapper)

#undef DEF_NATIVE_2

#define PY_CAST_INT(T)                                  \
template<> inline T py_cast<T>(VM* vm, PyObject* obj){  \
    if(is_small_int(obj)) return (T)(PK_BITS(obj) >> 2);    \
    if(is_heap_int(obj)) return (T)PK_OBJ_GET(i64, obj);    \
    vm->check_type(obj, vm->tp_int);                        \
    return 0;                                               \
}                                                           \
template<> inline T _py_cast<T>(VM* vm, PyObject* obj){     \
    PK_UNUSED(vm);                                          \
    if(is_small_int(obj)) return (T)(PK_BITS(obj) >> 2);    \
    return (T)PK_OBJ_GET(i64, obj);                         \
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
    if(is_float(obj)) return untag_float(obj);
    i64 bits;
    if(try_cast_int(obj, &bits)) return (float)bits;
    vm->TypeError("expected 'int' or 'float', got " + OBJ_NAME(vm->_t(obj)).escape());
    return 0;
}
template<> inline float _py_cast<float>(VM* vm, PyObject* obj){
    return py_cast<float>(vm, obj);
}
template<> inline double py_cast<double>(VM* vm, PyObject* obj){
    if(is_float(obj)) return untag_float(obj);
    i64 bits;
    if(try_cast_int(obj, &bits)) return (float)bits;
    vm->TypeError("expected 'int' or 'float', got " + OBJ_NAME(vm->_t(obj)).escape());
    return 0;
}
template<> inline double _py_cast<double>(VM* vm, PyObject* obj){
    return py_cast<double>(vm, obj);
}

#define PY_VAR_INT(T)                                       \
    inline PyObject* py_var(VM* vm, T _val){                \
        i64 val = static_cast<i64>(_val);                   \
        if(val >= Number::kMinSmallInt && val <= Number::kMaxSmallInt){     \
            val = (val << 2) | 0b10;                        \
            return reinterpret_cast<PyObject*>(val);        \
        }else{                                              \
            return vm->heap.gcnew<i64>(vm->tp_int, val);    \
        }                                                   \
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
#undef PY_VAR_INT

inline PyObject* py_var(VM* vm, float _val){
    PK_UNUSED(vm);
    return tag_float(static_cast<f64>(_val));
}

inline PyObject* py_var(VM* vm, double _val){
    PK_UNUSED(vm);
    return tag_float(static_cast<f64>(_val));
}

inline PyObject* py_var(VM* vm, bool val){
    return val ? vm->True : vm->False;
}

template<> inline bool py_cast<bool>(VM* vm, PyObject* obj){
    if(obj == vm->True) return true;
    if(obj == vm->False) return false;
    vm->check_non_tagged_type(obj, vm->tp_bool);
    return false;
}
template<> inline bool _py_cast<bool>(VM* vm, PyObject* obj){
    return obj == vm->True;
}

template<> inline CString py_cast<CString>(VM* vm, PyObject* obj){
    vm->check_non_tagged_type(obj, vm->tp_str);
    return PK_OBJ_GET(Str, obj).c_str();
}

template<> inline CString _py_cast<CString>(VM* vm, PyObject* obj){
    return PK_OBJ_GET(Str, obj).c_str();
}

inline PyObject* py_var(VM* vm, const char* val){
    return VAR(Str(val));
}

template<>
inline const char* py_cast<const char*>(VM* vm, PyObject* obj){
    if(obj == vm->None) return nullptr;
    vm->check_non_tagged_type(obj, vm->tp_str);
    return PK_OBJ_GET(Str, obj).c_str();
}

template<>
inline const char* _py_cast<const char*>(VM* vm, PyObject* obj){
    return PK_OBJ_GET(Str, obj).c_str();
}

inline PyObject* py_var(VM* vm, std::string val){
    return VAR(Str(std::move(val)));
}

inline PyObject* py_var(VM* vm, std::string_view val){
    return VAR(Str(val));
}

inline PyObject* py_var(VM* vm, NoReturn val){
    PK_UNUSED(val);
    return vm->None;
}

template<int ARGC>
PyObject* VM::bind_method(PyObject* obj, Str name, NativeFuncC fn) {
    check_non_tagged_type(obj, tp_type);
    PyObject* nf = VAR(NativeFunc(fn, ARGC, true));
    obj->attr().set(name, nf);
    return nf;
}

template<int ARGC>
PyObject* VM::bind_func(PyObject* obj, Str name, NativeFuncC fn) {
    PyObject* nf = VAR(NativeFunc(fn, ARGC, false));
    obj->attr().set(name, nf);
    return nf;
}

/***************************************************/

template<typename T>
PyObject* PyArrayGetItem(VM* vm, PyObject* obj, PyObject* index){
    static_assert(std::is_same_v<T, List> || std::is_same_v<T, Tuple>);
    const T& self = _CAST(T&, obj);

    if(is_non_tagged_type(index, vm->tp_slice)){
        const Slice& s = _CAST(Slice&, index);
        int start, stop, step;
        vm->parse_int_slice(s, self.size(), start, stop, step);
        List new_list;
        for(int i=start; step>0?i<stop:i>stop; i+=step) new_list.push_back(self[i]);
        return VAR(T(std::move(new_list)));
    }

    int i = CAST(int, index);
    i = vm->normalized_index(i, self.size());
    return self[i];
}
}   // namespace pkpy