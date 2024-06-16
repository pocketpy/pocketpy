#pragma once

#include "pocketpy/objects/object.hpp"
#include "pocketpy/objects/dict.hpp"
#include "pocketpy/objects/error.hpp"
#include "pocketpy/objects/builtins.hpp"
#include "pocketpy/interpreter/gc.hpp"
#include "pocketpy/interpreter/frame.hpp"
#include "pocketpy/interpreter/profiler.hpp"

#include <typeindex>

namespace pkpy {

/* Stack manipulation macros */
// https://github.com/python/cpython/blob/3.9/Python/ceval.c#L1123
#define TOP() (s_data.top())
#define SECOND() (s_data.second())
#define THIRD() (s_data.third())
#define STACK_SHRINK(n) (s_data.shrink(n))
#define PUSH(v) (s_data.push(v))
#define POP() (s_data.pop())
#define POPX() (s_data.popx())
#define STACK_VIEW(n) (s_data.view(n))

typedef PyVar (*BinaryFuncC)(VM*, PyVar, PyVar);
typedef void (*RegisterFunc)(VM*, PyObject*, PyObject*);

#if PK_ENABLE_PROFILER
struct NextBreakpoint {
    int callstack_size;
    int lineno;
    bool should_step_into;

    NextBreakpoint() : callstack_size(0) {}

    NextBreakpoint(int callstack_size, int lineno, bool should_step_into) :
        callstack_size(callstack_size), lineno(lineno), should_step_into(should_step_into) {}

    void _step(VM* vm);

    bool empty() const { return callstack_size == 0; }
};
#endif

struct PyTypeInfo {
    struct Vt {
        void (*_dtor)(void*);
        void (*_gc_mark)(void*, VM*);

        Vt() : _dtor(nullptr), _gc_mark(nullptr) {}

        operator bool () const { return _dtor || _gc_mark; }

        template <typename T>
        inline static Vt get() {
            static_assert(std::is_same_v<T, std::decay_t<T>>);
            Vt vt;
            if constexpr(!std::is_trivially_destructible_v<T>) {
                vt._dtor = [](void* p) {
                    ((T*)p)->~T();
                };
            }
            if constexpr(has_gc_marker<T>::value) {
                vt._gc_mark = [](void* p, VM* vm) {
                    ((T*)p)->_gc_mark(vm);
                };
            }
            return vt;
        }
    };

    PyObject* obj;  // never be garbage collected
    Type base;
    PyObject* mod;  // never be garbage collected
    StrName name;
    bool subclass_enabled;
    Vt vt;

    PyTypeInfo(PyObject* obj, Type base, PyObject* mod, StrName name, bool subclass_enabled, Vt vt = {}) :
        obj(obj), base(base), mod(mod), name(name), subclass_enabled(subclass_enabled), vt(vt) {}

    vector<StrName> annotated_fields;

    // unary operators
    Str (*m__repr__)(VM* vm, PyVar) = nullptr;
    Str (*m__str__)(VM* vm, PyVar) = nullptr;
    i64 (*m__hash__)(VM* vm, PyVar) = nullptr;
    i64 (*m__len__)(VM* vm, PyVar) = nullptr;
    PyVar (*m__iter__)(VM* vm, PyVar) = nullptr;
    unsigned (*op__next__)(VM* vm, PyVar) = nullptr;
    PyVar (*m__neg__)(VM* vm, PyVar) = nullptr;
    PyVar (*m__invert__)(VM* vm, PyVar) = nullptr;

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
    PyVar (*m__getitem__)(VM* vm, PyVar, PyVar) = nullptr;
    void (*m__setitem__)(VM* vm, PyVar, PyVar, PyVar) = nullptr;
    void (*m__delitem__)(VM* vm, PyVar, PyVar) = nullptr;

    // attributes
    void (*m__setattr__)(VM* vm, PyVar, StrName, PyVar) = nullptr;
    PyVar (*m__getattr__)(VM* vm, PyVar, StrName) = nullptr;
    bool (*m__delattr__)(VM* vm, PyVar, StrName) = nullptr;

    // backdoors
    void (*on_end_subclass)(VM* vm, PyTypeInfo*) = nullptr;
};

struct ImportContext {
    PK_ALWAYS_PASS_BY_POINTER(ImportContext)

    vector<Str> pending;
    vector<bool> pending_is_init;  // a.k.a __init__.py

    ImportContext() {}

    struct Temp {
        PK_ALWAYS_PASS_BY_POINTER(Temp)

        ImportContext* ctx;

        Temp(ImportContext* ctx, Str name, bool is_init) : ctx(ctx) {
            ctx->pending.push_back(name);
            ctx->pending_is_init.push_back(is_init);
        }

        ~Temp() {
            ctx->pending.pop_back();
            ctx->pending_is_init.pop_back();
        }
    };

    Temp scope(Str name, bool is_init) { return {this, name, is_init}; }
};

class VM {
    PK_ALWAYS_PASS_BY_POINTER(VM)

    VM* vm;  // self reference to simplify code

public:
    ManagedHeap heap;
    ValueStack s_data;
    CallStack callstack;
    vector<PyTypeInfo> _all_types;

    NameDict _modules;                      // loaded modules
    small_map<StrName, Str> _lazy_modules;  // lazy loaded modules

    struct {
        PyObject* error;
        vector<ArgsView> s_view;
    } __c;

    PyVar StopIteration;  // a special Exception class
    PyObject* builtins;
    PyObject* _main;

    // typeid -> Type
    small_map<std::type_index, Type> _cxx_typeid_map;
    // this is for repr() recursion detection (no need to mark)
    vector<PyVar> _repr_recursion_set;

    ImportContext __import_context;
    PyObject* __last_exception;
    PyObject* __curr_class;
    PyVar __cached_object_new;
    small_map<std::string_view, PyVar> __cached_op_funcs;
    FuncDecl_ __dynamic_func_decl;
    PyVar __vectorcall_buffer[PK_MAX_CO_VARNAMES];

#if PK_ENABLE_PROFILER
    LineProfiler* _profiler = nullptr;
    NextBreakpoint _next_breakpoint;
#endif

    void (*_ceval_on_step)(VM*, Frame*, Bytecode bc);
    void (*_stdout)(const char*, int);
    void (*_stderr)(const char*, int);
    unsigned char* (*_import_handler)(const char*, int*);
    // function<void(const char*, int)> _stdout;
    // function<void(const char*, int)> _stderr;
    // function<unsigned char*(const char*, int*)> _import_handler;

    // for quick access
    constexpr static Type tp_object = Type(1), tp_type = Type(2);
    constexpr static Type tp_int = Type(kTpIntIndex), tp_float = Type(kTpFloatIndex), tp_bool = Type(5),
                          tp_str = Type(6);
    constexpr static Type tp_list = Type(7), tp_tuple = Type(8);
    constexpr static Type tp_slice = Type(9), tp_range = Type(10), tp_module = Type(11);
    constexpr static Type tp_function = Type(12), tp_native_func = Type(13), tp_bound_method = Type(14);
    constexpr static Type tp_super = Type(15), tp_exception = Type(16), tp_bytes = Type(17), tp_mappingproxy = Type(18);
    constexpr static Type tp_dict = Type(19), tp_property = Type(20), tp_star_wrapper = Type(21);
    constexpr static Type tp_staticmethod = Type(22), tp_classmethod = Type(23);
    constexpr static Type tp_none_type = Type(kTpNoneTypeIndex), tp_not_implemented_type = Type(kTpNotImplementedTypeIndex);
    constexpr static Type tp_ellipsis = Type(26);

    inline static PyVar True = pkpy_True;
    inline static PyVar False = pkpy_False;
    inline static PyVar None = pkpy_None;
    inline static PyVar NotImplemented = pkpy_NotImplemented;
    inline static PyVar Ellipsis = pkpy_Ellipsis;

    const bool enable_os;
    VM(bool enable_os = true);

    // clang-format off
#if PK_REGION("Python Equivalents")
    Str py_str(PyVar obj);                              // x -> str(x)
    Str py_repr(PyVar obj);                             // x -> repr(x)
    Str py_json(PyVar obj);                             // x -> json.dumps(x)

    PyVar py_iter(PyVar obj);                           // x -> iter(x)
    PyVar py_next(PyVar);                               // x -> next(x)
    PyVar _py_next(const PyTypeInfo*, PyVar);           // x -> next(x) with type info cache
    PyObject* py_import(Str path, bool throw_err=true); // x -> __import__(x)
    PyVar py_negate(PyVar obj);                         // x -> -x

    List py_list(PyVar);                                // x -> list(x)
    bool py_callable(PyVar obj);                        // x -> callable(x)
    bool py_bool(PyVar obj){                            // x -> bool(x)
        if(obj.type == tp_bool) return obj._bool;
        return __py_bool_non_trivial(obj);
    }
    i64 py_hash(PyVar obj);                             // x -> hash(x)

    bool py_eq(PyVar lhs, PyVar rhs);                   // (lhs, rhs) -> lhs == rhs
    bool py_lt(PyVar lhs, PyVar rhs);                   // (lhs, rhs) -> lhs < rhs
    bool py_le(PyVar lhs, PyVar rhs);                   // (lhs, rhs) -> lhs <= rhs
    bool py_gt(PyVar lhs, PyVar rhs);                   // (lhs, rhs) -> lhs > rhs
    bool py_ge(PyVar lhs, PyVar rhs);                   // (lhs, rhs) -> lhs >= rhs
    bool py_ne(PyVar lhs, PyVar rhs){                   // (lhs, rhs) -> lhs != rhs
        return !py_eq(lhs, rhs);
    }

    PyVar py_op(std::string_view name);                 // (name) -> operator.name

    void py_exec(std::string_view, PyVar, PyVar);       // exec(source, globals, locals)
    PyVar py_eval(std::string_view, PyVar, PyVar);      // eval(source, globals, locals)
#endif

#if PK_REGION("Utility Methods")
    ArgsView cast_array_view(PyVar obj);
    void set_main_argv(int argc, char** argv);
    i64 normalized_index(i64 index, int size);
    Str disassemble(CodeObject_ co);
    void parse_int_slice(const Slice& s, int length, int& start, int& stop, int& step);
    void obj_gc_mark(PyVar obj) { if(obj.is_ptr) __obj_gc_mark(obj.get()); }
    void obj_gc_mark(PyObject* p) { if(p) __obj_gc_mark(p); }
#endif

#if PK_REGION("Name Lookup Methods")
    PyVar* find_name_in_mro(Type cls, StrName name);
    PyVar get_unbound_method(PyVar obj, StrName name, PyVar* self, bool throw_err=true, bool fallback=false);
    PyVar getattr(PyVar obj, StrName name, bool throw_err=true);
    void delattr(PyVar obj, StrName name);
    void setattr(PyVar obj, StrName name, PyVar value);
#endif

#if PK_REGION("Source Execution Methods")
    CodeObject_ compile(std::string_view source, const Str& filename, CompileMode mode, bool unknown_global_scope=false);
    Str precompile(std::string_view source, const Str& filename, CompileMode mode);
    PyVar exec(std::string_view source, Str filename, CompileMode mode, PyObject* _module=nullptr);
    PyVar exec(std::string_view source);
    PyVar eval(std::string_view source);

    template<typename ...Args>
    PyVar _exec(Args&&... args){
        callstack.emplace(s_data._sp, std::forward<Args>(args)...);
        return __run_top_frame();
    }
#endif

#if PK_REGION("Invocation Methods")
    PyVar vectorcall(int ARGC, int KWARGC=0, bool op_call=false);

    template<typename... Args>
    PyVar call(PyVar callable, Args&&... args){
        PUSH(callable); PUSH(PY_NULL);
        __push_varargs(args...);
        return vectorcall(sizeof...(args));
    }

    template<typename... Args>
    PyVar call_method(PyVar self, PyVar callable, Args&&... args){
        PUSH(callable); PUSH(self);
        __push_varargs(args...);
        return vectorcall(sizeof...(args));
    }

    template<typename... Args>
    PyVar call_method(PyVar self, StrName name, Args&&... args){
        PyVar callable = get_unbound_method(self, name, &self);
        return call_method(self, callable, args...);
    }
#endif

#if PK_REGION("Logging Methods")
    virtual void stdout_write(const Str& s){ _stdout(s.c_str(), s.size); }
    virtual void stderr_write(const Str& s){ _stderr(s.c_str(), s.size); }
#endif

#if PK_REGION("Magic Bindings")
    void bind__repr__(Type type, Str (*f)(VM*, PyVar));
    void bind__str__(Type type, Str (*f)(VM*, PyVar));
    void bind__iter__(Type type, PyVar (*f)(VM*, PyVar));

    void bind__next__(Type type, unsigned (*f)(VM*, PyVar));
    [[deprecated]] void bind__next__(Type type, PyVar (*f)(VM*, PyVar));
    void bind__neg__(Type type, PyVar (*f)(VM*, PyVar));
    void bind__invert__(Type type, PyVar (*f)(VM*, PyVar));
    void bind__hash__(Type type, i64 (*f)(VM* vm, PyVar));
    void bind__len__(Type type, i64 (*f)(VM* vm, PyVar));

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

    void bind__getitem__(Type type, PyVar (*f)(VM*, PyVar, PyVar));
    void bind__setitem__(Type type, void (*f)(VM*, PyVar, PyVar, PyVar));
    void bind__delitem__(Type type, void (*f)(VM*, PyVar, PyVar));
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
    [[noreturn]] void _error(PyVar);
    [[noreturn]] void StackOverflowError() { __builtin_error("StackOverflowError"); }
    [[noreturn]] void IOError(const Str& msg) { __builtin_error("IOError", msg); }
    [[noreturn]] void NotImplementedError(){ __builtin_error("NotImplementedError"); }
    [[noreturn]] void TypeError(const Str& msg){ __builtin_error("TypeError", msg); }
    [[noreturn]] void TypeError(Type expected, Type actual) { TypeError("expected " + _type_name(vm, expected).escape() + ", got " + _type_name(vm, actual).escape()); }
    [[noreturn]] void IndexError(const Str& msg){ __builtin_error("IndexError", msg); }
    [[noreturn]] void ValueError(const Str& msg){ __builtin_error("ValueError", msg); }
    [[noreturn]] void RuntimeError(const Str& msg){ __builtin_error("RuntimeError", msg); }
    [[noreturn]] void ZeroDivisionError(const Str& msg){ __builtin_error("ZeroDivisionError", msg); }
    [[noreturn]] void ZeroDivisionError(){ __builtin_error("ZeroDivisionError", "division by zero"); }
    [[noreturn]] void NameError(StrName name){ __builtin_error("NameError", _S("name ", name.escape() + " is not defined")); }
    [[noreturn]] void UnboundLocalError(StrName name){ __builtin_error("UnboundLocalError", _S("local variable ", name.escape() + " referenced before assignment")); }
    [[noreturn]] void KeyError(PyVar obj){ __builtin_error("KeyError", obj); }
    [[noreturn]] void ImportError(const Str& msg){ __builtin_error("ImportError", msg); }
    [[noreturn]] void AssertionError(const Str& msg){ __builtin_error("AssertionError", msg); }
    [[noreturn]] void AssertionError(){ __builtin_error("AssertionError"); }
    [[noreturn]] void BinaryOptError(const char* op, PyVar _0, PyVar _1);
    [[noreturn]] void AttributeError(PyVar obj, StrName name);
    [[noreturn]] void AttributeError(const Str& msg){ __builtin_error("AttributeError", msg); }
#endif

#if PK_REGION("Type Checking Methods")
    bool isinstance(PyVar obj, Type base);
    bool issubclass(Type cls, Type base);
    void check_type(PyVar obj, Type type){ if(!is_type(obj, type)) TypeError(type, _tp(obj)); }
    void check_compatible_type(PyVar obj, Type type){ if(!isinstance(obj, type)) TypeError(type, _tp(obj)); }

    Type _tp(PyVar obj){ return obj.type; }
    const PyTypeInfo* _tp_info(PyVar obj) { return &_all_types[_tp(obj)]; }
    const PyTypeInfo* _tp_info(Type type) { return &_all_types[type]; }
    PyObject* _t(PyVar obj){ return _all_types[_tp(obj)].obj; }
    PyObject* _t(Type type){ return _all_types[type].obj; }
#endif

#if PK_REGION("User Type Registration")
    PyObject* new_module(Str name, Str package="");
    PyObject* new_type_object(PyObject* mod, StrName name, Type base, bool subclass_enabled, PyTypeInfo::Vt vt={});

    template<typename T>
    PyObject* new_type_object(PyObject* mod, StrName name, Type base, bool subclass_enabled){
        return new_type_object(mod, name, base, subclass_enabled, PyTypeInfo::Vt::get<T>());
    }

    template<typename T>
    Type _tp_user(){ return _find_type_in_cxx_typeid_map<T>(); }
    template<typename T>
    bool is_user_type(PyVar obj){ return _tp(obj) == _tp_user<T>(); }

    template<typename T>
    PyObject* register_user_class(PyObject*, StrName, RegisterFunc, Type base=tp_object, bool subclass_enabled=false);
    template<typename T>
    PyObject* register_user_class(PyObject*, StrName, Type base=tp_object, bool subclass_enabled=false);

    template<typename T, typename ...Args>
    PyVar new_user_object(Args&&... args){
        return new_object<T>(_tp_user<T>(), std::forward<Args>(args)...);
    }

    template<typename T, typename ...Args>
    PyVar new_object(Type type, Args&&... args){
        if constexpr(is_sso_v<T>) return PyVar(type, T(std::forward<Args>(args)...));
        else return heap.gcnew<T>(type, std::forward<Args>(args)...);
    }
#endif

    template <typename T>
    Type _find_type_in_cxx_typeid_map() {
        auto it = _cxx_typeid_map.try_get(typeid(T));
        if(it == nullptr) PK_FATAL_ERROR("T not found in cxx_typeid_map\n")
        return *it;
    }

    /********** private **********/
    virtual ~VM();

#if PK_DEBUG_CEVAL_STEP
    void __log_s_data(const char* title = nullptr);
#endif
    PyVar __py_exec_internal(const CodeObject_& code, PyVar globals, PyVar locals);
    void __breakpoint();
    PyVar __format_object(PyVar, Str);
    PyVar __run_top_frame();
    void __pop_frame();
    PyVar __py_generator(LinkedFrame* frame, ArgsView buffer);
    void __op_unpack_sequence(uint16_t arg);
    void __prepare_py_call(PyVar*, ArgsView, ArgsView, const FuncDecl_&);
    void __unpack_as_list(ArgsView args, List& list);
    void __unpack_as_dict(ArgsView args, Dict& dict);
    [[noreturn]] void __raise_exc(bool re_raise = false);
    [[noreturn]] void __builtin_error(StrName type);
    [[noreturn]] void __builtin_error(StrName type, PyVar arg);
    [[noreturn]] void __builtin_error(StrName type, const Str& msg);
    [[noreturn]] void __compile_error(Error* err);
    void __init_builtin_types();
    void __post_init_builtin_types();
    void __push_varargs() {}
    void __push_varargs(PyVar _0) { PUSH(_0); }
    void __push_varargs(PyVar _0, PyVar _1) {
        PUSH(_0);
        PUSH(_1);
    }
    void __push_varargs(PyVar _0, PyVar _1, PyVar _2) {
        PUSH(_0);
        PUSH(_1);
        PUSH(_2);
    }
    void __push_varargs(PyVar _0, PyVar _1, PyVar _2, PyVar _3) {
        PUSH(_0);
        PUSH(_1);
        PUSH(_2);
        PUSH(_3);
    }
    PyVar __pack_next_retval(unsigned);
    PyVar __minmax_reduce(bool (VM::*op)(PyVar, PyVar), PyVar args, PyVar key);
    bool __py_bool_non_trivial(PyVar);
    void __obj_gc_mark(PyObject*);
    void __stack_gc_mark(PyVar* begin, PyVar* end);
    void* __stack_alloc(int size);
};

template <typename T>
constexpr inline bool is_immutable_v =
    is_integral_v<T> || is_floating_point_v<T> || std::is_same_v<T, Str> || std::is_same_v<T, Tuple> ||
    std::is_same_v<T, Bytes> || std::is_same_v<T, bool> || std::is_same_v<T, Range> || std::is_same_v<T, Slice> ||
    std::is_pointer_v<T> || std::is_enum_v<T>;

template<typename T> constexpr Type _tp_builtin() { return Type(); }
template<> constexpr Type _tp_builtin<Str>() { return VM::tp_str; }
template<> constexpr Type _tp_builtin<List>() { return VM::tp_list; }
template<> constexpr Type _tp_builtin<Tuple>() { return VM::tp_tuple; }
template<> constexpr Type _tp_builtin<Function>() { return VM::tp_function; }
template<> constexpr Type _tp_builtin<NativeFunc>() { return VM::tp_native_func; }
template<> constexpr Type _tp_builtin<BoundMethod>() { return VM::tp_bound_method; }
template<> constexpr Type _tp_builtin<Range>() { return VM::tp_range; }
template<> constexpr Type _tp_builtin<Slice>() { return VM::tp_slice; }
template<> constexpr Type _tp_builtin<Exception>() { return VM::tp_exception; }
template<> constexpr Type _tp_builtin<Bytes>() { return VM::tp_bytes; }
template<> constexpr Type _tp_builtin<MappingProxy>() { return VM::tp_mappingproxy; }
template<> constexpr Type _tp_builtin<Dict>() { return VM::tp_dict; }
template<> constexpr Type _tp_builtin<Property>() { return VM::tp_property; }
template<> constexpr Type _tp_builtin<StarWrapper>() { return VM::tp_star_wrapper; }
template<> constexpr Type _tp_builtin<StaticMethod>() { return VM::tp_staticmethod; }
template<> constexpr Type _tp_builtin<ClassMethod>() { return VM::tp_classmethod; }

// clang-format on

template <typename __T>
PyVar py_var(VM* vm, __T&& value) {
    using T = std::decay_t<__T>;

    static_assert(!std::is_same_v<T, PyVar>, "py_var(VM*, PyVar) is not allowed");

    if constexpr(std::is_same_v<T, const char*> || std::is_same_v<T, std::string> ||
                 std::is_same_v<T, std::string_view>) {
        // str (shortcuts)
        return VAR(Str(std::forward<__T>(value)));
    } else if constexpr(std::is_same_v<T, NoReturn>) {
        // NoneType
        return vm->None;
    } else if constexpr(std::is_same_v<T, bool>) {
        // bool
        return value ? vm->True : vm->False;
    } else if constexpr(is_integral_v<T>) {
        // int
        return PyVar(VM::tp_int, static_cast<i64>(value));
    } else if constexpr(is_floating_point_v<T>) {
        // float
        return PyVar(VM::tp_float, static_cast<f64>(value));
    } else if constexpr(std::is_pointer_v<T>) {
        return from_void_p(vm, (void*)value);
    } else {
        constexpr Type const_type = _tp_builtin<T>();
        if constexpr((bool)const_type) {
            if constexpr(is_sso_v<T>)
                return PyVar(const_type, value);
            else
                return vm->heap.gcnew<T>(const_type, std::forward<__T>(value));
        } else {
            Type type = vm->_find_type_in_cxx_typeid_map<T>();
            if constexpr(is_sso_v<T>)
                return PyVar(type, value);
            else
                return vm->heap.gcnew<T>(type, std::forward<__T>(value));
        }
    }
}

// fast path for bool if py_var<> cannot be inlined
inline PyVar py_var(VM* vm, bool value) { return value ? vm->True : vm->False; }

template <typename __T, bool with_check>
__T _py_cast__internal(VM* vm, PyVar obj) {
    static_assert(!std::is_rvalue_reference_v<__T>, "rvalue reference is not allowed");
    using T = std::decay_t<__T>;
    static_assert(!(is_sso_v<T> && std::is_reference_v<__T>), "SSO types cannot be reference");

    if constexpr(std::is_same_v<T, const char*> || std::is_same_v<T, CString>) {
        static_assert(!std::is_reference_v<__T>);
        // str (shortcuts)
        if(is_none(obj)) return nullptr;
        if constexpr(with_check) vm->check_type(obj, vm->tp_str);
        return PK_OBJ_GET(Str, obj).c_str();
    } else if constexpr(std::is_same_v<T, bool>) {
        static_assert(!std::is_reference_v<__T>);
        // bool
        if constexpr(with_check) {
            if(obj == vm->True) return true;
            if(obj == vm->False) return false;
            vm->TypeError("expected 'bool', got " + _type_name(vm, vm->_tp(obj)).escape());
        }
        return obj == vm->True;
    } else if constexpr(is_integral_v<T>) {
        static_assert(!std::is_reference_v<__T>);
        // int
        if constexpr(with_check) {
            if(is_int(obj)) return (T)obj.as<i64>();
            vm->TypeError("expected 'int', got " + _type_name(vm, vm->_tp(obj)).escape());
        }
        return (T)obj.as<i64>();
    } else if constexpr(is_floating_point_v<T>) {
        static_assert(!std::is_reference_v<__T>);
        if(is_float(obj)) return (T)obj.as<f64>();
        if(is_int(obj)) return (T)obj.as<i64>();
        vm->TypeError("expected 'int' or 'float', got " + _type_name(vm, vm->_tp(obj)).escape());
        return 0.0f;
    } else if constexpr(std::is_enum_v<T>) {
        static_assert(!std::is_reference_v<__T>);
        return (__T)_py_cast__internal<i64, with_check>(vm, obj);
    } else if constexpr(std::is_pointer_v<T>) {
        static_assert(!std::is_reference_v<__T>);
        return to_void_p<T>(vm, obj);
    } else {
        constexpr Type const_type = _tp_builtin<T>();
        if constexpr((bool)const_type) {
            if constexpr(with_check) {
                if constexpr(std::is_same_v<T, Exception>) {
                    // Exception is `subclass_enabled`
                    vm->check_compatible_type(obj, const_type);
                } else {
                    vm->check_type(obj, const_type);
                }
            }
            return PK_OBJ_GET(T, obj);
        } else {
            if constexpr(with_check) {
                Type type = vm->_find_type_in_cxx_typeid_map<T>();
                vm->check_compatible_type(obj, type);
            }
            return PK_OBJ_GET(T, obj);
        }
    }
}

template <typename __T>
__T py_cast(VM* vm, PyVar obj) {
    return _py_cast__internal<__T, true>(vm, obj);
}

template <typename __T>
__T _py_cast(VM* vm, PyVar obj) {
    return _py_cast__internal<__T, false>(vm, obj);
}

template <typename T>
PyObject*
    VM::register_user_class(PyObject* mod, StrName name, RegisterFunc _register, Type base, bool subclass_enabled) {
    PyObject* type = new_type_object(mod, name, base, subclass_enabled, PyTypeInfo::Vt::get<T>());
    mod->attr().set(name, type);
    _cxx_typeid_map.insert(typeid(T), type->as<Type>());
    _register(this, mod, type);
    if(!type->attr().contains(__new__)) {
        if constexpr(std::is_default_constructible_v<T>) {
            bind_func(type, __new__, -1, [](VM* vm, ArgsView args) {
                Type cls_t = args[0]->as<Type>();
                return vm->new_object<T>(cls_t);
            });
        } else {
            bind_func(type, __new__, -1, [](VM* vm, ArgsView args) {
                vm->NotImplementedError();
                return vm->None;
            });
        }
    }
    return type;
}

template <typename T>
PyObject* VM::register_user_class(PyObject* mod, StrName name, Type base, bool subclass_enabled) {
    return register_user_class<T>(mod, name, &T::_register, base, subclass_enabled);
}

}  // namespace pkpy
