#pragma once

#include "common.h"
#include "namedict.h"
#include "tuplelist.h"

namespace pkpy {
    
struct CodeObject;
struct Frame;
struct Function;
class VM;

typedef std::function<PyObject*(VM*, ArgsView)> NativeFuncRaw;
typedef shared_ptr<CodeObject> CodeObject_;

struct NativeFunc {
    NativeFuncRaw f;
    int argc;       // DONOT include self
    bool method;
    
    NativeFunc(NativeFuncRaw f, int argc, bool method) : f(f), argc(argc), method(method) {}
    PyObject* operator()(VM* vm, ArgsView args) const;
};

struct FuncDecl {
    struct KwArg {
        int key;                // index in co->varnames
        PyObject* value;        // default value
    };
    CodeObject_ code;           // code object of this function
    pod_vector<int> args;       // indices in co->varnames
    int starred_arg = -1;       // index in co->varnames, -1 if no *arg
    pod_vector<KwArg> kwargs;   // indices in co->varnames
    bool nested = false;        // whether this function is nested
    void _gc_mark() const;
};

using FuncDecl_ = shared_ptr<FuncDecl>;

struct BoundMethod {
    PyObject* obj;
    PyObject* method;
    BoundMethod(PyObject* obj, PyObject* method) : obj(obj), method(method) {}
};

struct Range {
    i64 start = 0;
    i64 stop = -1;
    i64 step = 1;
};

using Super = std::pair<PyObject*, Type>;

// TODO: re-examine the design of Slice
struct Slice {
    int start = 0;
    int stop = 0x7fffffff;
    int step = 1;

    void normalize(int len){
        if(start < 0) start += len;
        if(stop < 0) stop += len;
        if(start < 0) start = 0;
        if(stop > len) stop = len;
        if(stop < start) stop = start;
    }
};

class BaseIter {
protected:
    VM* vm;
public:
    BaseIter(VM* vm) : vm(vm) {}
    virtual void _gc_mark() const {}
    virtual PyObject* next() = 0;
    virtual ~BaseIter() = default;
};

struct GCHeader {
    bool enabled;   // whether this object is managed by GC
    bool marked;    // whether this object is marked
    GCHeader() : enabled(true), marked(false) {}
};

struct PyObject{
    GCHeader gc;
    Type type;
    NameDict* _attr;

    bool is_attr_valid() const noexcept { return _attr != nullptr; }
    NameDict& attr() noexcept { return *_attr; }
    PyObject* attr(StrName name) const noexcept { return (*_attr)[name]; }
    virtual void* value() = 0;
    virtual void _obj_gc_mark() = 0;

    PyObject(Type type) : type(type) {}
    virtual ~PyObject() {
        if(_attr == nullptr) return;
        _attr->~NameDict();
        pool64.dealloc(_attr);
    }

    void enable_instance_dict(float lf=kInstAttrLoadFactor) noexcept {
        _attr = new(pool64.alloc<NameDict>()) NameDict(lf);
    }
};

template<typename T>
void gc_mark(T& t);

template <typename T>
struct Py_ final: PyObject {
    T _value;

    Py_(Type type, const T& val): PyObject(type), _value(val) { _init(); }
    Py_(Type type, T&& val): PyObject(type), _value(std::move(val)) { _init(); }

    void _init() noexcept {
        if constexpr (std::is_same_v<T, Type> || std::is_same_v<T, DummyModule>) {
            _attr = new(pool64.alloc<NameDict>()) NameDict(kTypeAttrLoadFactor);
        }else if constexpr(std::is_same_v<T, DummyInstance>){
            _attr = new(pool64.alloc<NameDict>()) NameDict(kInstAttrLoadFactor);
        }else if constexpr(std::is_same_v<T, Function> || std::is_same_v<T, NativeFunc>){
            _attr = new(pool64.alloc<NameDict>()) NameDict(kInstAttrLoadFactor);
        }else{
            _attr = nullptr;
        }
    }
    void* value() override { return &_value; }

    void _obj_gc_mark() override {
        if(gc.marked) return;
        gc.marked = true;
        if(_attr != nullptr) pkpy::gc_mark<NameDict>(*_attr);
        pkpy::gc_mark<T>(_value);   // handle PyObject* inside _value `T`
    }
};

#define OBJ_GET(T, obj) (((Py_<T>*)(obj))->_value)
#define OBJ_MARK(obj) if(!is_tagged(obj)) (obj)->_obj_gc_mark()

Str obj_type_name(VM* vm, Type type);

#if DEBUG_NO_BUILTIN_MODULES
#define OBJ_NAME(obj) Str("<?>")
#else
#define OBJ_NAME(obj) OBJ_GET(Str, vm->getattr(obj, __name__))
#endif

const int kTpIntIndex = 2;
const int kTpFloatIndex = 3;

inline bool is_type(PyObject* obj, Type type) {
#if DEBUG_EXTRA_CHECK
    if(obj == nullptr) throw std::runtime_error("is_type() called with nullptr");
#endif
    switch(type.index){
        case kTpIntIndex: return is_int(obj);
        case kTpFloatIndex: return is_float(obj);
        default: return !is_tagged(obj) && obj->type == type;
    }
}

inline bool is_non_tagged_type(PyObject* obj, Type type) {
#if DEBUG_EXTRA_CHECK
    if(obj == nullptr) throw std::runtime_error("is_non_tagged_type() called with nullptr");
#endif
    return !is_tagged(obj) && obj->type == type;
}

#define PY_CLASS(T, mod, name) \
    static Type _type(VM* vm) {  \
        static const StrName __x0(#mod);      \
        static const StrName __x1(#name);     \
        return OBJ_GET(Type, vm->_modules[__x0]->attr(__x1));               \
    }                                                                       \
    static PyObject* register_class(VM* vm, PyObject* mod) {                \
        PyObject* type = vm->new_type_object(mod, #name, vm->tp_object);    \
        if(OBJ_NAME(mod) != #mod) {                                         \
            auto msg = fmt("register_class() failed: ", OBJ_NAME(mod), " != ", #mod); \
            throw std::runtime_error(msg);                                  \
        }                                                                   \
        T::_register(vm, mod, type);                                        \
        type->attr()._try_perfect_rehash();                                 \
        return type;                                                        \
    }                                                                       

union BitsCvt {
    i64 _int;
    f64 _float;
    BitsCvt(i64 val) : _int(val) {}
    BitsCvt(f64 val) : _float(val) {}
};

template <typename, typename=void> struct is_py_class : std::false_type {};
template <typename T> struct is_py_class<T, std::void_t<decltype(T::_type)>> : std::true_type {};

template<typename T> void _check_py_class(VM*, PyObject*);
template<typename T> T py_pointer_cast(VM*, PyObject*);
template<typename T> T py_value_cast(VM*, PyObject*);

template<typename __T>
__T py_cast(VM* vm, PyObject* obj) {
    using T = std::decay_t<__T>;
    if constexpr(std::is_pointer_v<T>){
        return py_pointer_cast<T>(vm, obj);
    }else if constexpr(is_py_class<T>::value){
        _check_py_class<T>(vm, obj);
        return OBJ_GET(T, obj);
    }else if constexpr(std::is_pod_v<T>){
        return py_value_cast<T>(vm, obj);
    }else{
        return Discarded();
    }
}

template<typename __T>
__T _py_cast(VM* vm, PyObject* obj) {
    using T = std::decay_t<__T>;
    if constexpr(std::is_pointer_v<__T>){
        return py_pointer_cast<__T>(vm, obj);
    }else if constexpr(is_py_class<T>::value){
        return OBJ_GET(T, obj);
    }else{
        return Discarded();
    }
}

#define VAR(x) py_var(vm, x)
#define VAR_T(T, ...) vm->heap.gcnew<T>(T::_type(vm), T(__VA_ARGS__))
#define CAST(T, x) py_cast<T>(vm, x)
#define _CAST(T, x) _py_cast<T>(vm, x)

}   // namespace pkpy