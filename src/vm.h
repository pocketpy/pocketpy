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

typedef Bytes (*ReadFileCwdFunc)(const Str& name);
inline ReadFileCwdFunc _read_file_cwd = [](const Str& name) { return Bytes(); };
inline int set_read_file_cwd(ReadFileCwdFunc func) { _read_file_cwd = func; return 0; }

#define DEF_NATIVE_2(ctype, ptype)                                      \
    template<> inline ctype py_cast<ctype>(VM* vm, PyObject* obj) {     \
        vm->check_non_tagged_type(obj, vm->ptype);                      \
        return OBJ_GET(ctype, obj);                                     \
    }                                                                   \
    template<> inline ctype _py_cast<ctype>(VM* vm, PyObject* obj) {    \
        return OBJ_GET(ctype, obj);                                     \
    }                                                                   \
    template<> inline ctype& py_cast<ctype&>(VM* vm, PyObject* obj) {   \
        vm->check_non_tagged_type(obj, vm->ptype);                      \
        return OBJ_GET(ctype, obj);                                     \
    }                                                                   \
    template<> inline ctype& _py_cast<ctype&>(VM* vm, PyObject* obj) {  \
        return OBJ_GET(ctype, obj);                                     \
    }                                                                   \
    inline PyObject* py_var(VM* vm, const ctype& value) { return vm->heap.gcnew(vm->ptype, value);}     \
    inline PyObject* py_var(VM* vm, ctype&& value) { return vm->heap.gcnew(vm->ptype, std::move(value));}


struct PyTypeInfo{
    PyObject* obj;
    Type base;
    Str name;
    bool subclass_enabled;

    // cached special methods
    // unary operators
    PyObject* (*m__repr__)(VM* vm, PyObject*) = nullptr;
    PyObject* (*m__str__)(VM* vm, PyObject*) = nullptr;
    i64 (*m__hash__)(VM* vm, PyObject*) = nullptr;
    i64 (*m__len__)(VM* vm, PyObject*) = nullptr;
    PyObject* (*m__iter__)(VM* vm, PyObject*) = nullptr;
    PyObject* (*m__next__)(VM* vm, PyObject*) = nullptr;
    PyObject* (*m__json__)(VM* vm, PyObject*) = nullptr;
    PyObject* (*m__neg__)(VM* vm, PyObject*) = nullptr;
    PyObject* (*m__bool__)(VM* vm, PyObject*) = nullptr;

    bool (*m__eq__)(VM* vm, PyObject*, PyObject*) = nullptr;
    bool (*m__lt__)(VM* vm, PyObject*, PyObject*) = nullptr;
    bool (*m__le__)(VM* vm, PyObject*, PyObject*) = nullptr;
    bool (*m__gt__)(VM* vm, PyObject*, PyObject*) = nullptr;
    bool (*m__ge__)(VM* vm, PyObject*, PyObject*) = nullptr;
    bool (*m__contains__)(VM* vm, PyObject*, PyObject*) = nullptr;

    // binary operators
    PyObject* (*m__add__)(VM* vm, PyObject*, PyObject*) = nullptr;
    PyObject* (*m__sub__)(VM* vm, PyObject*, PyObject*) = nullptr;
    PyObject* (*m__mul__)(VM* vm, PyObject*, PyObject*) = nullptr;
    PyObject* (*m__truediv__)(VM* vm, PyObject*, PyObject*) = nullptr;
    PyObject* (*m__floordiv__)(VM* vm, PyObject*, PyObject*) = nullptr;
    PyObject* (*m__mod__)(VM* vm, PyObject*, PyObject*) = nullptr;
    PyObject* (*m__pow__)(VM* vm, PyObject*, PyObject*) = nullptr;
    PyObject* (*m__matmul__)(VM* vm, PyObject*, PyObject*) = nullptr;

    PyObject* (*m__lshift__)(VM* vm, PyObject*, PyObject*) = nullptr;
    PyObject* (*m__rshift__)(VM* vm, PyObject*, PyObject*) = nullptr;
    PyObject* (*m__and__)(VM* vm, PyObject*, PyObject*) = nullptr;
    PyObject* (*m__or__)(VM* vm, PyObject*, PyObject*) = nullptr;
    PyObject* (*m__xor__)(VM* vm, PyObject*, PyObject*) = nullptr;

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
};

typedef void(*PrintFunc)(VM*, const Str&);

class VM {
    VM* vm;     // self reference for simplify code
public:
    ManagedHeap heap;
    ValueStack s_data;
    stack< Frame > callstack;
    std::vector<PyTypeInfo> _all_types;
    
    NameDict _modules;                                 // loaded modules
    std::map<StrName, Str> _lazy_modules;              // lazy loaded modules

    PyObject* None;
    PyObject* True;
    PyObject* False;
    PyObject* Ellipsis;
    PyObject* builtins;         // builtins module
    PyObject* StopIteration;
    PyObject* _main;            // __main__ module

    PyObject* _last_exception;

    PrintFunc _stdout;
    PrintFunc _stderr;

    // for quick access
    Type tp_object, tp_type, tp_int, tp_float, tp_bool, tp_str;
    Type tp_list, tp_tuple;
    Type tp_function, tp_native_func, tp_bound_method;
    Type tp_slice, tp_range, tp_module;
    Type tp_super, tp_exception, tp_bytes, tp_mappingproxy;
    Type tp_dict, tp_property, tp_star_wrapper;

    const bool enable_os;

    VM(bool enable_os=true) : heap(this), enable_os(enable_os) {
        this->vm = this;
        _stdout = [](VM* vm, const Str& s) { std::cout << s; };
        _stderr = [](VM* vm, const Str& s) { std::cerr << s; };
        callstack.reserve(8);
        _main = nullptr;
        _last_exception = nullptr;
        init_builtin_types();
    }

    FrameId top_frame() {
#if DEBUG_EXTRA_CHECK
        if(callstack.empty()) FATAL_ERROR();
#endif
        return FrameId(&callstack.data(), callstack.size()-1);
    }

    PyObject* py_str(PyObject* obj){
        const PyTypeInfo* ti = _inst_type_info(obj);
        if(ti->m__str__) return ti->m__str__(this, obj);
        PyObject* self;
        PyObject* f = get_unbound_method(obj, __str__, &self, false);
        if(self != PY_NULL) return call_method(self, f);
        return py_repr(obj);
    }

    PyObject* py_repr(PyObject* obj){
        const PyTypeInfo* ti = _inst_type_info(obj);
        if(ti->m__repr__) return ti->m__repr__(this, obj);
        return call_method(obj, __repr__);
    }

    PyObject* py_json(PyObject* obj){
        const PyTypeInfo* ti = _inst_type_info(obj);
        if(ti->m__json__) return ti->m__json__(this, obj);
        return call_method(obj, __json__);
    }

    PyObject* py_iter(PyObject* obj){
        const PyTypeInfo* ti = _inst_type_info(obj);
        if(ti->m__iter__) return ti->m__iter__(this, obj);
        PyObject* self;
        PyObject* iter_f = get_unbound_method(obj, __iter__, &self, false);
        if(self != PY_NULL) return call_method(self, iter_f);
        TypeError(OBJ_NAME(_t(obj)).escape() + " object is not iterable");
        return nullptr;
    }

    PyObject* find_name_in_mro(PyObject* cls, StrName name){
        PyObject* val;
        do{
            val = cls->attr().try_get(name);
            if(val != nullptr) return val;
            Type base = _all_types[OBJ_GET(Type, cls)].base;
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

    PyObject* exec(Str source, Str filename, CompileMode mode, PyObject* _module=nullptr){
        if(_module == nullptr) _module = _main;
        try {
            CodeObject_ code = compile(source, filename, mode);
#if DEBUG_DIS_EXEC
            if(_module == _main) std::cout << disassemble(code) << '\n';
#endif
            return _exec(code, _module);
        }catch (const Exception& e){
            _stderr(this, e.summary() + "\n");
        }
#if !DEBUG_FULL_EXCEPTION
        catch (const std::exception& e) {
            Str msg = "An std::exception occurred! It could be a bug.\n";
            msg = msg + e.what();
            _stderr(this, msg + "\n");
        }
#endif
        callstack.clear();
        s_data.clear();
        return nullptr;
    }

    template<typename ...Args>
    PyObject* _exec(Args&&... args){
        callstack.emplace(&s_data, s_data._sp, std::forward<Args>(args)...);
        return _run_top_frame();
    }

    void _pop_frame(){
        Frame* frame = &callstack.top();
        s_data.reset(frame->_sp_base);
        callstack.pop();
    }

    void _push_varargs(){ }
    void _push_varargs(PyObject* _0){ PUSH(_0); }
    void _push_varargs(PyObject* _0, PyObject* _1){ PUSH(_0); PUSH(_1); }
    void _push_varargs(PyObject* _0, PyObject* _1, PyObject* _2){ PUSH(_0); PUSH(_1); PUSH(_2); }
    void _push_varargs(PyObject* _0, PyObject* _1, PyObject* _2, PyObject* _3){ PUSH(_0); PUSH(_1); PUSH(_2); PUSH(_3); }

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

    PyObject* property(NativeFuncC fget, NativeFuncC fset=nullptr){
        PyObject* _0 = heap.gcnew(tp_native_func, NativeFunc(fget, 1, false));
        PyObject* _1 = vm->None;
        if(fset != nullptr) _1 = heap.gcnew(tp_native_func, NativeFunc(fset, 2, false));
        return call(_t(tp_property), _0, _1);
    }

    PyObject* new_type_object(PyObject* mod, StrName name, Type base, bool subclass_enabled=true){
        PyObject* obj = heap._new<Type>(tp_type, _all_types.size());
        const PyTypeInfo& base_info = _all_types[base];
        if(!base_info.subclass_enabled){
            TypeError(fmt("type ", base_info.name.escape(), " is not `subclass_enabled`"));
        }
        PyTypeInfo info{
            obj,
            base,
            (mod!=nullptr && mod!=builtins) ? Str(OBJ_NAME(mod)+"."+name.sv()): name.sv(),
            subclass_enabled,
        };
        if(mod != nullptr) mod->attr().set(name, obj);
        _all_types.push_back(info);
        return obj;
    }

    Type _new_type_object(StrName name, Type base=0) {
        PyObject* obj = new_type_object(nullptr, name, base, false);
        return OBJ_GET(Type, obj);
    }

    PyObject* _find_type_object(const Str& type){
        PyObject* obj = builtins->attr().try_get(type);
        if(obj == nullptr){
            for(auto& t: _all_types) if(t.name == type) return t.obj;
            throw std::runtime_error(fmt("type not found: ", type));
        }
        check_non_tagged_type(obj, tp_type);
        return obj;
    }

    Type _type(const Str& type){
        PyObject* obj = _find_type_object(type);
        return OBJ_GET(Type, obj);
    }

    PyTypeInfo* _type_info(const Str& type){
        PyObject* obj = builtins->attr().try_get(type);
        if(obj == nullptr){
            for(auto& t: _all_types) if(t.name == type) return &t;
            FATAL_ERROR();
        }
        return &_all_types[OBJ_GET(Type, obj)];
    }

    PyTypeInfo* _type_info(Type type){
        return &_all_types[type];
    }

    const PyTypeInfo* _inst_type_info(PyObject* obj){
        if(is_int(obj)) return &_all_types[tp_int];
        if(is_float(obj)) return &_all_types[tp_float];
        return &_all_types[obj->type];
    }

#define BIND_UNARY_SPECIAL(name)                                                        \
    void bind##name(Type type, PyObject* (*f)(VM*, PyObject*)){                         \
        _all_types[type].m##name = f;                                                   \
        PyObject* nf = bind_method<0>(_t(type), #name, [](VM* vm, ArgsView args){       \
            return lambda_get_userdata<PyObject*(*)(VM*, PyObject*)>(args.begin())(vm, args[0]);\
        });                                                                             \
        OBJ_GET(NativeFunc, nf).set_userdata(f);                                        \
    }

    BIND_UNARY_SPECIAL(__repr__)
    BIND_UNARY_SPECIAL(__str__)
    BIND_UNARY_SPECIAL(__iter__)
    BIND_UNARY_SPECIAL(__next__)
    BIND_UNARY_SPECIAL(__json__)
    BIND_UNARY_SPECIAL(__neg__)
    BIND_UNARY_SPECIAL(__bool__)

    void bind__hash__(Type type, i64 (*f)(VM* vm, PyObject*));
    void bind__len__(Type type, i64 (*f)(VM* vm, PyObject*));
#undef BIND_UNARY_SPECIAL


#define BIND_LOGICAL_SPECIAL(name)                                                      \
    void bind##name(Type type, bool (*f)(VM*, PyObject*, PyObject*)){                   \
        PyObject* obj = _t(type);                                                       \
        _all_types[type].m##name = f;                                                   \
        PyObject* nf = bind_method<1>(obj, #name, [](VM* vm, ArgsView args){            \
            bool ok = lambda_get_userdata<bool(*)(VM*, PyObject*, PyObject*)>(args.begin())(vm, args[0], args[1]); \
            return ok ? vm->True : vm->False;                                           \
        });                                                                             \
        OBJ_GET(NativeFunc, nf).set_userdata(f);                                        \
    }

    BIND_LOGICAL_SPECIAL(__eq__)
    BIND_LOGICAL_SPECIAL(__lt__)
    BIND_LOGICAL_SPECIAL(__le__)
    BIND_LOGICAL_SPECIAL(__gt__)
    BIND_LOGICAL_SPECIAL(__ge__)
    BIND_LOGICAL_SPECIAL(__contains__)

#undef BIND_LOGICAL_SPECIAL


#define BIND_BINARY_SPECIAL(name)                                                       \
    void bind##name(Type type, PyObject* (*f)(VM*, PyObject*, PyObject*)){              \
        PyObject* obj = _t(type);                                                       \
        _all_types[type].m##name = f;                                                   \
        PyObject* nf = bind_method<1>(obj, #name, [](VM* vm, ArgsView args){            \
            return lambda_get_userdata<PyObject*(*)(VM*, PyObject*, PyObject*)>(args.begin())(vm, args[0], args[1]); \
        });                                                                             \
        OBJ_GET(NativeFunc, nf).set_userdata(f);                                        \
    }

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
        OBJ_GET(NativeFunc, nf).set_userdata(f);
    }

    void bind__setitem__(Type type, void (*f)(VM*, PyObject*, PyObject*, PyObject*)){
        PyObject* obj = _t(type);
        _all_types[type].m__setitem__ = f;
        PyObject* nf = bind_method<2>(obj, "__setitem__", [](VM* vm, ArgsView args){
            lambda_get_userdata<void(*)(VM* vm, PyObject*, PyObject*, PyObject*)>(args.begin())(vm, args[0], args[1], args[2]);
            return vm->None;
        });
        OBJ_GET(NativeFunc, nf).set_userdata(f);
    }

    void bind__delitem__(Type type, void (*f)(VM*, PyObject*, PyObject*)){
        PyObject* obj = _t(type);
        _all_types[type].m__delitem__ = f;
        PyObject* nf = bind_method<1>(obj, "__delitem__", [](VM* vm, ArgsView args){
            lambda_get_userdata<void(*)(VM*, PyObject*, PyObject*)>(args.begin())(vm, args[0], args[1]);
            return vm->None;
        });
        OBJ_GET(NativeFunc, nf).set_userdata(f);
    }

    bool py_equals(PyObject* lhs, PyObject* rhs){
        if(lhs == rhs) return true;
        const PyTypeInfo* ti = _inst_type_info(lhs);
        if(ti->m__eq__) return ti->m__eq__(this, lhs, rhs);
        return call_method(lhs, __eq__, rhs) == True;
    }

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
            Type t = OBJ_GET(Type, args[0]);
            return vm->heap.gcnew<T>(t, T());
        });
    }

    template<typename T, typename __T>
    PyObject* bind_notimplemented_constructor(__T&& type) {
        return bind_constructor<-1>(std::forward<__T>(type), [](VM* vm, ArgsView args){
            vm->NotImplementedError();
            return vm->None;
        });
    }

    template<int ARGC>
    PyObject* bind_builtin_func(Str name, NativeFuncC fn) {
        return bind_func<ARGC>(builtins, name, fn);
    }

    int normalized_index(int index, int size){
        if(index < 0) index += size;
        if(index < 0 || index >= size){
            IndexError(std::to_string(index) + " not in [0, " + std::to_string(size) + ")");
        }
        return index;
    }

    PyObject* py_next(PyObject* obj){
        const PyTypeInfo* ti = _inst_type_info(obj);
        if(ti->m__next__) return ti->m__next__(this, obj);
        return call_method(obj, __next__);
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

    void StackOverflowError() { _error("StackOverflowError", ""); }
    void IOError(const Str& msg) { _error("IOError", msg); }
    void NotImplementedError(){ _error("NotImplementedError", ""); }
    void TypeError(const Str& msg){ _error("TypeError", msg); }
    void IndexError(const Str& msg){ _error("IndexError", msg); }
    void ValueError(const Str& msg){ _error("ValueError", msg); }
    void NameError(StrName name){ _error("NameError", fmt("name ", name.escape() + " is not defined")); }
    void KeyError(PyObject* obj){ _error("KeyError", OBJ_GET(Str, py_repr(obj))); }

    void AttributeError(PyObject* obj, StrName name){
        // OBJ_NAME calls getattr, which may lead to a infinite recursion
        _error("AttributeError", fmt("type ", OBJ_NAME(_t(obj)).escape(), " has no attribute ", name.escape()));
    }

    void AttributeError(Str msg){ _error("AttributeError", msg); }

    void check_type(PyObject* obj, Type type){
        if(is_type(obj, type)) return;
        TypeError("expected " + OBJ_NAME(_t(type)).escape() + ", but got " + OBJ_NAME(_t(obj)).escape());
    }

    void check_non_tagged_type(PyObject* obj, Type type){
        if(is_non_tagged_type(obj, type)) return;
        TypeError("expected " + OBJ_NAME(_t(type)).escape() + ", but got " + OBJ_NAME(_t(obj)).escape());
    }

    void check_int(PyObject* obj){
        if(is_int(obj)) return;
        check_type(obj, tp_int);    // if failed, redirect to check_type to raise TypeError
    }

    void check_float(PyObject* obj){
        if(is_float(obj)) return;
        check_type(obj, tp_float);  // if failed, redirect to check_type to raise TypeError
    }

    PyObject* _t(Type t){
        return _all_types[t.index].obj;
    }

    PyObject* _t(PyObject* obj){
        if(is_int(obj)) return _t(tp_int);
        if(is_float(obj)) return _t(tp_float);
        return _all_types[obj->type].obj;
    }

    struct ImportContext{
        // 0: normal; 1: __init__.py; 2: relative
        std::vector<std::pair<StrName, int>> pending;

        struct Temp{
            VM* vm;
            StrName name;

            Temp(VM* vm, StrName name, int type): vm(vm), name(name){
                ImportContext* ctx = &vm->_import_context;
                ctx->pending.emplace_back(name, type);
            }

            ~Temp(){
                ImportContext* ctx = &vm->_import_context;
                ctx->pending.pop_back();
            }
        };

        Temp temp(VM* vm, StrName name, int type){
            return Temp(vm, name, type);
        }
    };

    ImportContext _import_context;

    PyObject* py_import(StrName name, bool relative=false){
        Str filename;
        int type;
        if(relative){
            ImportContext* ctx = &_import_context;
            type = 2;
            for(auto it=ctx->pending.rbegin(); it!=ctx->pending.rend(); ++it){
                if(it->second == 2) continue;
                if(it->second == 1){
                    filename = fmt(it->first, kPlatformSep, name, ".py");
                    name = fmt(it->first, '.', name).c_str();
                    break;
                }
            }
            if(filename.length() == 0) _error("ImportError", "relative import outside of package");
        }else{
            type = 0;
            filename = fmt(name, ".py");
        }
        for(auto& [k, v]: _import_context.pending){
            if(k == name){
                vm->_error("ImportError", fmt("circular import ", name.escape()));
            }
        }
        PyObject* ext_mod = _modules.try_get(name);
        if(ext_mod == nullptr){
            Str source;
            auto it = _lazy_modules.find(name);
            if(it == _lazy_modules.end()){
                Bytes b = _read_file_cwd(filename);
                if(!relative && !b){
                    filename = fmt(name, kPlatformSep, "__init__.py");
                    b = _read_file_cwd(filename);
                    if(b) type = 1;
                }
                if(!b) _error("ImportError", fmt("module ", name.escape(), " not found"));
                source = Str(b.str());
            }else{
                source = it->second;
                _lazy_modules.erase(it);
            }
            auto _ = _import_context.temp(this, name, type);
            CodeObject_ code = compile(source, filename, EXEC_MODE);
            PyObject* new_mod = new_module(name);
            _exec(code, new_mod);
            new_mod->attr()._try_perfect_rehash();
            return new_mod;
        }else{
            return ext_mod;
        }
    }

    ~VM() {
        callstack.clear();
        s_data.clear();
        _all_types.clear();
        _modules.clear();
        _lazy_modules.clear();
    }
#if DEBUG_CEVAL_STEP
    void _log_s_data(const char* title = nullptr);
#endif
    void _unpack_as_list(ArgsView args, List& list);
    void _unpack_as_dict(ArgsView args, Dict& dict);
    PyObject* vectorcall(int ARGC, int KWARGC=0, bool op_call=false);
    CodeObject_ compile(Str source, Str filename, CompileMode mode, bool unknown_global_scope=false);
    PyObject* py_negate(PyObject* obj);
    f64 num_to_float(PyObject* obj);
    bool py_bool(PyObject* obj);
    i64 py_hash(PyObject* obj);
    PyObject* py_list(PyObject*);
    PyObject* new_module(StrName name);
    Str disassemble(CodeObject_ co);
    void init_builtin_types();
    PyObject* getattr(PyObject* obj, StrName name, bool throw_err=true);
    PyObject* get_unbound_method(PyObject* obj, StrName name, PyObject** self, bool throw_err=true, bool fallback=false);
    void parse_int_slice(const Slice& s, int length, int& start, int& stop, int& step);
    PyObject* format(Str, PyObject*);
    void setattr(PyObject* obj, StrName name, PyObject* value);
    template<int ARGC>
    PyObject* bind_method(PyObject*, Str, NativeFuncC);
    template<int ARGC>
    PyObject* bind_func(PyObject*, Str, NativeFuncC);
    void _error(Exception);
    PyObject* _run_top_frame();
    void post_init();
    PyObject* _py_generator(Frame&& frame, ArgsView buffer);
};

inline PyObject* NativeFunc::operator()(VM* vm, ArgsView args) const{
    int args_size = args.size() - (int)method;  // remove self
    if(args_size != argc && argc != -1) {
        vm->TypeError(fmt("expected ", argc, " arguments, but got ", args_size));
    }
#if DEBUG_EXTRA_CHECK
    if(f == nullptr) FATAL_ERROR();
#endif
    return f(vm, args);
}

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
    vm->check_int(obj);                                 \
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
    vm->check_float(obj);
    i64 bits = BITS(obj) & Number::c1;
    return BitsCvt(bits)._float;
}
template<> inline float _py_cast<float>(VM* vm, PyObject* obj){
    i64 bits = BITS(obj) & Number::c1;
    return BitsCvt(bits)._float;
}
template<> inline double py_cast<double>(VM* vm, PyObject* obj){
    vm->check_float(obj);
    i64 bits = BITS(obj) & Number::c1;
    return BitsCvt(bits)._float;
}
template<> inline double _py_cast<double>(VM* vm, PyObject* obj){
    i64 bits = BITS(obj) & Number::c1;
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
        BitsCvt val(static_cast<f64>(_val));        \
        i64 bits = val._int & Number::c1;           \
        i64 tail = val._int & Number::c2;           \
        if(tail == 0b10){                           \
            if(bits&0b100) bits += 0b100;           \
        }else if(tail == 0b11){                     \
            bits += 0b100;                          \
        }                                           \
        bits |= 0b10;                               \
        return reinterpret_cast<PyObject*>(bits);   \
    }

PY_VAR_FLOAT(float)
PY_VAR_FLOAT(double)

#undef PY_VAR_INT
#undef PY_VAR_FLOAT

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

inline PyObject* py_var(VM* vm, const char val[]){
    return VAR(Str(val));
}

inline PyObject* py_var(VM* vm, std::string val){
    return VAR(Str(std::move(val)));
}

inline PyObject* py_var(VM* vm, std::string_view val){
    return VAR(Str(val));
}

inline PyObject* py_var(VM* vm, NoReturn val){
    return vm->None;
}

inline PyObject* py_var(VM* vm, PyObject* val){
    return val;
}

inline PyObject* VM::py_negate(PyObject* obj){
    const PyTypeInfo* ti = _inst_type_info(obj);
    if(ti->m__neg__) return ti->m__neg__(this, obj);
    return call_method(obj, __neg__);
}

inline f64 VM::num_to_float(PyObject* obj){
    if(is_float(obj)){
        return _CAST(f64, obj);
    } else if (is_int(obj)){
        return (f64)_CAST(i64, obj);
    }
    TypeError("expected 'int' or 'float', got " + OBJ_NAME(_t(obj)).escape());
    return 0;
}

inline bool VM::py_bool(PyObject* obj){
    if(is_non_tagged_type(obj, tp_bool)) return obj == True;
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

inline PyObject* VM::py_list(PyObject* it){
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

inline void VM::parse_int_slice(const Slice& s, int length, int& start, int& stop, int& step){
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

inline i64 VM::py_hash(PyObject* obj){
    const PyTypeInfo* ti = _inst_type_info(obj);
    if(ti->m__hash__) return ti->m__hash__(this, obj);
    PyObject* ret = call_method(obj, __hash__);
    return CAST(i64, ret);
}

inline PyObject* VM::format(Str spec, PyObject* obj){
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
    if(spec[0] == '0'){
        pad_c = '0';
        spec = spec.substr(1);
    }
    char align;
    if(spec[0] == '>'){
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
            width = Number::stoi(spec.substr(0, dot).str());
            precision = Number::stoi(spec.substr(dot+1).str());
        }else{
            width = Number::stoi(spec.str());
            precision = -1;
        }
    }catch(...){
        ValueError("invalid format specifer");
    }

    if(type != 'f' && dot >= 0) ValueError("precision not allowed in the format specifier");
    Str ret;
    if(type == 'f'){
        f64 val = num_to_float(obj);
        if(precision < 0) precision = 6;
        std::stringstream ss;
        ss << std::fixed << std::setprecision(precision) << val;
        ret = ss.str();
    }else if(type == 'd'){
        ret = std::to_string(CAST(i64, obj));
    }else if(type == 's'){
        ret = CAST(Str&, obj);
    }else{
        ret = CAST(Str&, py_str(obj));
    }
    if(width > ret.length()){
        int pad = width - ret.length();
        std::string padding(pad, pad_c);
        if(align == '>') ret = padding.c_str() + ret;
        else ret = ret + padding.c_str();
    }
    return VAR(ret);
}

inline PyObject* VM::new_module(StrName name) {
    PyObject* obj = heap._new<DummyModule>(tp_module, DummyModule());
    obj->attr().set("__name__", VAR(name.sv()));
    // we do not allow override in order to avoid memory leak
    // it is because Module objects are not garbage collected
    if(_modules.contains(name)) throw std::runtime_error("module already exists");
    _modules.set(name, obj);
    return obj;
}

inline std::string _opcode_argstr(VM* vm, Bytecode byte, const CodeObject* co){
    std::string argStr = byte.arg == -1 ? "" : std::to_string(byte.arg);
    switch(byte.op){
        case OP_LOAD_CONST:
            if(vm != nullptr){
                argStr += fmt(" (", CAST(Str, vm->py_repr(co->consts[byte.arg])), ")");
            }
            break;
        case OP_LOAD_NAME: case OP_LOAD_GLOBAL: case OP_LOAD_NONLOCAL: case OP_STORE_GLOBAL:
        case OP_LOAD_ATTR: case OP_LOAD_METHOD: case OP_STORE_ATTR: case OP_DELETE_ATTR:
        case OP_IMPORT_NAME: case OP_BEGIN_CLASS: case OP_RAISE:
        case OP_DELETE_GLOBAL: case OP_INC_GLOBAL: case OP_DEC_GLOBAL:
            argStr += fmt(" (", StrName(byte.arg).sv(), ")");
            break;
        case OP_LOAD_FAST: case OP_STORE_FAST: case OP_DELETE_FAST: case OP_INC_FAST: case OP_DEC_FAST:
            argStr += fmt(" (", co->varnames[byte.arg].sv(), ")");
            break;
        case OP_LOAD_FUNCTION:
            argStr += fmt(" (", co->func_decls[byte.arg]->code->name, ")");
            break;
    }
    return argStr;
}

inline Str VM::disassemble(CodeObject_ co){
    auto pad = [](const Str& s, const int n){
        if(s.length() >= n) return s.substr(0, n);
        return s + std::string(n - s.length(), ' ');
    };

    std::vector<int> jumpTargets;
    for(auto byte : co->codes){
        if(byte.op == OP_JUMP_ABSOLUTE || byte.op == OP_POP_JUMP_IF_FALSE || byte.op == OP_POP_JUMP_IF_FALSE || byte.op == OP_SHORTCUT_IF_FALSE_OR_POP){
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

#if DEBUG_CEVAL_STEP
inline void VM::_log_s_data(const char* title) {
    if(_main == nullptr) return;
    if(callstack.empty()) return;
    std::stringstream ss;
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
            Type t = OBJ_GET(Type, obj);
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

inline void VM::init_builtin_types(){
    _all_types.push_back({heap._new<Type>(Type(1), Type(0)), -1, "object", true});
    _all_types.push_back({heap._new<Type>(Type(1), Type(1)), 0, "type", false});
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

    this->None = heap._new<Dummy>(_new_type_object("NoneType"), {});
    this->Ellipsis = heap._new<Dummy>(_new_type_object("ellipsis"), {});
    this->True = heap._new<Dummy>(tp_bool, {});
    this->False = heap._new<Dummy>(tp_bool, {});
    this->StopIteration = heap._new<Dummy>(_new_type_object("StopIterationType"), {});

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
    builtins->attr().set("slice", _t(tp_slice));

    post_init();
    for(int i=0; i<_all_types.size(); i++){
        _all_types[i].obj->attr()._try_perfect_rehash();
    }
    for(auto [k, v]: _modules.items()) v->attr()._try_perfect_rehash();
    this->_main = new_module("__main__");
}

// `heap.gc_scope_lock();` needed before calling this function
inline void VM::_unpack_as_list(ArgsView args, List& list){
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
inline void VM::_unpack_as_dict(ArgsView args, Dict& dict){
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

inline PyObject* VM::vectorcall(int ARGC, int KWARGC, bool op_call){
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

    if(is_non_tagged_type(callable, tp_native_func)){
        const auto& f = OBJ_GET(NativeFunc, callable);
        if(KWARGC != 0) TypeError("native_func does not accept keyword arguments");
        PyObject* ret = f(this, args);
        s_data.reset(p0);
        return ret;
    }

    ArgsView kwargs(p1, s_data._sp);

    if(is_non_tagged_type(callable, tp_function)){
        /*****************_py_call*****************/
        // callable must be a `function` object
        if(s_data.is_overflow()) StackOverflowError();

        const Function& fn = CAST(Function&, callable);
        const CodeObject* co = fn.decl->code.get();
        int co_nlocals = co->varnames.size();

        if(args.size() < fn.argc){
            vm->TypeError(fmt(
                "expected ",
                fn.argc - (int)method_call,
                " positional arguments, but got ",
                args.size() - (int)method_call,
                " (", fn.decl->code->name, ')'
            ));
        }

        // if this function is simple, a.k.a, no kwargs and no *args and not a generator
        // we can use a fast path to avoid using buffer copy
        if(fn.is_simple){
            if(args.size() > fn.argc) TypeError("too many positional arguments");
            int spaces = co_nlocals - fn.argc;
            for(int j=0; j<spaces; j++) PUSH(PY_NULL);
            callstack.emplace(&s_data, p0, co, fn._module, callable, FastLocals(co, args.begin()));
            if(op_call) return PY_OP_CALL;
            return _run_top_frame();
        }

        int i = 0;
        static THREAD_LOCAL PyObject* buffer[PK_MAX_CO_VARNAMES];

        // prepare args
        for(int index: fn.decl->args) buffer[index] = args[i++];
        // set extra varnames to nullptr
        for(int j=i; j<co_nlocals; j++) buffer[j] = PY_NULL;
        // prepare kwdefaults
        for(auto& kv: fn.decl->kwargs) buffer[kv.key] = kv.value;
        
        // handle *args
        if(fn.decl->starred_arg != -1){
            ArgsView vargs(args.begin() + i, args.end());
            buffer[fn.decl->starred_arg] = VAR(vargs.to_tuple());
            i += vargs.size();
        }else{
            // kwdefaults override
            for(auto& kv: fn.decl->kwargs){
                if(i >= args.size()) break;
                buffer[kv.key] = args[i++];
            }
            if(i < args.size()) TypeError(fmt("too many arguments", " (", fn.decl->code->name, ')'));
        }
        
        PyObject* vkwargs;
        if(fn.decl->starred_kwarg != -1){
            vkwargs = VAR(Dict(this));
            buffer[fn.decl->starred_kwarg] = vkwargs;
        }else{
            vkwargs = nullptr;
        }

        for(int i=0; i<kwargs.size(); i+=2){
            StrName key(CAST(int, kwargs[i]));
            int index = co->varnames_inv.try_get(key);
            if(index < 0){
                if(vkwargs == nullptr){
                    TypeError(fmt(key.escape(), " is an invalid keyword argument for ", co->name, "()"));
                }else{
                    Dict& dict = _CAST(Dict&, vkwargs);
                    dict.set(VAR(key.sv()), kwargs[i+1]);
                }
            }else{
                buffer[index] = kwargs[i+1];
            }
        }
        
        if(co->is_generator){
            s_data.reset(p0);
            return _py_generator(
                Frame(&s_data, nullptr, co, fn._module, callable),
                ArgsView(buffer, buffer + co_nlocals)
            );
        }

        // copy buffer back to stack
        s_data.reset(args.begin());
        for(int i=0; i<co_nlocals; i++) PUSH(buffer[i]);
        callstack.emplace(&s_data, p0, co, fn._module, callable, FastLocals(co, args.begin()));
        if(op_call) return PY_OP_CALL;
        return _run_top_frame();
        /*****************_py_call*****************/
    }

    if(is_non_tagged_type(callable, tp_type)){
        if(method_call) FATAL_ERROR();
        // [type, NULL, args..., kwargs...]

        DEF_SNAME(__new__);
        PyObject* new_f = find_name_in_mro(callable, __new__);
        PyObject* obj;
        if(new_f != nullptr){
            PUSH(new_f);
            PUSH(PY_NULL);
            PUSH(callable);    // cls
            for(PyObject* obj: args) PUSH(obj);
            for(PyObject* obj: kwargs) PUSH(obj);
            // if obj is not an instance of callable, the behavior is undefined
            obj = vectorcall(ARGC+1, KWARGC);
        }else{
            // fast path for object.__new__
            Type t = OBJ_GET(Type, callable);
            obj= vm->heap.gcnew<DummyInstance>(t, {});
        }

        // __init__
        PyObject* self;
        DEF_SNAME(__init__);
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
    DEF_SNAME(__call__);
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

// https://docs.python.org/3/howto/descriptor.html#invocation-from-an-instance
inline PyObject* VM::getattr(PyObject* obj, StrName name, bool throw_err){
    PyObject* objtype = _t(obj);
    // handle super() proxy
    if(is_non_tagged_type(obj, tp_super)){
        const Super& super = OBJ_GET(Super, obj);
        obj = super.first;
        objtype = _t(super.second);
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
        PyObject* val = obj->attr().try_get(name);
        if(val != nullptr) return val;
    }
    if(cls_var != nullptr){
        // bound method is non-data descriptor
        if(is_non_tagged_type(cls_var, tp_function) || is_non_tagged_type(cls_var, tp_native_func)){
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
    *self = PY_NULL;
    PyObject* objtype = _t(obj);
    // handle super() proxy
    if(is_non_tagged_type(obj, tp_super)){
        const Super& super = OBJ_GET(Super, obj);
        obj = super.first;
        objtype = _t(super.second);
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

inline void VM::setattr(PyObject* obj, StrName name, PyObject* value){
    PyObject* objtype = _t(obj);
    // handle super() proxy
    if(is_non_tagged_type(obj, tp_super)){
        Super& super = OBJ_GET(Super, obj);
        obj = super.first;
        objtype = _t(super.second);
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

inline void VM::_error(Exception e){
    if(callstack.empty()){
        e.is_re = false;
        throw e;
    }
    PUSH(VAR(e));
    _raise();
}

inline void ManagedHeap::mark() {
    for(PyObject* obj: _no_gc) OBJ_MARK(obj);
    for(auto& frame : vm->callstack.data()) frame._gc_mark();
    for(PyObject* obj: vm->s_data) OBJ_MARK(obj);
    if(_gc_marker_ex) _gc_marker_ex(vm);
    if(vm->_last_exception) OBJ_MARK(vm->_last_exception);
}

inline Str obj_type_name(VM *vm, Type type){
    return vm->_all_types[type].name;
}

#undef PY_VAR_INT
#undef PY_VAR_FLOAT

/***************************************************/

template<typename T>
PyObject* PyArrayGetItem(VM* vm, PyObject* obj, PyObject* index){
    static_assert(std::is_same_v<T, List> || std::is_same_v<T, Tuple>);
    const T& self = _CAST(T&, obj);

    if(is_type(index, vm->tp_slice)){
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

inline void VM::bind__hash__(Type type, i64 (*f)(VM*, PyObject*)){
    PyObject* obj = _t(type);
    _all_types[type].m__hash__ = f;
    PyObject* nf = bind_method<0>(obj, "__hash__", [](VM* vm, ArgsView args){
        i64 ret = lambda_get_userdata<i64(*)(VM*, PyObject*)>(args.begin())(vm, args[0]);
        return VAR(ret);
    });
    OBJ_GET(NativeFunc, nf).set_userdata(f);
}

inline void VM::bind__len__(Type type, i64 (*f)(VM*, PyObject*)){
    PyObject* obj = _t(type);
    _all_types[type].m__len__ = f;
    PyObject* nf = bind_method<0>(obj, "__len__", [](VM* vm, ArgsView args){
        i64 ret = lambda_get_userdata<i64(*)(VM*, PyObject*)>(args.begin())(vm, args[0]);
        return VAR(ret);
    });
    OBJ_GET(NativeFunc, nf).set_userdata(f);
}


inline void Dict::_probe(PyObject *key, bool &ok, int &i) const{
    ok = false;
    i = vm->py_hash(key) & _mask;
    while(_items[i].first != nullptr) {
        if(vm->py_equals(_items[i].first, key)) { ok = true; break; }
        i = (i + 1) & _mask;
    }
}

}   // namespace pkpy