#pragma once
#include <tuple>
#include <string_view>
#include <type_traits>

namespace pybind11 {

template <typename T>
constexpr bool dependent_false = false;

template <typename T, typename Tuple>
struct tuple_push_front;

template <typename T, typename... Ts>
struct tuple_push_front<T, std::tuple<Ts...>> {
    using type = std::tuple<T, Ts...>;
};

template <typename T, typename Tuple>
using tuple_push_front_t = typename tuple_push_front<T, Tuple>::type;

// traits for function types
template <typename Fn>
struct function_traits {
    static_assert(dependent_false<Fn>, "unsupported function type");
};

#define PYBIND11_FUNCTION_TRAITS_SPECIALIZE(...)                                                                       \
    template <typename R, typename... Args>                                                                            \
    struct function_traits<R(Args...) __VA_ARGS__> {                                                                   \
        using return_type = R;                                                                                         \
        using args_type = std::tuple<Args...>;                                                                         \
        constexpr static std::size_t args_count = sizeof...(Args);                                                     \
    };

PYBIND11_FUNCTION_TRAITS_SPECIALIZE()
PYBIND11_FUNCTION_TRAITS_SPECIALIZE(&)
PYBIND11_FUNCTION_TRAITS_SPECIALIZE(const)
PYBIND11_FUNCTION_TRAITS_SPECIALIZE(const&)
PYBIND11_FUNCTION_TRAITS_SPECIALIZE(noexcept)
PYBIND11_FUNCTION_TRAITS_SPECIALIZE(& noexcept)
PYBIND11_FUNCTION_TRAITS_SPECIALIZE(const noexcept)
PYBIND11_FUNCTION_TRAITS_SPECIALIZE(const& noexcept)

#undef PYBIND11_FUNCTION_TRAITS_SPECIALIZE

template <typename T>
using function_return_t = typename function_traits<T>::return_type;

template <typename T>
using function_args_t = typename function_traits<T>::args_type;

template <typename T>
constexpr std::size_t function_args_count = function_traits<T>::args_count;

// traits for member pointers
template <typename T>
struct member_traits;

template <typename M, typename C>
struct member_traits<M C::*> {
    using member_type = M;
    using class_type = C;
};

template <typename T>
using member_type_t = typename member_traits<T>::member_type;

template <typename T>
using class_type_t = typename member_traits<T>::class_type;

// some traits for distinguishing between function pointers, member function pointers and
// functors
using std::is_member_function_pointer_v;
using std::is_member_object_pointer_v;

template <typename T>
constexpr inline bool is_function_pointer_v = std::is_function_v<std::remove_pointer_t<T>>;

template <typename T, typename U = void>
constexpr bool is_functor_v = false;

template <typename T>
constexpr inline bool is_functor_v<T, std::void_t<decltype(&T::operator())>> = true;

template <typename T, typename SFINAE = void>
struct callable_traits;

template <typename T>
struct callable_traits<T, std::enable_if_t<is_member_function_pointer_v<T>>> {
    using args_type = tuple_push_front_t<class_type_t<T>&, function_args_t<member_type_t<T>>>;
    using return_type = function_return_t<member_type_t<T>>;
};

template <typename T>
struct callable_traits<T, std::enable_if_t<is_function_pointer_v<T>>> {
    using args_type = function_args_t<std::remove_pointer_t<T>>;
    using return_type = function_return_t<std::remove_pointer_t<T>>;
};

template <typename T>
struct callable_traits<T, std::enable_if_t<is_functor_v<T>>> {
    using args_type = function_args_t<member_type_t<decltype(&T::operator())>>;
    using return_type = function_return_t<member_type_t<decltype(&T::operator())>>;
};

template <typename Callable>
using callable_args_t = typename callable_traits<Callable>::args_type;

template <typename Callable>
using callable_return_t = typename callable_traits<Callable>::return_type;

template <typename Callable>
constexpr std::size_t callable_args_count_v = std::tuple_size_v<callable_args_t<Callable>>;

template <typename T>
struct type_identity {
    using type = T;
};

template <typename T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

template <typename T, typename... Ts>
constexpr inline std::size_t types_count_v = (std::is_same_v<T, Ts> + ...);

template <typename T>
constexpr inline std::size_t types_count_v<T> = 0;

template <typename T, typename... Ts>
constexpr inline int type_index_v = [] {
    bool arr[sizeof...(Ts) + 1] = {std::is_same_v<T, Ts>...};
    for(int i = 0; i < sizeof...(Ts); ++i) {
        if(arr[i]) return i;
    }
    return -1;
}();

static_assert(types_count_v<int, int, float, int> == 2);
static_assert(types_count_v<int, float, double> == 0);
static_assert(type_index_v<int, int, float, int> == 0);
static_assert(type_index_v<float, int, float, int> == 1);
static_assert(type_index_v<int, float, double> == -1);

template <typename T>
constexpr inline bool is_multiple_pointer_v = std::is_pointer_v<T> && is_multiple_pointer_v<std::remove_pointer_t<T>>;

template <typename T>
using pybind11_decay_t = std::decay_t<std::remove_pointer_t<std::decay_t<T>>>;

template <typename T>
constexpr auto type_name() {
#if __GNUC__ || __clang__
    std::string_view name = __PRETTY_FUNCTION__;
    std::size_t start = name.find('=') + 2;
    std::size_t end = name.size() - 1;
    return std::string_view{name.data() + start, end - start};
#elif _MSC_VER
    std::string_view name = __FUNCSIG__;
    std::size_t start = name.find('<') + 1;
    std::size_t end = name.rfind(">(");
    name = std::string_view{name.data() + start, end - start};
    start = name.find(' ');
    return start == std::string_view::npos ? name : std::string_view{name.data() + start + 1, name.size() - start - 1};
#else
    static_assert(false, "Unsupported compiler");
#endif
}

template <typename... Args>
struct overload_cast_t {
    template <typename Return>
    constexpr auto operator() (Return (*pf)(Args...)) const noexcept -> decltype(pf) {
        return pf;
    }

    template <typename Return, typename Class>
    constexpr auto operator() (Return (Class::*pmf)(Args...), std::false_type = {}) const noexcept -> decltype(pmf) {
        return pmf;
    }

    template <typename Return, typename Class>
    constexpr auto operator() (Return (Class::*pmf)(Args...) const, std::true_type) const noexcept -> decltype(pmf) {
        return pmf;
    }
};

/// Syntax sugar for resolving overloaded function pointers:
///  - regular: static_cast<Return (Class::*)(Arg0, Arg1, Arg2)>(&Class::func)
///  - sweet:   overload_cast<Arg0, Arg1, Arg2>(&Class::func)
template <typename... Args>
constexpr inline overload_cast_t<Args...> overload_cast;

/// Const member function selector for overload_cast
///  - regular: static_cast<Return (Class::*)(Arg) const>(&Class::func)
///  - sweet:   overload_cast<Arg>(&Class::func, const_)
constexpr static auto const_ = std::true_type{};
}  // namespace pybind11
