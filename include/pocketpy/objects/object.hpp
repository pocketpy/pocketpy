#pragma once

#include "pocketpy/common/namedict.hpp"
#include "pocketpy/objects/base.hpp"

namespace pkpy {
using NameDict = NameDictImpl<PyVar>;
using NameDict_ = std::shared_ptr<NameDict>;
using NameDictInt = NameDictImpl<int>;

static_assert(sizeof(NameDict) <= 128);

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

    PyVar attr(StrName name) const {
        assert(is_attr_valid());
        return (*_attr)[name];
    }

    PyObject(Type type) : gc_marked(false), type(type), _attr(nullptr) {}

    template <typename T, typename... Args>
    void placement_new(Args&&... args) {
        static_assert(std::is_same_v<T, std::decay_t<T>>);
        new (_value_ptr()) T(std::forward<Args>(args)...);

        // backdoor for important builtin types
        if constexpr(std::is_same_v<T, DummyInstance>) {
            _attr = new NameDict();
        } else if constexpr(std::is_same_v<T, Type>) {
            _attr = new NameDict(PK_TYPE_ATTR_LOAD_FACTOR);
        } else if constexpr(std::is_same_v<T, DummyModule>) {
            _attr = new NameDict(PK_TYPE_ATTR_LOAD_FACTOR);
        }
    }
};

static_assert(sizeof(PyObject) <= 16);

}  // namespace pkpy
