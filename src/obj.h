#pragma once

#include "common.h"
#include "namedict.h"
#include "tuplelist.h"

namespace pkpy {
    
struct CodeObject;
struct Frame;
struct BaseRef;
class VM;

typedef std::function<PyObject*(VM*, Args&)> NativeFuncRaw;
typedef shared_ptr<CodeObject> CodeObject_;
typedef shared_ptr<NameDict> NameDict_;

struct NativeFunc {
    NativeFuncRaw f;
    int argc;       // DONOT include self
    bool method;
    
    NativeFunc(NativeFuncRaw f, int argc, bool method) : f(f), argc(argc), method(method) {}
    PyObject* operator()(VM* vm, Args& args) const;
};

struct FuncDecl {
    StrName name;
    CodeObject_ code;
    std::vector<StrName> args;
    StrName starred_arg;                // empty if no *arg
    NameDict kwargs;              // empty if no k=v
    std::vector<StrName> kwargs_order;

    bool has_name(StrName val) const {
        bool _0 = std::find(args.begin(), args.end(), val) != args.end();
        bool _1 = starred_arg == val;
        bool _2 = kwargs.contains(val);
        return _0 || _1 || _2;
    }
};

using FuncDecl_ = shared_ptr<FuncDecl>;

struct Function{
    FuncDecl_ decl;
    PyObject* _module;
    NameDict_ _closure;
};

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

struct StarWrapper {
    PyObject* obj;
    StarWrapper(PyObject* obj): obj(obj) {}
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
    virtual void _mark() {}
    virtual PyObject* next() = 0;
    virtual ~BaseIter() = default;
};

struct GCHeader {
    bool enabled;   // whether this object is managed by GC
    bool marked;    // whether this object is marked
    GCHeader() : enabled(true), marked(false) {}
};

struct PyObject {
    GCHeader gc;
    Type type;
    NameDict* _attr;

    bool is_attr_valid() const noexcept { return _attr != nullptr; }
    NameDict& attr() noexcept { return *_attr; }
    PyObject* attr(StrName name) const noexcept { return (*_attr)[name]; }
    virtual void* value() = 0;
    virtual void _mark() = 0;

    PyObject(Type type) : type(type) {}
    virtual ~PyObject() { delete _attr; }
};

template<typename T>
void _mark(T& t);

template <typename T>
struct Py_ : PyObject {
    T _value;

    Py_(Type type, const T& val): PyObject(type), _value(val) { _init(); }
    Py_(Type type, T&& val): PyObject(type), _value(std::move(val)) { _init(); }

    void _init() noexcept {
        if constexpr (std::is_same_v<T, Type> || std::is_same_v<T, DummyModule>) {
            _attr = new NameDict(8, kTypeAttrLoadFactor);
        }else if constexpr(std::is_same_v<T, DummyInstance>){
            _attr = new NameDict(8, kInstAttrLoadFactor);
        }else if constexpr(std::is_same_v<T, Function> || std::is_same_v<T, NativeFunc>){
            _attr = new NameDict(8, kInstAttrLoadFactor);
        }else{
            _attr = nullptr;
        }
    }
    void* value() override { return &_value; }

    void _mark() override {
        if(gc.marked) return;
        gc.marked = true;
        if(_attr != nullptr) _attr->_mark();
        pkpy::_mark<T>(_value);   // handle PyObject* inside _value `T`
    }
};

#define OBJ_GET(T, obj) (((Py_<T>*)(obj))->_value)
#define OBJ_MARK(obj) if(!is_tagged(obj)) obj->_mark()

#if DEBUG_NO_BUILTIN_MODULES
#define OBJ_NAME(obj) Str("<?>")
#else
#define OBJ_NAME(obj) OBJ_GET(Str, vm->getattr(obj, __name__))
#endif

const int kTpIntIndex = 2;
const int kTpFloatIndex = 3;

inline bool is_type(PyObject* obj, Type type) noexcept {
    switch(type.index){
        case kTpIntIndex: return is_int(obj);
        case kTpFloatIndex: return is_float(obj);
        default: return !is_tagged(obj) && obj->type == type;
    }
}

#define PY_CLASS(T, mod, name) \
    static Type _type(VM* vm) {  \
        static const StrName __x0(#mod);      \
        static const StrName __x1(#name);     \
        return OBJ_GET(Type, vm->_modules[__x0]->attr(__x1));               \
    }                                                                       \
    static PyObject* register_class(VM* vm, PyObject* mod) {                \
        PyObject* type = vm->new_type_object(mod, #name, vm->tp_object);    \
        if(OBJ_NAME(mod) != #mod) UNREACHABLE();                            \
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
struct Discarded { };

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