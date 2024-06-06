#pragma once

#include "pocketpy/common/str.hpp"
#include "pocketpy/common/config.h"
#include "pocketpy/objects/base.hpp"

namespace pkpy {

struct NameDict;

struct PyObject final {
    bool gc_marked;   // whether this object is marked
    Type type;        // we have a duplicated type here for convenience
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

    PyObject(Type type) : gc_marked(false), type(type), _attr(nullptr) {}

    PyVar attr(StrName name) const;
    static NameDict* __init_namedict(float lf);

    template <typename T, typename... Args>
    void placement_new(Args&&... args) {
        static_assert(std::is_same_v<T, std::decay_t<T>>);
        new (_value_ptr()) T(std::forward<Args>(args)...);

        // backdoor for important builtin types
        if constexpr(std::is_same_v<T, DummyInstance>) {
            _attr = __init_namedict(PK_INST_ATTR_LOAD_FACTOR);
        } else if constexpr(std::is_same_v<T, Type>) {
            _attr = __init_namedict(PK_TYPE_ATTR_LOAD_FACTOR);
        } else if constexpr(std::is_same_v<T, DummyModule>) {
            _attr = __init_namedict(PK_TYPE_ATTR_LOAD_FACTOR);
        }
    }
};

static_assert(sizeof(PyObject) <= 16);

}  // namespace pkpy
