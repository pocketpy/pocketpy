#pragma once

#include "common.h"
#include "namedict.h"
#include "tuplelist.h"
#include <type_traits>

namespace pkpy {
    
struct CodeObject;
struct Frame;
struct BaseRef;
class VM;

typedef std::function<PyVar(VM*, Args&)> NativeFuncRaw;
typedef shared_ptr<CodeObject> CodeObject_;
typedef shared_ptr<NameDict> NameDict_;

struct NativeFunc {
    NativeFuncRaw f;
    int argc;       // DONOT include self
    bool method;
    
    NativeFunc(NativeFuncRaw f, int argc, bool method) : f(f), argc(argc), method(method) {}
    inline PyVar operator()(VM* vm, Args& args) const;
};

struct Function {
    StrName name;
    CodeObject_ code;
    std::vector<StrName> args;
    StrName starred_arg;                // empty if no *arg
    NameDict kwargs;              // empty if no k=v
    std::vector<StrName> kwargs_order;

    // runtime settings
    PyVar _module = nullptr;
    NameDict_ _closure = nullptr;

    bool has_name(const Str& val) const {
        bool _0 = std::find(args.begin(), args.end(), val) != args.end();
        bool _1 = starred_arg == val;
        bool _2 = kwargs.contains(val);
        return _0 || _1 || _2;
    }
};

struct BoundMethod {
    PyVar obj;
    PyVar method;
    BoundMethod(const PyVar& obj, const PyVar& method) : obj(obj), method(method) {}
};

struct Range {
    i64 start = 0;
    i64 stop = -1;
    i64 step = 1;
};

struct StarWrapper {
    PyVar obj;
    bool rvalue;
    StarWrapper(const PyVar& obj, bool rvalue): obj(obj), rvalue(rvalue) {}
};

struct Slice {
    int start = 0;
    int stop = 0x7fffffff; 

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
    PyVar _ref;     // keep a reference to the object so it will not be deleted while iterating
public:
    virtual PyVar next() = 0;
    PyVarRef loop_var;
    BaseIter(VM* vm, PyVar _ref) : vm(vm), _ref(_ref) {}
    virtual ~BaseIter() = default;
};

struct PyObject {
    Type type;
    NameDict* _attr;

    inline bool is_attr_valid() const noexcept { return _attr != nullptr; }
    inline NameDict& attr() noexcept { return *_attr; }
    inline const PyVar& attr(StrName name) const noexcept { return _attr->get(name); }
    virtual void* value() = 0;

    PyObject(Type type) : type(type) {}
    virtual ~PyObject() { delete _attr; }
};

template <typename T>
struct Py_ : PyObject {
    T _value;

    Py_(Type type, const T& val): PyObject(type), _value(val) { _init(); }
    Py_(Type type, T&& val): PyObject(type), _value(std::move(val)) { _init(); }

    inline void _init() noexcept {
        if constexpr (std::is_same_v<T, Type> || std::is_same_v<T, DummyModule>) {
            _attr = new NameDict(16, kTypeAttrLoadFactor);
        }else if constexpr(std::is_same_v<T, DummyInstance>){
            _attr = new NameDict(4, kInstAttrLoadFactor);
        }else if constexpr(std::is_same_v<T, Function> || std::is_same_v<T, NativeFunc>){
            _attr = new NameDict(4, kInstAttrLoadFactor);
        }else{
            _attr = nullptr;
        }
    }
    void* value() override { return &_value; }
};

#define OBJ_GET(T, obj) (((Py_<T>*)((obj).get()))->_value)
#define OBJ_NAME(obj) OBJ_GET(Str, (obj)->attr(__name__))

const int kTpIntIndex = 2;
const int kTpFloatIndex = 3;

inline bool is_type(const PyVar& obj, Type type) noexcept {
    switch(type.index){
        case kTpIntIndex: return obj.is_tag_01();
        case kTpFloatIndex: return obj.is_tag_10();
        default: return !obj.is_tagged() && obj->type == type;
    }
}

inline bool is_both_int_or_float(const PyVar& a, const PyVar& b) noexcept {
    return ((a.bits | b.bits) & 0b11) != 0b00;
}

inline bool is_both_int(const PyVar& a, const PyVar& b) noexcept {
    return (a.bits & b.bits & 0b11) == 0b01;
}

inline bool is_int(const PyVar& obj) noexcept {
    return obj.is_tag_01();
}

inline bool is_float(const PyVar& obj) noexcept {
    return obj.is_tag_10();
}

#define PY_CLASS(T, mod, name) \
    static Type _type(VM* vm) {  \
        static StrName __x0(#mod);      \
        static StrName __x1(#name);     \
        return OBJ_GET(Type, vm->_modules[__x0]->attr(__x1));               \
    }                                                                       \
    static PyVar register_class(VM* vm, PyVar mod) {                        \
        PyVar type = vm->new_type_object(mod, #name, vm->_t(vm->tp_object));\
        if(OBJ_NAME(mod) != #mod) UNREACHABLE();                            \
        T::_register(vm, mod, type);                                        \
        type->attr()._try_perfect_rehash();                                 \
        return type;                                                        \
    }                                                                       

union __8B {
    i64 _int;
    f64 _float;
    __8B(i64 val) : _int(val) {}
    __8B(f64 val) : _float(val) {}
};

template <typename, typename = void> struct is_py_class : std::false_type {};
template <typename T> struct is_py_class<T, std::void_t<decltype(T::_type)>> : std::true_type {};

template<typename T>
void _check_py_class(VM* vm, const PyVar& var);

template<typename T>
T py_pointer_cast(VM* vm, const PyVar& var);

struct Discarded {};

template<typename __T>
__T py_cast(VM* vm, const PyVar& obj) {
    using T = std::decay_t<__T>;
    if constexpr(std::is_pointer_v<T>){
        return py_pointer_cast<T>(vm, obj);
    }else if constexpr(is_py_class<T>::value){
        _check_py_class<T>(vm, obj);
        return OBJ_GET(T, obj);
    }else{
        return Discarded();
    }
}

template<typename __T>
__T _py_cast(VM* vm, const PyVar& obj) {
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
#define VAR_T(T, ...) vm->new_object(T::_type(vm), T(__VA_ARGS__))
#define CAST(T, x) py_cast<T>(vm, x)
#define _CAST(T, x) _py_cast<T>(vm, x)

}   // namespace pkpy