#pragma once

#include "pocketpy/common/types.hpp"
#include "pocketpy/common/traits.hpp"

#include <cstdint>
#include <cassert>
#include <cstdlib>
#include <cstring>

namespace pkpy {

struct Type {
    int16_t index;

    constexpr Type() : index(0) {}

    explicit constexpr Type(int index) : index(index) {}

    bool operator== (Type other) const { return this->index == other.index; }

    bool operator!= (Type other) const { return this->index != other.index; }

    constexpr operator int () const { return index; }
};

struct const_sso_var {};

struct PyVar final {
    Type type;
    bool is_ptr;
    uint8_t flags;
    // 12 bytes SSO
    int _0;
    i64 _1;

    // uninitialized
    PyVar() = default;

    // implict conversion
    PyVar(PyObject* p);

    /* We must initialize all members to allow == operator to work correctly */
    // constexpr initialized
    constexpr PyVar(const const_sso_var&, Type type, int value) :
        type(type), is_ptr(false), flags(0), _0(value), _1(0) {}

    // zero initialized
    constexpr PyVar(std::nullptr_t) : type(0), is_ptr(false), flags(0), _0(0), _1(0) {}

    // PyObject* initialized (is_sso = false)
    PyVar(Type type, PyObject* p) : type(type), is_ptr(true), flags(0), _0(0), _1(reinterpret_cast<i64>(p)) {}

    // SSO initialized (is_sso = true)
    template <typename T>
    PyVar(Type type, T value) : type(type), is_ptr(false), flags(0), _0(0), _1(0) {
        static_assert(sizeof(T) <= 12, "SSO size exceeded");
        as<T>() = value;
    }

    template <typename T>
    T& as() {
        static_assert(!std::is_reference_v<T>);
        if constexpr(sizeof(T) <= 8) {
            return reinterpret_cast<T&>(_1);
        } else {
            return reinterpret_cast<T&>(_0);
        }
    }

    explicit operator bool () const { return (bool)type; }

    void set_null() {
        _qword(0) = 0;
        _qword(1) = 0;
    }

    i64 _qword(int i) const { return ((const i64*)this)[i]; }

    i64& _qword(int i) { return ((i64*)this)[i]; }

    bool operator== (const PyVar& other) const { return _qword(0) == other._qword(0) && _qword(1) == other._qword(1); }

    bool operator!= (const PyVar& other) const { return _qword(0) != other._qword(0) || _qword(1) != other._qword(1); }

    bool operator== (std::nullptr_t) const { return !(bool)type; }

    bool operator!= (std::nullptr_t) const { return (bool)type; }

    PyObject* get() const {
        assert(is_ptr);
        return reinterpret_cast<PyObject*>(_1);
    }

    PyObject* operator->() const {
        assert(is_ptr);
        return reinterpret_cast<PyObject*>(_1);
    }

    i64 hash() const { return _0 + _1; }

    template <typename T>
    obj_get_t<T> obj_get();

    // std::less<> for map-like containers
    bool operator< (const PyVar& other) const { return memcmp(this, &other, sizeof(PyVar)) < 0; }
};

static_assert(sizeof(PyVar) == 16 && is_pod_v<PyVar>);
}  // namespace pkpy
