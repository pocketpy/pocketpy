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
#define PEEK(n)           (s_data.peek(n))
#define STACK_SHRINK(n)   (s_data.shrink(n))
#define PUSH(v)           (s_data.push(v))
#define POP()             (s_data.pop())
#define POPX()            (s_data.popx())
#define STACK_VIEW(n)     (s_data.view(n))

typedef PyObject* (*BinaryFuncC)(VM*, PyObject*, PyObject*);

struct NextBreakpoint{
    int callstack_size;
    int lineno;
    bool should_step_into;
    NextBreakpoint(): callstack_size(0) {}
    NextBreakpoint(int callstack_size, int lineno, bool should_step_into): callstack_size(callstack_size), lineno(lineno), should_step_into(should_step_into) {}
    void _step(VM* vm);
    bool empty() const { return callstack_size == 0; }
};

struct PyTypeInfo{
    PyObject* obj;      // never be garbage collected
    Type base;
    PyObject* mod;      // never be garbage collected
    StrName name;
    bool subclass_enabled;

    pod_vector<StrName> annotated_fields = {};

    // cached special methods
    // unary operators
    PyObject* (*m__repr__)(VM* vm, PyObject*) = nullptr;
    PyObject* (*m__str__)(VM* vm, PyObject*) = nullptr;
    i64 (*m__hash__)(VM* vm, PyObject*) = nullptr;
    i64 (*m__len__)(VM* vm, PyObject*) = nullptr;
    PyObject* (*m__iter__)(VM* vm, PyObject*) = nullptr;
    PyObject* (*m__next__)(VM* vm, PyObject*) = nullptr;
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

typedef void(*PrintFunc)(const char*, int);

class VM {
    PK_ALWAYS_PASS_BY_POINTER(VM)
    
    VM* vm;     // self reference for simplify code
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

    // this is for repr() recursion detection (no need to mark)
    std::set<PyObject*> _repr_recursion_set;

    // cached code objects for FSTRING_EVAL
    std::map<std::string_view, CodeObject_> _cached_codes;

    // typeid -> Type
    std::map<const std::type_index, Type> _cxx_typeid_map;

    void (*_ceval_on_step)(VM*, Frame*, Bytecode bc) = nullptr;

    LineProfiler* _profiler = nullptr;
    NextBreakpoint _next_breakpoint;

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
    static constexpr Type tp_staticmethod=21, tp_classmethod=22;

    PyObject* cached_object__new__;

    const bool enable_os;

    VM(bool enable_os=true);

    void set_main_argv(int argc, char** argv);
    void _breakpoint();

    Frame* top_frame(){
        return &callstack.top();
    }

    void _pop_frame(){
        s_data.reset(callstack.top()._sp_base);
        callstack.pop();
        
        if(!_next_breakpoint.empty() && callstack.size()<_next_breakpoint.callstack_size){
            _next_breakpoint = NextBreakpoint();
        }
    }

    PyObject* py_str(PyObject* obj);
    PyObject* py_repr(PyObject* obj);
    PyObject* py_json(PyObject* obj);
    PyObject* py_iter(PyObject* obj);

    std::pair<PyObject**, int> _cast_array(PyObject* obj);

    PyObject* find_name_in_mro(Type cls, StrName name);
    bool isinstance(PyObject* obj, Type base);
    bool issubclass(Type cls, Type base);

    CodeObject_ compile(std::string_view source, const Str& filename, CompileMode mode, bool unknown_global_scope=false);
    Str precompile(std::string_view source, const Str& filename, CompileMode mode);

    PyObject* exec(std::string_view source, Str filename, CompileMode mode, PyObject* _module=nullptr);
    PyObject* exec(std::string_view source);
    PyObject* eval(std::string_view source);

    template<typename ...Args>
    PyObject* _exec(Args&&... args){
        callstack.emplace(s_data._sp, std::forward<Args>(args)...);
        return _run_top_frame();
    }

    void _push_varargs(){}
    void _push_varargs(PyObject* _0){ PUSH(_0); }
    void _push_varargs(PyObject* _0, PyObject* _1){ PUSH(_0); PUSH(_1); }
    void _push_varargs(PyObject* _0, PyObject* _1, PyObject* _2){ PUSH(_0); PUSH(_1); PUSH(_2); }
    void _push_varargs(PyObject* _0, PyObject* _1, PyObject* _2, PyObject* _3){ PUSH(_0); PUSH(_1); PUSH(_2); PUSH(_3); }

    virtual void stdout_write(const Str& s){
        _stdout(s.data, s.size);
    }

    virtual void stderr_write(const Str& s){
        _stderr(s.data, s.size);
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
    Type _new_type_object(StrName name, Type base=0, bool subclass_enabled=false);
    const PyTypeInfo* _inst_type_info(PyObject* obj);

    void bind__repr__(Type type, PyObject* (*f)(VM*, PyObject*));
    void bind__str__(Type type, PyObject* (*f)(VM*, PyObject*));
    void bind__iter__(Type type, PyObject* (*f)(VM*, PyObject*));
    void bind__next__(Type type, PyObject* (*f)(VM*, PyObject*));
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

    bool py_eq(PyObject* lhs, PyObject* rhs);
    // new in v1.2.9
    bool py_lt(PyObject* lhs, PyObject* rhs);
    bool py_le(PyObject* lhs, PyObject* rhs);
    bool py_gt(PyObject* lhs, PyObject* rhs);
    bool py_ge(PyObject* lhs, PyObject* rhs);
    bool py_ne(PyObject* lhs, PyObject* rhs) { return !py_eq(lhs, rhs); }

    template<int ARGC, typename __T>
    PyObject* bind_constructor(__T&& type, NativeFuncC fn) {
        static_assert(ARGC==-1 || ARGC>=1);
        return bind_func<ARGC>(std::forward<__T>(type), __new__, fn);
    }

    template<typename T, typename __T>
    PyObject* bind_notimplemented_constructor(__T&& type) {
        return bind_func<-1>(std::forward<__T>(type), __new__, [](VM* vm, ArgsView args){
            vm->NotImplementedError();
            return vm->None;
        });
    }

    i64 normalized_index(i64 index, int size);
    PyObject* py_next(PyObject* obj);
    bool py_callable(PyObject* obj);
    
    /***** Error Reporter *****/
    void _raise(bool re_raise=false);

    void _builtin_error(StrName type);
    void _builtin_error(StrName type, PyObject* arg);
    void _builtin_error(StrName type, const Str& msg);

    void StackOverflowError() { _builtin_error("StackOverflowError"); }
    void IOError(const Str& msg) { _builtin_error("IOError", msg); }
    void NotImplementedError(){ _builtin_error("NotImplementedError"); }
    void TypeError(const Str& msg){ _builtin_error("TypeError", msg); }
    void IndexError(const Str& msg){ _builtin_error("IndexError", msg); }
    void ValueError(const Str& msg){ _builtin_error("ValueError", msg); }
    void RuntimeError(const Str& msg){ _builtin_error("RuntimeError", msg); }
    void ZeroDivisionError(const Str& msg){ _builtin_error("ZeroDivisionError", msg); }
    void ZeroDivisionError(){ _builtin_error("ZeroDivisionError", "division by zero"); }
    void NameError(StrName name){ _builtin_error("NameError", _S("name ", name.escape() + " is not defined")); }
    void UnboundLocalError(StrName name){ _builtin_error("UnboundLocalError", _S("local variable ", name.escape() + " referenced before assignment")); }
    void KeyError(PyObject* obj){ _builtin_error("KeyError", obj); }
    void ImportError(const Str& msg){ _builtin_error("ImportError", msg); }

    void BinaryOptError(const char* op, PyObject* _0, PyObject* _1);
    void AttributeError(PyObject* obj, StrName name);
    void AttributeError(const Str& msg){ _builtin_error("AttributeError", msg); }

    void check_type(PyObject* obj, Type type){
        if(is_type(obj, type)) return;
        TypeError("expected " + _type_name(vm, type).escape() + ", got " + _type_name(vm, _tp(obj)).escape());
    }

    [[deprecated("use check_type() instead")]]
    void check_non_tagged_type(PyObject* obj, Type type){
        return check_type(obj, type);
    }

    void check_compatible_type(PyObject* obj, Type type){
        if(isinstance(obj, type)) return;
        TypeError("expected " + _type_name(vm, type).escape() + ", got " + _type_name(vm, _tp(obj)).escape());
    }

    PyObject* _t(Type t){
        return _all_types[t.index].obj;
    }

    Type _tp(PyObject* obj){
        if(!is_tagged(obj)) return obj->type;
        return tp_int;
    }

    PyObject* _t(PyObject* obj){
        return _all_types[_tp(obj).index].obj;
    }

    struct ImportContext{
        std::vector<Str> pending;
        pod_vector<bool> pending_is_init;   // a.k.a __init__.py

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

    ImportContext _import_context;
    PyObject* py_import(Str path, bool throw_err=true);
    virtual ~VM();

#if PK_DEBUG_CEVAL_STEP
    void _log_s_data(const char* title = nullptr);
#endif
    void _unpack_as_list(ArgsView args, List& list);
    void _unpack_as_dict(ArgsView args, Dict& dict);
    PyObject* vectorcall(int ARGC, int KWARGC=0, bool op_call=false);
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
    PyObject* bind_method(Type, StrName, NativeFuncC);
    template<int ARGC>
    PyObject* bind_method(PyObject*, StrName, NativeFuncC);
    template<int ARGC>
    PyObject* bind_func(PyObject*, StrName, NativeFuncC, UserData userdata={}, BindType bt=BindType::DEFAULT);
    void _error(PyObject*);
    PyObject* _run_top_frame();
    void post_init();
    PyObject* _py_generator(Frame&& frame, ArgsView buffer);
    void _prepare_py_call(PyObject**, ArgsView, ArgsView, const FuncDecl_&);
    // new style binding api
    PyObject* bind(PyObject*, const char*, const char*, NativeFuncC, UserData userdata={}, BindType bt=BindType::DEFAULT);
    PyObject* bind(PyObject*, const char*, NativeFuncC, UserData userdata={}, BindType bt=BindType::DEFAULT);
    PyObject* bind_property(PyObject*, const char*, NativeFuncC fget, NativeFuncC fset=nullptr);

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
};


template<typename T>
inline constexpr bool is_immutable_v = is_integral_v<T> || is_floating_point_v<T>
    || std::is_same_v<T, Str> || std::is_same_v<T, Tuple> || std::is_same_v<T, Bytes> || std::is_same_v<T, bool>
    || std::is_same_v<T, Range> || std::is_same_v<T, Slice>
    || std::is_pointer_v<T> || std::is_enum_v<T>;

template<typename T> constexpr Type _find_type_in_const_cxx_typeid_map(){ return -1; }
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


template<int ARGC>
PyObject* VM::bind_method(Type type, StrName name, NativeFuncC fn) {
    PyObject* nf = VAR(NativeFunc(fn, ARGC, true));
    _t(type)->attr().set(name, nf);
    return nf;
}

template<int ARGC>
PyObject* VM::bind_method(PyObject* obj, StrName name, NativeFuncC fn) {
    check_type(obj, tp_type);
    return bind_method<ARGC>(PK_OBJ_GET(Type, obj), name, fn);
}

template<int ARGC>
PyObject* VM::bind_func(PyObject* obj, StrName name, NativeFuncC fn, UserData userdata, BindType bt) {
    PyObject* nf = VAR(NativeFunc(fn, ARGC, false));
    PK_OBJ_GET(NativeFunc, nf).set_userdata(userdata);
    switch(bt){
        case BindType::DEFAULT: break;
        case BindType::STATICMETHOD: nf = VAR(StaticMethod(nf)); break;
        case BindType::CLASSMETHOD: nf = VAR(ClassMethod(nf)); break;
    }
    obj->attr().set(name, nf);
    return nf;
}

}   // namespace pkpy