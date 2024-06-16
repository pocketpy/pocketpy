#pragma once

#include "pocketpy/common/types.hpp"
#include "pocketpy/common/traits.hpp"
#include "pocketpy/objects/base.h"

#include <cstddef>
#include <cstdint>
#include <cassert>
#include <cstdlib>
#include <cstring>

namespace pkpy {

struct Type {
    pkpy_Type index;
    constexpr Type() : index(0) {}
    constexpr Type(pkpy_Type index) : index(index) {}
    bool operator== (Type other) const { return this->index == other.index; }
    bool operator!= (Type other) const { return this->index != other.index; }
    constexpr operator pkpy_Type () const { return index; }
};

struct PyVar final: ::PyVar {
    // uninitialized
    PyVar() = default;

    // implict conversion
    PyVar(PyObject* existing){
        PyVar__ctor3(this, (::PyObject*)existing);
    }

    /* We must initialize all members to allow == operator to work correctly */
    // zero initialized
    PyVar(std::nullptr_t){
        set_null();
    }

    // PyObject* initialized (is_sso = false)
    PyVar(Type type, PyObject* p){
        PyVar__ctor(this, type, (::PyObject*)p);
    }

    PyVar(Type type, i64 value){
        this->type = type;
        this->is_ptr = false;
        this->_i64 = value;
    }

    explicit operator bool () const { return (bool)type; }

    void set_null() {
        memset(this, 0, sizeof(PyVar));
    }

    bool operator==(PyObject* other){
        return is_ptr && (PyObject*)_obj == other;
    }
    bool operator!=(PyObject* other){
        return !is_ptr || (PyObject*)_obj != other;
    }
    bool operator==(std::nullptr_t){
        return type == 0;
    }
    bool operator!=(std::nullptr_t){
        return type != 0;
    }

    PyObject* get() const {
        assert(is_ptr);
        return (PyObject*)_obj;
    }

    PyObject* operator->() const {
        assert(is_ptr);
        return (PyObject*)_obj;
    }

    i64 hash() const { return PyVar__hash(this); }

    template <typename T>
    obj_get_t<T> obj_get();

    // implicit convert from ::PyVar
    PyVar(const ::PyVar& var) {
        memcpy(this, &var, sizeof(var));
    }
};

static_assert(sizeof(PyVar) == 16 && is_pod_v<PyVar>);
}  // namespace pkpy
