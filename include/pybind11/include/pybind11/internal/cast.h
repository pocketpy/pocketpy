#pragma once

#include "instance.h"
#include "builtins.h"
#include "type_traits.h"

namespace pybind11 {

    using pkpy::is_floating_point_v;
    using pkpy::is_integral_v;

    template <typename T>
    constexpr inline bool is_string_v =
        std::is_same_v<T, char*> || std::is_same_v<T, const char*> ||
        std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>;

    template <typename T>
    constexpr bool is_pyobject_v = std::is_base_of_v<handle, T>;

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

        static handle cast(bool src, return_value_policy, handle) {
            return src ? vm->True : vm->False;
        }
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
                // FIXME: support other kinds of string
                value = pkpy::_py_cast<std::string>(vm, src.ptr());
                return true;
            }

            return false;
        }

        static handle cast(const std::string& src, return_value_policy, handle) {
            return pkpy::py_var(vm, src);
        }
    };

    template <typename T>
    struct type_caster<T, std::enable_if_t<is_pyobject_v<T>>> {
        T value;

        bool load(const handle& src, bool) {
            if(isinstance<T>(src)) {
                value = reinterpret_borrow<T>(src);
                return true;
            }

            return false;
        }

        template <typename U>
        static handle cast(U&& src, return_value_policy, handle) {
            return std::forward<U>(src);
        }
    };

    template <typename T, typename>
    struct type_caster {
        value_wrapper<T> value;

        using underlying_type = std::remove_pointer_t<decltype(value.pointer)>;

        bool load(handle src, bool convert) {
            if(isinstance<underlying_type>(src)) {
                auto& i = _builtin_cast<instance>(src);
                value.pointer = &i.cast<underlying_type>();
                return true;
            }

            return false;
        }

        template <typename U>
        static handle cast(U&& value, return_value_policy policy, const handle& parent = handle()) {
            // TODO: support implicit cast
            const auto& info = typeid(underlying_type);
            bool existed = vm->_cxx_typeid_map.find(info) != vm->_cxx_typeid_map.end();
            if(existed) {
                auto type = vm->_cxx_typeid_map[info];
                return instance::create(std::forward<U>(value), type, policy, parent.ptr());
            }
            vm->TypeError("type not registered");
        }
    };

    template <typename T>
    struct type_caster<T, std::enable_if_t<std::is_pointer_v<T> || std::is_reference_v<T>>> {
        using underlying = std::conditional_t<std::is_pointer_v<T>,
                                              std::remove_pointer_t<T>,
                                              std::remove_reference_t<T>>;

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

