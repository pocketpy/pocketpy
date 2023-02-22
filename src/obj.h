#pragma once

#include "namedict.h"
#include "tuplelist.h"

struct CodeObject;
struct Frame;
struct BaseRef;
class VM;

typedef std::function<PyVar(VM*, pkpy::Args&)> NativeFuncRaw;
typedef pkpy::shared_ptr<CodeObject> CodeObject_;

namespace pkpy{
struct NativeFunc {
    NativeFuncRaw f;
    int argc;       // DONOT include self
    bool method;
    
    NativeFunc(NativeFuncRaw f, int argc, bool method) : f(f), argc(argc), method(method) {}
    inline PyVar operator()(VM* vm, pkpy::Args& args) const;
};

struct Function {
    Str name;
    CodeObject_ code;
    std::vector<StrName> args;
    StrName starred_arg;                // empty if no *arg
    pkpy::NameDict kwargs;              // empty if no k=v
    std::vector<StrName> kwargs_order;

    // runtime settings
    PyVar _module;
    pkpy::shared_ptr<pkpy::NameDict> _closure;

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
};

struct Range {
    i64 start = 0;
    i64 stop = -1;
    i64 step = 1;
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
}

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
    pkpy::NameDict* _attr;

    inline bool is_attr_valid() const noexcept { return _attr != nullptr; }
    inline pkpy::NameDict& attr() noexcept { return *_attr; }
    inline PyVar& attr(StrName name) noexcept { return (*_attr)[name]; }
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
            _attr = new pkpy::NameDict(8, kTypeAttrLoadFactor);
        }else if constexpr(std::is_same_v<T, DummyInstance>){
            _attr = new pkpy::NameDict(4, kInstAttrLoadFactor);
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

#define PY_CLASS(mod, name) \
    inline static Type _type(VM* vm) { return OBJ_GET(Type, vm->_modules[#mod]->attr(#name)); } \
    inline static const char* _mod() { return #mod; } \
    inline static const char* _name() { return #name; }

union __8B {
    i64 _int;
    f64 _float;
    __8B(i64 val) : _int(val) {}
    __8B(f64 val) : _float(val) {}
};