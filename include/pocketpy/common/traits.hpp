#pragma once

#include <type_traits>

namespace pkpy {

// is_pod_v<> for c++17 and c++20
template <typename T>
constexpr inline bool is_pod_v = std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>;

// https://en.cppreference.com/w/cpp/types/is_integral
template <typename T>
constexpr inline bool is_integral_v = !std::is_same_v<T, bool> && std::is_integral_v<T>;

template <typename T>
constexpr inline bool is_floating_point_v = std::is_same_v<T, float> || std::is_same_v<T, double>;

// by default, only `int` and `float` enable SSO
// users can specialize this template to enable SSO for other types
// SSO types cannot have instance dict
template <typename T>
constexpr inline bool is_sso_v = is_integral_v<T> || is_floating_point_v<T>;

// if is_sso_v<T> is true, return T, else return T&
template <typename T>
using obj_get_t = std::conditional_t<is_sso_v<T>, T, T&>;

template <typename T>
constexpr inline bool is_trivially_relocatable_v =
    std::is_trivially_copyable_v<T> && std::is_trivially_destructible_v<T>;

template <typename, typename = void>
struct has_gc_marker : std::false_type {};

template <typename T>
struct has_gc_marker<T, std::void_t<decltype(&T::_gc_mark)>> : std::true_type {};

template <typename T>
constexpr inline int py_sizeof = 16 + sizeof(T);
}  // namespace pkpy
