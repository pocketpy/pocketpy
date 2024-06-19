#pragma once
#include "instance.h"
#include "accessor.h"

namespace pybind11::impl {

using pkpy::is_floating_point_v;
using pkpy::is_integral_v;

template <typename T>
constexpr inline bool is_string_v =
    std::is_same_v<T, const char*> || std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>;

template <typename T>
constexpr bool is_pointer_v = std::is_pointer_v<T> && !std::is_same_v<T, char*> && !std::is_same_v<T, const char*>;

template <typename T, typename>
struct type_caster;

template <>
struct type_caster<bool> {
    bool value;

    bool load(const handle& src, bool) {
        if(isinstance<pybind11::bool_>(src)) {
            value = pkpy::_py_cast<bool>(vm, src.ptr());
            return true;
        }

        return false;
    }

    static handle cast(bool src, return_value_policy, handle) { return src ? vm->True : vm->False; }
};

template <typename T>
struct type_caster<T, std::enable_if_t<is_integral_v<T>>> {
    T value;

    bool load(const handle& src, bool convert) {
        if(isinstance<pybind11::int_>(src)) {
            value = pkpy::_py_cast<T>(vm, src.ptr());
            return true;
        }

        return false;
    }

    static handle cast(T src, return_value_policy, handle) { return pkpy::py_var(vm, src); }
};

template <typename T>
struct type_caster<T, std::enable_if_t<is_floating_point_v<T>>> {
    T value;

    bool load(const handle& src, bool convert) {
        if(isinstance<pybind11::float_>(src)) {
            value = pkpy::_py_cast<T>(vm, src.ptr());
            return true;
        }

        if(convert && isinstance<pybind11::int_>(src)) {
            value = pkpy::_py_cast<int64_t>(vm, src.ptr());
            return true;
        }

        return false;
    }

    static handle cast(T src, return_value_policy, handle) { return pkpy::py_var(vm, src); }
};

template <typename T>
struct type_caster<T, std::enable_if_t<is_string_v<T>>> {
    T value;

    bool load(const handle& src, bool) {
        if(isinstance<pybind11::str>(src)) {
            auto& str = src._as<pkpy::Str>();
            if constexpr(std::is_same_v<T, std::string>) {
                value = str;
            } else if constexpr(std::is_same_v<T, std::string_view>) {
                value = str;
            } else if constexpr(std::is_same_v<T, const char*>) {
                value = str.c_str();
            }
            return true;
        }

        return false;
    }

    template <typename U>
    static handle cast(U&& src, return_value_policy, handle) {
        return str(std::forward<U>(src));
    }
};

template <typename T>
struct type_caster<T, std::enable_if_t<is_pyobject_v<T>>> {
    T value;

    bool load(const handle& src, bool) {
        if(isinstance<T>(src)) {
            value = src;
            return true;
        }

        return false;
    }

    static handle cast(const handle& src, return_value_policy, handle) { return src; }
};

template <typename T, typename>
struct type_caster {
    struct value_wrapper {
        T* pointer;

        operator T& () { return *pointer; }
    };

    value_wrapper value;

    using underlying_type = std::remove_pointer_t<decltype(value.pointer)>;

    bool load(handle src, bool convert) {
        if(isinstance<underlying_type>(src)) {
            auto& i = src._as<instance>();
            value.pointer = &i._as<underlying_type>();
            return true;
        }

        return false;
    }

    template <typename U>
    static handle cast(U&& value, return_value_policy policy, const handle& parent = handle()) {
        const auto& info = typeid(underlying_type);
        auto type = type_visitor::type<underlying_type>();
        return instance::create(std::forward<U>(value), type, policy, parent.ptr());
        // TODO: support implicit cast
    }
};

template <typename T>
struct type_caster<T, std::enable_if_t<is_pointer_v<T> || std::is_reference_v<T>>> {
    using underlying =
        std::remove_cv_t<std::conditional_t<is_pointer_v<T>, std::remove_pointer_t<T>, std::remove_reference_t<T>>>;

    struct wrapper {
        type_caster<underlying> caster;

        operator T () {
            if constexpr(std::is_pointer_v<T>) {
                return caster.value.pointer;
            } else {
                return caster.value;
            }
        }
    };

    wrapper value;

    bool load(const handle& src, bool convert) { return value.caster.load(src, convert); }

    template <typename U>
    static handle cast(U&& value, return_value_policy policy, const handle& parent) {
        return type_caster<underlying>::cast(std::forward<U>(value), policy, parent);
    }
};

}  // namespace pybind11

