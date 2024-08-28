#pragma once

#include "instance.h"

namespace pkbind {

template <typename T>
constexpr inline bool is_string_v =
    std::is_same_v<T, const char*> || std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>;

/// The `type_caster` class is responsible for converting between Python objects and C++ objects.
///
/// The static method `type_caster<T>::cast(...)` is used to convert a C++ object to a Python object.
/// If the conversion fails, an exception is thrown.
///
/// The method `type_caster<T>::load(...)` is used to try to convert a Python object to a C++ object.
/// If the conversion is successful, it returns true, and you can then call `type_caster<T>::value()`
/// to access the resulting C++ object. If the conversion fails, it returns false.
///
/// NOTE: The type T could be a reference type or a pointer type. What is the lifetime of the reference or pointer?
/// It depends on the referenced type. For some types, such as bool, int, float, etc., the loaded value is stored
/// in the type_caster itself, so the lifetime of the reference is no longer than the lifetime of the type_caster
/// object. For other user-registered types, the lifetime of the reference is the same as the corresponding Python
/// object. A static variable `is_temporary_v` is used to indicate whether the loaded value is temporary or not.

template <typename T, typename SFINAE>
struct type_caster {
    T* data;

    static_assert(!std::is_pointer_v<T>, "type caster for pointer type must be specialized.");
    static_assert(!std::is_reference_v<T>, "type caster for reference type must be specialized.");

    template <typename U>
    static object cast(U&& value, return_value_policy policy, handle parent) {
        // TODO: support implicit cast
        return instance::create(type::of<T>(), std::forward<U>(value), parent, policy);
    }

    bool load(handle src, bool convert) {
        if(isinstance<T>(src)) {
            auto& i = *static_cast<instance*>(py_touserdata(src.ptr()));
            data = &i.as<T>();
            return true;
        }

        return false;
    }

    T& value() { return *data; }

    constexpr inline static bool is_temporary_v = false;
};

template <>
struct type_caster<bool> {
    bool data;

    static object cast(bool src, return_value_policy, handle) { return bool_(src); }

    bool load(handle src, bool) {
        if(isinstance<pkbind::bool_>(src)) {
            data = py_tobool(src.ptr());
            return true;
        }

        return false;
    }

    bool& value() { return data; }

    constexpr inline static bool is_temporary_v = true;
};

template <typename T>
struct type_caster<T, std::enable_if_t<is_integer_v<T>>> {
    T data;

    static object cast(T src, return_value_policy, handle) { return int_(src); }

    bool load(handle src, bool) {
        if(isinstance<int_>(src)) {
            data = static_cast<T>(py_toint(src.ptr()));
            return true;
        }

        return false;
    }

    T& value() { return data; }

    constexpr inline static bool is_temporary_v = true;
};

template <typename T>
struct type_caster<T, std::enable_if_t<is_floating_point_v<T>>> {
    T data;

    static object cast(T src, return_value_policy, handle) { return float_(src); }

    bool load(handle src, bool convert) {
        if(isinstance<pkbind::float_>(src)) {
            data = static_cast<T>(py_tofloat(src.ptr()));
            return true;
        }

        if(convert && isinstance<pkbind::int_>(src)) {
            data = static_cast<T>(py_toint(src.ptr()));
            return true;
        }

        return false;
    }

    T& value() { return data; }

    constexpr inline static bool is_temporary_v = true;
};

template <typename T>
struct type_caster<T, std::enable_if_t<is_string_v<T>>> {
    T data;

    template <typename U>
    static object cast(U&& src, return_value_policy, handle) {
        return str(std::forward<U>(src));
    }

    bool load(handle src, bool) {
        if(isinstance<pkbind::str>(src)) {
            data = py_tostr(src.ptr());
            return true;
        }

        return false;
    }

    T& value() { return data; }

    constexpr inline static bool is_temporary_v = true;
};

template <typename T>
struct type_caster<T, std::enable_if_t<is_pyobject_v<T>>> {
    T data;

    template <typename U>
    static object cast(U&& src, return_value_policy, handle) {
        return object(std::forward<U>(src));
    }

    bool load(handle src, bool) {
        if(isinstance<T>(src)) {
            data = T(src.ptr(), object::realloc_t{});
            return true;
        }

        return false;
    }

    T& value() { return data; }

    constexpr inline static bool is_temporary_v = true;
};

template <typename T>
struct type_caster<T, std::enable_if_t<is_pointer_v<T> || std::is_reference_v<T>>> {
    using underlying =
        std::remove_cv_t<std::conditional_t<is_pointer_v<T>, std::remove_pointer_t<T>, std::remove_reference_t<T>>>;

    type_caster<underlying> caster;

    template <typename U>
    static object cast(U&& value, return_value_policy policy, handle parent) {
        return type_caster<underlying>::cast(std::forward<U>(value), policy, parent);
    }

    bool load(handle src, bool convert) { return caster.load(src, convert); }

    T value() {
        if constexpr(std::is_pointer_v<T>) {
            return &caster.value();
        } else {
            return caster.value();
        }
    }

    constexpr inline static bool is_temporary_v = type_caster<underlying>::is_temporary_v;
};

}  // namespace pkbind
