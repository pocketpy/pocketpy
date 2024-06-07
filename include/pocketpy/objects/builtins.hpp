#pragma once

#include "pocketpy/common/vector.hpp"
#include "pocketpy/objects/object.hpp"

namespace pkpy {

struct BoundMethod {
    PyVar self;
    PyVar func;

    BoundMethod(PyVar self, PyVar func) : self(self), func(func) {}

    void _gc_mark(VM*) const;
};

struct StaticMethod {
    PyVar func;

    StaticMethod(PyVar func) : func(func) {}

    void _gc_mark(VM*) const;
};

struct ClassMethod {
    PyVar func;

    ClassMethod(PyVar func) : func(func) {}

    void _gc_mark(VM*) const;
};

struct Property {
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

struct StarWrapper {
    int level;  // either 1 or 2
    PyVar obj;

    StarWrapper(int level, PyVar obj) : level(level), obj(obj) {}

    void _gc_mark(VM*) const;
};

using Bytes = array<unsigned char>;

struct Super {
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

const inline int16_t kTpIntIndex = 3;
const inline int16_t kTpFloatIndex = 4;
const inline int16_t kTpNoneTypeIndex = 24;
const inline int16_t kTpNotImplementedTypeIndex = 25;

inline bool is_tagged(PyVar p) noexcept { return !p.is_ptr; }

inline bool is_float(PyVar p) noexcept { return p.type.index == kTpFloatIndex; }

inline bool is_int(PyVar p) noexcept { return p.type.index == kTpIntIndex; }

inline bool is_none(PyVar p) noexcept { return p.type.index == kTpNoneTypeIndex; }

inline bool is_not_implemented(PyVar p) noexcept { return p.type.index == kTpNotImplementedTypeIndex; }

inline bool is_type(PyVar obj, Type type) {
    assert(obj != nullptr);
    return obj.type == type;
}

inline bool is_type(PyObject* p, Type type) {
    assert(p != nullptr);
    return p->type == type;
}

struct MappingProxy {
    PyObject* obj;

    MappingProxy(PyObject* obj) : obj(obj) {}

    NameDict& attr() { return obj->attr(); }

    void _gc_mark(VM*) const;
};

StrName _type_name(VM* vm, Type type);
template <typename T>
T to_void_p(VM*, PyVar);
PyVar from_void_p(VM*, void*);

template <typename T>
obj_get_t<T> PyVar::obj_get() {
    if constexpr(is_sso_v<T>) {
        return as<T>();
    } else {
        assert(is_ptr);
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
extern const PyVar PY_OP_YIELD;

}  // namespace pkpy
