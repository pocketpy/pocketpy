#pragma once

#include "pocketpy/common/types.hpp"
#include "pocketpy/common/traits.hpp"
#include "pocketpy/objects/base.h"

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
        PyVar__ctor2(this, (::PyObject*)existing);
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

    // SSO initialized (is_sso = true)
    template <typename T>
    PyVar(Type type, T value){
        static_assert(sizeof(T) <= 12, "SSO size exceeded");
        this->type = type;
        this->is_ptr = false;
        this->flags = 0;
        this->flags_ex = 0;
        this->_i64 = 0;
        as<T>() = value;
    }

    template <typename T>
    T& as() {
        static_assert(!std::is_reference_v<T>);
        if constexpr(sizeof(T) <= 8) {
            return reinterpret_cast<T&>(_i64);
        } else {
            return reinterpret_cast<T&>(flags_ex);
        }
    }

    explicit operator bool () const { return (bool)type; }

    void set_null() {
        memset(this, 0, sizeof(PyVar));
    }

    bool operator== (const PyVar& other) const { return PyVar__equal(this, &other); }
    bool operator!= (const PyVar& other) const { return !PyVar__equal(this, &other); }
    bool operator== (std::nullptr_t) const { return !(bool)type; }
    bool operator!= (std::nullptr_t) const { return (bool)type; }

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

    // std::less<> for map-like containers
    bool operator< (const PyVar& other) const {
        return PyVar__less(this, &other);
    }

    // implicit convert from ::PyVar
    PyVar(const ::PyVar& var) {
        memcpy(this, &var, sizeof(var));
    }
};

static_assert(sizeof(PyVar) == 16 && is_pod_v<PyVar>);
}  // namespace pkpy
