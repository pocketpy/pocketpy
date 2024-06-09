#pragma once

#include "pocketpy/common/str.hpp"
#include "pocketpy/common/config.h"
#include "pocketpy/objects/base.hpp"

namespace pkpy {

struct NameDict;

struct PyObject final {
    Type type;        // we have a duplicated type here for convenience
    bool gc_is_large;
    bool gc_marked;
    NameDict* _attr;  // gc will delete this on destruction

    bool is_attr_valid() const noexcept { return _attr != nullptr; }

    void* _value_ptr() noexcept { return (char*)this + 16; }

    template <typename T>
    T& as() noexcept {
        static_assert(std::is_same_v<T, std::decay_t<T>>);
        return *reinterpret_cast<T*>(_value_ptr());
    }

    NameDict& attr() {
        assert(is_attr_valid());
        return *_attr;
    }

    PyObject(Type type, bool gc_is_large) : type(type), gc_is_large(gc_is_large), gc_marked(false), _attr(nullptr) {}

    PyVar attr(StrName name) const;
};

static_assert(sizeof(PyObject) <= 16);

}  // namespace pkpy
