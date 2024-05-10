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
#include "profiler.h"


namespace pkpy{

/* Stack manipulation macros */
// https://github.com/python/cpython/blob/3.9/Python/ceval.c#L1123
#define TOP()             (s_data.top())
#define SECOND()          (s_data.second())
#define THIRD()           (s_data.third())
#define STACK_SHRINK(n)   (s_data.shrink(n))
#define PUSH(v)           (s_data.push(v))
#define POP()             (s_data.pop())
#define POPX()            (s_data.popx())
#define STACK_VIEW(n)     (s_data.view(n))

typedef PyObject* (*BinaryFuncC)(VM*, PyObject*, PyObject*);
typedef void (*RegisterFunc)(VM*, PyObject*, PyObject*);

#if PK_ENABLE_PROFILER
struct NextBreakpoint{
    int callstack_size;
    int lineno;
    bool should_step_into;
    NextBreakpoint(): callstack_size(0) {}
    NextBreakpoint(int callstack_size, int lineno, bool should_step_into): callstack_size(callstack_size), lineno(lineno), should_step_into(should_step_into) {}
    void _step(VM* vm);
    bool empty() const { return callstack_size == 0; }
};
#endif

struct PyTypeInfo{
    PyObject* obj;      // never be garbage collected
    Type base;
    PyObject* mod;      // never be garbage collected
    StrName name;
    bool subclass_enabled;

    std::vector<StrName> annotated_fields = {};

    // unary operators
    Str (*m__repr__)(VM* vm, PyObject*) = nullptr;
    Str (*m__str__)(VM* vm, PyObject*) = nullptr;
    i64 (*m__hash__)(VM* vm, PyObject*) = nullptr;
    i64 (*m__len__)(VM* vm, PyObject*) = nullptr;
    PyObject* (*m__iter__)(VM* vm, PyObject*) = nullptr;
    unsigned (*m__next__)(VM* vm, PyObject*) = nullptr;
    PyObject* (*m__neg__)(VM* vm, PyObject*) = nullptr;
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

    // attributes
    void (*m__setattr__)(VM* vm, PyObject*, StrName, PyObject*) = nullptr;
    PyObject* (*m__getattr__)(VM* vm, PyObject*, StrName) = nullptr;
    bool (*m__delattr__)(VM* vm, PyObject*, StrName) = nullptr;

    // backdoors
    void (*on_end_subclass)(VM* vm, PyTypeInfo*) = nullptr;
};

struct ImportContext{
    PK_ALWAYS_PASS_BY_POINTER(ImportContext)

    std::vector<Str> pending;
    std::vector<bool> pending_is_init;   // a.k.a __init__.py

    ImportContext() {}

    struct Temp{
        PK_ALWAYS_PASS_BY_POINTER(Temp)

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

class VM {
    PK_ALWAYS_PASS_BY_POINTER(VM)
    
    VM* vm;     // self reference to simplify code
public:
    ManagedHeap heap;
    ValueStack s_data;
    CallStack callstack;
    std::vector<PyTypeInfo> _all_types;
    
    NameDict _modules;                                 // loaded modules
    std::map<StrName, Str> _lazy_modules;              // lazy loaded modules

    struct{
        PyObject* error;
        stack_no_copy<ArgsView> s_view;
    } __c;

    PyObject *None, *True, *False, *NotImplemented;
    PyObject *StopIteration, *Ellipsis;
    PyObject *builtins, *_main;

    // typeid -> Type
    std::map<const std::type_index, Type> _cxx_typeid_map;
    // this is for repr() recursion detection (no need to mark)
    std::set<PyObject*> _repr_recursion_set;

    ImportContext __import_context;
    PyObject* __last_exception;
    PyObject* __curr_class; 
    PyObject* __cached_object_new;
    std::map<std::string_view, CodeObject_> __cached_codes;
    FuncDecl_ __dynamic_func_decl;

#if PK_ENABLE_PROFILER
    LineProfiler* _profiler = nullptr;
    NextBreakpoint _next_breakpoint;
#endif

    void (*_ceval_on_step)(VM*, Frame*, Bytecode bc);
    void(*_stdout)(const char*, int);
    void(*_stderr)(const char*, int);
    unsigned char* (*_import_handler)(const char*, int*);
    // function<void(const char*, int)> _stdout;
    // function<void(const char*, int)> _stderr;
    // function<unsigned char*(const char*, int*)> _import_handler;
    
    // for quick access
    static constexpr Type tp_object=Type(0), tp_type=Type(1);
    static constexpr Type tp_int=Type(kTpIntIndex), tp_float=Type(kTpFloatIndex), tp_bool=Type(4), tp_str=Type(5);
    static constexpr Type tp_list=Type(6), tp_tuple=Type(7);
    static constexpr Type tp_slice=Type(8), tp_range=Type(9), tp_module=Type(10);
    static constexpr Type tp_function=Type(11), tp_native_func=Type(12), tp_bound_method=Type(13);
    static constexpr Type tp_super=Type(14), tp_exception=Type(15), tp_bytes=Type(16), tp_mappingproxy=Type(17);
    static constexpr Type tp_dict=Type(18), tp_property=Type(19), tp_star_wrapper=Type(20);
    static constexpr Type tp_staticmethod=Type(21), tp_classmethod=Type(22);

    const bool enable_os;
    VM(bool enable_os=true);

#if PK_REGION("Python Equivalents")
    Str py_str(PyObject* obj);                              // x -> str(x)
    Str py_repr(PyObject* obj);                             // x -> repr(x)
    Str py_json(PyObject* obj);                             // x -> json.dumps(x)

    PyObject* py_iter(PyObject* obj);                       // x -> iter(x)
    PyObject* py_next(PyObject*);                           // x -> next(x)
    PyObject* _py_next(const PyTypeInfo*, PyObject*);       // x -> next(x) with type info cache
    PyObject* py_import(Str path, bool throw_err=true);     // x -> __import__(x)
    PyObject* py_negate(PyObject* obj);                     // x -> -x

    List py_list(PyObject*);                                // x -> list(x)
    bool py_callable(PyObject* obj);                        // x -> callable(x)
    bool py_bool(PyObject* obj);                            // x -> bool(x)
    i64 py_hash(PyObject* obj);                             // x -> hash(x)

    bool py_eq(PyObject* lhs, PyObject* rhs);               // (lhs, rhs) -> lhs == rhs
    bool py_lt(PyObject* lhs, PyObject* rhs);               // (lhs, rhs) -> lhs < rhs
    bool py_le(PyObject* lhs, PyObject* rhs);               // (lhs, rhs) -> lhs <= rhs
    bool py_gt(PyObject* lhs, PyObject* rhs);               // (lhs, rhs) -> lhs > rhs
    bool py_ge(PyObject* lhs, PyObject* rhs);               // (lhs, rhs) -> lhs >= rhs
    bool py_ne(PyObject* lhs, PyObject* rhs) {              // (lhs, rhs) -> lhs != rhs
        return !py_eq(lhs, rhs);
    }

    void py_exec(std::string_view, PyObject*, PyObject*);       // exec(source, globals, locals)
    PyObject* py_eval(std::string_view, PyObject*, PyObject*);  // eval(source, globals, locals)
#endif

#if PK_REGION("Utility Methods")
    ArgsView cast_array_view(PyObject* obj);
    void set_main_argv(int argc, char** argv);
    i64 normalized_index(i64 index, int size);
    Str disassemble(CodeObject_ co);
    void parse_int_slice(const Slice& s, int length, int& start, int& stop, int& step);
#endif

#if PK_REGION("Name Lookup Methods")
    PyObject* find_name_in_mro(Type cls, StrName name);
    PyObject* get_unbound_method(PyObject* obj, StrName name, PyObject** self, bool throw_err=true, bool fallback=false);
    PyObject* getattr(PyObject* obj, StrName name, bool throw_err=true);
    void delattr(PyObject* obj, StrName name);
    void setattr(PyObject* obj, StrName name, PyObject* value);
#endif

#if PK_REGION("Source Execution Methods")
    CodeObject_ compile(std::string_view source, const Str& filename, CompileMode mode, bool unknown_global_scope=false);
    Str precompile(std::string_view source, const Str& filename, CompileMode mode);
    PyObject* exec(std::string_view source, Str filename, CompileMode mode, PyObject* _module=nullptr);
    PyObject* exec(std::string_view source);
    PyObject* eval(std::string_view source);

    template<typename ...Args>
    PyObject* _exec(Args&&... args){
        callstack.emplace(s_data._sp, std::forward<Args>(args)...);
        return __run_top_frame();
    }
#endif

#if PK_REGION("Invocation Methods")
    PyObject* vectorcall(int ARGC, int KWARGC=0, bool op_call=false);

    template<typename... Args>
    PyObject* call(PyObject* callable, Args&&... args){
        PUSH(callable); PUSH(PY_NULL);
        __push_varargs(args...);
        return vectorcall(sizeof...(args));
    }

    template<typename... Args>
    PyObject* call_method(PyObject* self, PyObject* callable, Args&&... args){
        PUSH(callable); PUSH(self);
        __push_varargs(args...);
        return vectorcall(sizeof...(args));
    }

    template<typename... Args>
    PyObject* call_method(PyObject* self, StrName name, Args&&... args){
        PyObject* callable = get_unbound_method(self, name, &self);
        return call_method(self, callable, args...);
    }
#endif

#if PK_REGION("Logging Methods")
    virtual void stdout_write(const Str& s){ _stdout(s.data, s.size); }
    virtual void stderr_write(const Str& s){ _stderr(s.data, s.size); }
#endif

#if PK_REGION("Magic Bindings")
    void bind__repr__(Type type, Str (*f)(VM*, PyObject*));
    void bind__str__(Type type, Str (*f)(VM*, PyObject*));
    void bind__iter__(Type type, PyObject* (*f)(VM*, PyObject*));

    void bind__next__(Type type, unsigned (*f)(VM*, PyObject*));
    [[deprecated]] void bind__next__(Type type, PyObject* (*f)(VM*, PyObject*));
    void bind__neg__(Type type, PyObject* (*f)(VM*, PyObject*));
    void bind__invert__(Type type, PyObject* (*f)(VM*, PyObject*));
    void bind__hash__(Type type, i64 (*f)(VM* vm, PyObject*));
    void bind__len__(Type type, i64 (*f)(VM* vm, PyObject*));

    void bind__eq__(Type type, BinaryFuncC f);
    void bind__lt__(Type type, BinaryFuncC f);
    void bind__le__(Type type, BinaryFuncC f);
    void bind__gt__(Type type, BinaryFuncC f);
    void bind__ge__(Type type, BinaryFuncC f);
    void bind__contains__(Type type, BinaryFuncC f);

    void bind__add__(Type type, BinaryFuncC f);
    void bind__sub__(Type type, BinaryFuncC f);
    void bind__mul__(Type type, BinaryFuncC f);
    void bind__truediv__(Type type, BinaryFuncC f);
    void bind__floordiv__(Type type, BinaryFuncC f);
    void bind__mod__(Type type, BinaryFuncC f);
    void bind__pow__(Type type, BinaryFuncC f);
    void bind__matmul__(Type type, BinaryFuncC f);

    void bind__lshift__(Type type, BinaryFuncC f);
    void bind__rshift__(Type type, BinaryFuncC f);
    void bind__and__(Type type, BinaryFuncC f);
    void bind__or__(Type type, BinaryFuncC f);
    void bind__xor__(Type type, BinaryFuncC f);

    void bind__getitem__(Type type, PyObject* (*f)(VM*, PyObject*, PyObject*));
    void bind__setitem__(Type type, void (*f)(VM*, PyObject*, PyObject*, PyObject*));
    void bind__delitem__(Type type, void (*f)(VM*, PyObject*, PyObject*));
#endif

#if PK_REGION("General Bindings")
    PyObject* bind_func(PyObject* obj, StrName name, int argc, NativeFuncC fn, any userdata={}, BindType bt=BindType::DEFAULT);
    PyObject* bind_func(Type type, StrName name, int argc, NativeFuncC fn, any userdata={}, BindType bt=BindType::DEFAULT){
        return bind_func(_t(type), name, argc, fn, std::move(userdata), bt);
    }
    PyObject* bind_property(PyObject*, const char*, NativeFuncC fget, NativeFuncC fset=nullptr);
    template<typename T, typename F, bool ReadOnly=false>
    PyObject* bind_field(PyObject*, const char*, F T::*);

    PyObject* bind(PyObject*, const char*, NativeFuncC, any userdata={}, BindType bt=BindType::DEFAULT);
    template<typename Ret, typename... Params>
    PyObject* bind(PyObject*, const char*, Ret(*)(Params...), BindType bt=BindType::DEFAULT);
    template<typename Ret, typename T, typename... Params>
    PyObject* bind(PyObject*, const char*, Ret(T::*)(Params...), BindType bt=BindType::DEFAULT);

    PyObject* bind(PyObject*, const char*, const char*, NativeFuncC, any userdata={}, BindType bt=BindType::DEFAULT);
    template<typename Ret, typename... Params>
    PyObject* bind(PyObject*, const char*, const char*, Ret(*)(Params...), BindType bt=BindType::DEFAULT);
    template<typename Ret, typename T, typename... Params>
    PyObject* bind(PyObject*, const char*, const char*, Ret(T::*)(Params...), BindType bt=BindType::DEFAULT);
#endif

#if PK_REGION("Error Reporting Methods")
    void _error(PyObject*);
    void StackOverflowError() { __builtin_error("StackOverflowError"); }
    void IOError(const Str& msg) { __builtin_error("IOError", msg); }
    void NotImplementedError(){ __builtin_error("NotImplementedError"); }
    void TypeError(const Str& msg){ __builtin_error("TypeError", msg); }
    void TypeError(Type expected, Type actual) { TypeError("expected " + _type_name(vm, expected).escape() + ", got " + _type_name(vm, actual).escape()); }
    void IndexError(const Str& msg){ __builtin_error("IndexError", msg); }
    void ValueError(const Str& msg){ __builtin_error("ValueError", msg); }
    void RuntimeError(const Str& msg){ __builtin_error("RuntimeError", msg); }
    void ZeroDivisionError(const Str& msg){ __builtin_error("ZeroDivisionError", msg); }
    void ZeroDivisionError(){ __builtin_error("ZeroDivisionError", "division by zero"); }
    void NameError(StrName name){ __builtin_error("NameError", _S("name ", name.escape() + " is not defined")); }
    void UnboundLocalError(StrName name){ __builtin_error("UnboundLocalError", _S("local variable ", name.escape() + " referenced before assignment")); }
    void KeyError(PyObject* obj){ __builtin_error("KeyError", obj); }
    void ImportError(const Str& msg){ __builtin_error("ImportError", msg); }
    void AssertionError(const Str& msg){ __builtin_error("AssertionError", msg); }
    void AssertionError(){ __builtin_error("AssertionError"); }
    void BinaryOptError(const char* op, PyObject* _0, PyObject* _1);
    void AttributeError(PyObject* obj, StrName name);
    void AttributeError(const Str& msg){ __builtin_error("AttributeError", msg); }
#endif

#if PK_REGION("Type Checking Methods")
    bool isinstance(PyObject* obj, Type base);
    bool issubclass(Type cls, Type base);
    void check_type(PyObject* obj, Type type){ if(!is_type(obj, type)) TypeError(type, _tp(obj)); }
    void check_compatible_type(PyObject* obj, Type type){ if(!isinstance(obj, type)) TypeError(type, _tp(obj)); }

    Type _tp(PyObject* obj){ return is_small_int(obj) ? tp_int : obj->type; }
    const PyTypeInfo* _tp_info(PyObject* obj) { return &_all_types[_tp(obj)]; }
    const PyTypeInfo* _tp_info(Type type) { return &_all_types[type]; }
    PyObject* _t(PyObject* obj){ return _all_types[_tp(obj)].obj; }
    PyObject* _t(Type type){ return _all_types[type].obj; }
#endif

#if PK_REGION("User Type Registration")
    PyObject* new_module(Str name, Str package="");
    PyObject* new_type_object(PyObject* mod, StrName name, Type base, bool subclass_enabled=true);

    template<typename T>
    Type _tp_user(){ return _find_type_in_cxx_typeid_map<T>(); }
    template<typename T>
    bool is_user_type(PyObject* obj){ return _tp(obj) == _tp_user<T>(); }

    template<typename T>
    PyObject* register_user_class(PyObject*, StrName, RegisterFunc, Type base=tp_object, bool subclass_enabled=false);
    template<typename T>
    PyObject* register_user_class(PyObject*, StrName, Type base=tp_object, bool subclass_enabled=false);

    template<typename T, typename ...Args>
    PyObject* new_user_object(Args&&... args){
        return heap.gcnew<T>(_tp_user<T>(), std::forward<Args>(args)...);
    }
#endif

    template<typename T>
    Type _find_type_in_cxx_typeid_map(){
        auto it = _cxx_typeid_map.find(typeid(T));
        if(it == _cxx_typeid_map.end()){
    #if __GNUC__ || __clang__
            throw std::runtime_error(__PRETTY_FUNCTION__ + std::string(" failed: T not found"));
    #elif _MSC_VER
            throw std::runtime_error(__FUNCSIG__ + std::string(" failed: T not found"));
    #else
            throw std::runtime_error("_find_type_in_cxx_typeid_map() failed: T not found");
    #endif
        }
        return it->second;
    }

    /********** private **********/
    virtual ~VM();

#if PK_DEBUG_CEVAL_STEP
    void __log_s_data(const char* title = nullptr);
#endif
    PyObject* __py_exec_internal(const CodeObject_& code, PyObject* globals, PyObject* locals);
    void __breakpoint();
    PyObject* __format_object(PyObject*, Str);
    PyObject* __run_top_frame();
    void __pop_frame();
    PyObject* __py_generator(Frame&& frame, ArgsView buffer);
    void __op_unpack_sequence(uint16_t arg);
    void __prepare_py_call(PyObject**, ArgsView, ArgsView, const FuncDecl_&);
    void __unpack_as_list(ArgsView args, List& list);
    void __unpack_as_dict(ArgsView args, Dict& dict);
    void __raise_exc(bool re_raise=false);
    void __init_builtin_types();
    void __post_init_builtin_types();
    void __builtin_error(StrName type);
    void __builtin_error(StrName type, PyObject* arg);
    void __builtin_error(StrName type, const Str& msg);
    void __push_varargs(){}
    void __push_varargs(PyObject* _0){ PUSH(_0); }
    void __push_varargs(PyObject* _0, PyObject* _1){ PUSH(_0); PUSH(_1); }
    void __push_varargs(PyObject* _0, PyObject* _1, PyObject* _2){ PUSH(_0); PUSH(_1); PUSH(_2); }
    void __push_varargs(PyObject* _0, PyObject* _1, PyObject* _2, PyObject* _3){ PUSH(_0); PUSH(_1); PUSH(_2); PUSH(_3); }
    PyObject* __pack_next_retval(unsigned);
    PyObject* __minmax_reduce(bool (VM::*op)(PyObject*, PyObject*), PyObject* args, PyObject* key);
};


template<typename T>
inline constexpr bool is_immutable_v = is_integral_v<T> || is_floating_point_v<T>
    || std::is_same_v<T, Str> || std::is_same_v<T, Tuple> || std::is_same_v<T, Bytes> || std::is_same_v<T, bool>
    || std::is_same_v<T, Range> || std::is_same_v<T, Slice>
    || std::is_pointer_v<T> || std::is_enum_v<T>;

template<typename T> constexpr Type _find_type_in_const_cxx_typeid_map(){ return Type(-1); }
template<> constexpr Type _find_type_in_const_cxx_typeid_map<Str>(){ return VM::tp_str; }
template<> constexpr Type _find_type_in_const_cxx_typeid_map<List>(){ return VM::tp_list; }
template<> constexpr Type _find_type_in_const_cxx_typeid_map<Tuple>(){ return VM::tp_tuple; }
template<> constexpr Type _find_type_in_const_cxx_typeid_map<Function>(){ return VM::tp_function; }
template<> constexpr Type _find_type_in_const_cxx_typeid_map<NativeFunc>(){ return VM::tp_native_func; }
template<> constexpr Type _find_type_in_const_cxx_typeid_map<BoundMethod>(){ return VM::tp_bound_method; }
template<> constexpr Type _find_type_in_const_cxx_typeid_map<Range>(){ return VM::tp_range; }
template<> constexpr Type _find_type_in_const_cxx_typeid_map<Slice>(){ return VM::tp_slice; }
template<> constexpr Type _find_type_in_const_cxx_typeid_map<Exception>(){ return VM::tp_exception; }
template<> constexpr Type _find_type_in_const_cxx_typeid_map<Bytes>(){ return VM::tp_bytes; }
template<> constexpr Type _find_type_in_const_cxx_typeid_map<MappingProxy>(){ return VM::tp_mappingproxy; }
template<> constexpr Type _find_type_in_const_cxx_typeid_map<Dict>(){ return VM::tp_dict; }
template<> constexpr Type _find_type_in_const_cxx_typeid_map<Property>(){ return VM::tp_property; }
template<> constexpr Type _find_type_in_const_cxx_typeid_map<StarWrapper>(){ return VM::tp_star_wrapper; }
template<> constexpr Type _find_type_in_const_cxx_typeid_map<StaticMethod>(){ return VM::tp_staticmethod; }
template<> constexpr Type _find_type_in_const_cxx_typeid_map<ClassMethod>(){ return VM::tp_classmethod; }

template<typename __T>
PyObject* py_var(VM* vm, __T&& value){
    using T = std::decay_t<__T>;

    static_assert(!std::is_same_v<T, PyObject*>, "py_var(VM*, PyObject*) is not allowed");

    if constexpr(std::is_same_v<T, const char*> || std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>){
        // str (shortcuts)
        return VAR(Str(std::forward<__T>(value)));
    }else if constexpr(std::is_same_v<T, NoReturn>){
        // NoneType
        return vm->None;
    }else if constexpr(std::is_same_v<T, bool>){
        // bool
        return value ? vm->True : vm->False;
    }else if constexpr(is_integral_v<T>){
        // int
        i64 val = static_cast<i64>(std::forward<__T>(value));
        if(val >= Number::kMinSmallInt && val <= Number::kMaxSmallInt){
            val = (val << 2) | 0b10;
            return reinterpret_cast<PyObject*>(val);
        }else{
            return vm->heap.gcnew<i64>(vm->tp_int, val);
        }
    }else if constexpr(is_floating_point_v<T>){
        // float
        f64 val = static_cast<f64>(std::forward<__T>(value));
        return vm->heap.gcnew<f64>(vm->tp_float, val);
    }else if constexpr(std::is_pointer_v<T>){
        return from_void_p(vm, (void*)value);
    }else{
        constexpr Type const_type = _find_type_in_const_cxx_typeid_map<T>();
        if constexpr(const_type.index >= 0){
            return vm->heap.gcnew<T>(const_type, std::forward<__T>(value));
        }
    }
    Type type = vm->_find_type_in_cxx_typeid_map<T>();
    return vm->heap.gcnew<T>(type, std::forward<__T>(value));
}

template<typename __T, bool with_check>
__T _py_cast__internal(VM* vm, PyObject* obj) {
    static_assert(!std::is_rvalue_reference_v<__T>, "rvalue reference is not allowed");

    using T = std::decay_t<__T>;

    if constexpr(std::is_same_v<T, const char*> || std::is_same_v<T, CString>){
        static_assert(!std::is_reference_v<__T>);
        // str (shortcuts)
        if(obj == vm->None) return nullptr;
        if constexpr(with_check) vm->check_type(obj, vm->tp_str);
        return PK_OBJ_GET(Str, obj).c_str();
    }else if constexpr(std::is_same_v<T, bool>){
        static_assert(!std::is_reference_v<__T>);
        // bool
        if constexpr(with_check){
            if(obj == vm->True) return true;
            if(obj == vm->False) return false;
            vm->TypeError("expected 'bool', got " + _type_name(vm, vm->_tp(obj)).escape());
        }else{
            return obj == vm->True;
        }
    }else if constexpr(is_integral_v<T>){
        static_assert(!std::is_reference_v<__T>);
        // int
        if constexpr(with_check){
            if(is_small_int(obj)) return (T)(PK_BITS(obj) >> 2);
            if(is_heap_int(obj)) return (T)PK_OBJ_GET(i64, obj);
            vm->TypeError("expected 'int', got " + _type_name(vm, vm->_tp(obj)).escape());
        }else{
            if(is_small_int(obj)) return (T)(PK_BITS(obj) >> 2);
            return (T)PK_OBJ_GET(i64, obj);
        }
    }else if constexpr(is_floating_point_v<T>){
        static_assert(!std::is_reference_v<__T>);
        // float
        if(is_float(obj)) return PK_OBJ_GET(f64, obj);
        i64 bits;
        if(try_cast_int(obj, &bits)) return (float)bits;
        vm->TypeError("expected 'int' or 'float', got " + _type_name(vm, vm->_tp(obj)).escape());
    }else if constexpr(std::is_enum_v<T>){
        static_assert(!std::is_reference_v<__T>);
        return (__T)_py_cast__internal<i64, with_check>(vm, obj);
    }else if constexpr(std::is_pointer_v<T>){
        static_assert(!std::is_reference_v<__T>);
        return to_void_p<T>(vm, obj);
    }else{
        constexpr Type const_type = _find_type_in_const_cxx_typeid_map<T>();
        if constexpr(const_type.index >= 0){
            if constexpr(with_check){
                if constexpr(std::is_same_v<T, Exception>){
                    // Exception is `subclass_enabled`
                    vm->check_compatible_type(obj, const_type);
                }else{
                    vm->check_type(obj, const_type);
                }
            }
            return PK_OBJ_GET(T, obj);
        }
    }
    Type type = vm->_find_type_in_cxx_typeid_map<T>();
    if constexpr(with_check) vm->check_compatible_type(obj, type);
    return PK_OBJ_GET(T, obj);
}

template<typename __T>
__T  py_cast(VM* vm, PyObject* obj) { return _py_cast__internal<__T, true>(vm, obj); }
template<typename __T>
__T _py_cast(VM* vm, PyObject* obj) { return _py_cast__internal<__T, false>(vm, obj); }

template<typename T>
PyObject* VM::register_user_class(PyObject* mod, StrName name, RegisterFunc _register, Type base, bool subclass_enabled){
    PyObject* type = new_type_object(mod, name, base, subclass_enabled);
    mod->attr().set(name, type);
    _cxx_typeid_map[typeid(T)] = PK_OBJ_GET(Type, type);
    _register(this, mod, type);
    if(!type->attr().contains(__new__)){
        if constexpr(std::is_default_constructible_v<T>) {
            bind_func(type, __new__, -1, [](VM* vm, ArgsView args){
                Type cls_t = PK_OBJ_GET(Type, args[0]);
                return vm->heap.gcnew<T>(cls_t);
            });
        }else{
            bind_func(type, __new__, -1, PK_ACTION(vm->NotImplementedError()));
        }
    }
    return type;
}

template<typename T>
PyObject* VM::register_user_class(PyObject* mod, StrName name, Type base, bool subclass_enabled){
    return register_user_class<T>(mod, name, &T::_register, base, subclass_enabled);
}

}   // namespace pkpy