#pragma once

#include "pocketpy/common/str.hpp"
#include "pocketpy/common/config.h"
#include "pocketpy/objects/base.hpp"
#include "pocketpy/objects/object.h"

namespace pkpy {

struct NameDict;

struct PyObject final: ::PyObject {
    bool is_attr_valid() const noexcept { return _attr != nullptr; }

    void* _value_ptr() noexcept { return (char*)this + 16; }

    NameDict& attr() const{
        return *(NameDict*)_attr;
    }

    PyVar attr(StrName name) const;

    template <typename T>
    T& as() noexcept {
        static_assert(std::is_same_v<T, std::decay_t<T>>);
        return *reinterpret_cast<T*>(_value_ptr());
    }

    PyObject(Type type, bool gc_is_large){
        this->type = type;
        this->gc_is_large = gc_is_large;
        this->gc_marked = false;
        this->_attr = nullptr;
    }
};

static_assert(sizeof(PyObject) <= 16);

}  // namespace pkpy
