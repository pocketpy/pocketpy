#pragma once

#include "common.h"
#include "namedict.h"
#include "tuplelist.h"

namespace pkpy {
    
#if PK_ENABLE_STD_FUNCTION
using NativeFuncC = function<PyVar(VM*, ArgsView)>;
#else
typedef PyVar (*NativeFuncC)(VM*, ArgsView);
#endif

enum class BindType{
    DEFAULT,
    STATICMETHOD,
    CLASSMETHOD,
};

struct BoundMethod {
    PyVar self;
    PyVar func;
    BoundMethod(PyVar self, PyVar func) : self(self), func(func) {}
    void _gc_mark(VM*) const;
};

struct StaticMethod{
    PyVar func;
    StaticMethod(PyVar func) : func(func) {}
    void _gc_mark(VM*) const;
};

struct ClassMethod{
    PyVar func;
    ClassMethod(PyVar func) : func(func) {}
    void _gc_mark(VM*) const;
};

struct Property{
    PyVar getter;
    PyVar setter;
    Property(PyVar getter, PyVar setter) : getter(getter), setter(setter) {}
    void _gc_mark(VM*) const;
};

struct Range {
    i64 start = 0;
    i64 stop = -1;
    i64 step = 1;
};

struct StackMemory{
    int count;
    StackMemory(int count) : count(count) {}
};

template<>
inline bool constexpr is_sso_v<StackMemory> = true;

struct StarWrapper{
    int level;      // either 1 or 2
    PyVar obj;
    StarWrapper(int level, PyVar obj) : level(level), obj(obj) {}
    void _gc_mark(VM*) const;
};

using Bytes = array<unsigned char>;

struct Super{
    PyVar first;
    Type second;
    Super(PyVar first, Type second) : first(first), second(second) {}
    void _gc_mark(VM*) const;
};

struct Slice {
    PyVar start;
    PyVar stop;
    PyVar step;
    Slice(PyVar start, PyVar stop, PyVar step) : start(start), stop(stop), step(step) {}
    void _gc_mark(VM*) const;
};

struct PyObject final{
    static constexpr int FIXED_SIZE = 16;
    
    bool gc_marked;     // whether this object is marked
    Type type;          // we have a duplicated type here for convenience
    NameDict* _attr;    // gc will delete this on destruction

    bool is_attr_valid() const noexcept { return _attr != nullptr; }
    void* _value_ptr() noexcept { return (char*)this + FIXED_SIZE; }

    template<typename T> T& as() noexcept {
        static_assert(std::is_same_v<T, std::decay_t<T>>);
        return *reinterpret_cast<T*>(_value_ptr());
    }

    NameDict& attr() {
        PK_DEBUG_ASSERT(is_attr_valid())
        return *_attr;
    }

    PyVar attr(StrName name) const {
        PK_DEBUG_ASSERT(is_attr_valid())
        return (*_attr)[name];
    }

    PyObject(Type type) : gc_marked(false), type(type), _attr(nullptr) {}

    template<typename T, typename ...Args>
    void placement_new(Args&&... args){
        static_assert(std::is_same_v<T, std::decay_t<T>>);
        new(_value_ptr()) T(std::forward<Args>(args)...);

        // backdoor for important builtin types
        if constexpr(std::is_same_v<T, DummyInstance>){
            _enable_instance_dict();
        }else if constexpr(std::is_same_v<T, Type>){
            _enable_instance_dict(PK_TYPE_ATTR_LOAD_FACTOR);
        }else if constexpr(std::is_same_v<T, DummyModule>){
            _enable_instance_dict(PK_TYPE_ATTR_LOAD_FACTOR);
        }
    }

    void _enable_instance_dict() {
        _attr = new(pool128_alloc<NameDict>()) NameDict();
    }
    void _enable_instance_dict(float lf){
        _attr = new(pool128_alloc<NameDict>()) NameDict(lf);
    }
};

static_assert(sizeof(PyObject) <= PyObject::FIXED_SIZE);
template<typename T>
inline constexpr int py_sizeof = PyObject::FIXED_SIZE + sizeof(T);

inline const int kTpIntIndex = 3;
inline const int kTpFloatIndex = 4;
inline const int kTpStackMemoryIndex = 27;

inline bool is_tagged(PyVar p) noexcept { return !p.is_ptr; }
inline bool is_float(PyVar p) noexcept { return p.type.index == kTpFloatIndex; }
inline bool is_int(PyVar p) noexcept { return p.type.index == kTpIntIndex; }

inline bool is_type(PyVar obj, Type type) {
    PK_DEBUG_ASSERT(obj != nullptr)
    return obj.type == type;
}

template <typename, typename=void> struct has_gc_marker : std::false_type {};
template <typename T> struct has_gc_marker<T, std::void_t<decltype(&T::_gc_mark)>> : std::true_type {};

struct MappingProxy{
    PyObject* obj;
    MappingProxy(PyObject* obj) : obj(obj) {}
    NameDict& attr() { return obj->attr(); }
    void _gc_mark(VM*) const;
};

StrName _type_name(VM* vm, Type type);
template<typename T> T to_void_p(VM*, PyVar);
PyVar from_void_p(VM*, void*);


template<typename T>
obj_get_t<T> PyVar::obj_get(){
    if constexpr(is_sso_v<T>){
        return as<T>();
    }else{
        PK_DEBUG_ASSERT(is_ptr)
        void* v = ((PyObject*)_1)->_value_ptr();
        return *reinterpret_cast<T*>(v);
    }
}

#define PK_OBJ_GET(T, obj) ((obj).obj_get<T>())

// deprecated
#define PK_OBJ_MARK(obj) vm->obj_gc_mark(obj)

#define VAR(x) py_var(vm, x)
#define CAST(T, x) py_cast<T>(vm, x)
#define _CAST(T, x) _py_cast<T>(vm, x)

#define CAST_F(x) py_cast<f64>(vm, x)
#define CAST_DEFAULT(T, x, default_value) (x != vm->None) ? py_cast<T>(vm, x) : (default_value)

/*****************************************************************/

#define PY_NULL nullptr
extern PyVar const PY_OP_CALL;
extern PyVar const PY_OP_YIELD;

}   // namespace pkpy