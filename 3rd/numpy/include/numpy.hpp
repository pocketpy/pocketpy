#pragma once

#include <any>
#include <cstdint>
#include <complex>
#include <chrono>
#include <iostream>
#include <limits>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

// Suppress xtensor warnings if SUPPRESS_XTENSOR_WARNINGS is set
#ifdef SUPPRESS_XTENSOR_WARNINGS
    #ifdef _MSC_VER
        #pragma warning(push, 0)
    #else
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wall"
        #pragma GCC diagnostic ignored "-Wextra"
        #pragma GCC system_header
    #endif
#endif

#include <xtensor/xarray.hpp>
#include <xtensor/xio.hpp>
#include <xtensor/xmath.hpp>
#include <xtensor/xrandom.hpp>
#include <xtensor/xsort.hpp>
#include <xtensor/xview.hpp>

#ifdef SUPPRESS_XTENSOR_WARNINGS
    #ifdef _MSC_VER
        #pragma warning(pop)
    #else
        #pragma GCC diagnostic pop
    #endif
#endif

namespace pkpy {

// Type aliases
using int8 = int8_t;
using int16 = int16_t;
using int32 = int32_t;
using int64 = int64_t;
using uint8 = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;
using int_ = int64;
using float32 = float;
using float64 = double;
using float_ = float64;
using bool_ = bool;
using complex64 = std::complex<float32>;
using complex128 = std::complex<float64>;
using complex_ = complex128;
using string = std::string;

template <typename T>
struct dtype_traits {
    constexpr const static char* name = "unknown";
};

#define REGISTER_DTYPE(Type, Name)                                                                                     \
    template <>                                                                                                        \
    struct dtype_traits<Type> {                                                                                        \
        static constexpr const char* name = Name;                                                                      \
    };

REGISTER_DTYPE(int8_t, "int8");
REGISTER_DTYPE(int16_t, "int16");
REGISTER_DTYPE(int32_t, "int32");
REGISTER_DTYPE(int64_t, "int64");
REGISTER_DTYPE(uint8_t, "uint8");
REGISTER_DTYPE(uint16_t, "uint16");
REGISTER_DTYPE(uint32_t, "uint32");
REGISTER_DTYPE(uint64_t, "uint64");
REGISTER_DTYPE(float, "float32");
REGISTER_DTYPE(float_, "float64");
REGISTER_DTYPE(bool_, "bool");
REGISTER_DTYPE(std::complex<float32>, "complex64");
REGISTER_DTYPE(std::complex<float64>, "complex128");

using _Dtype = std::string;
using _ShapeLike = std::vector<int>;

namespace numpy {

template <typename T>
class ndarray;

template <typename T>
constexpr inline auto is_ndarray_v = false;

template <typename T>
constexpr inline auto is_ndarray_v<ndarray<T>> = true;

template <typename T>
class ndarray {
public:
    // Constructor for xtensor xarray
    ndarray() = default;

    ndarray(const T scalar) : _array(scalar) {}

    ndarray(const xt::xarray<T>& arr) : _array(arr) {}

    // Constructor for mutli-dimensional array
    ndarray(std::initializer_list<T> init_list) : _array(init_list) {}

    ndarray(std::initializer_list<std::initializer_list<T>> init_list) : _array(init_list) {}

    ndarray(std::initializer_list<std::initializer_list<std::initializer_list<T>>> init_list) : _array(init_list) {}

    ndarray(std::initializer_list<std::initializer_list<std::initializer_list<std::initializer_list<T>>>> init_list) :
        _array(init_list) {}

    ndarray(std::initializer_list<
            std::initializer_list<std::initializer_list<std::initializer_list<std::initializer_list<T>>>>> init_list) :
        _array(init_list) {}

    // Accessor function for _array
    const xt::xarray<T>& get_array() const { return _array; }

    // Properties
    _Dtype dtype() const { return dtype_traits<T>::name; }

    int ndim() const { return static_cast<int>(_array.dimension()); }

    int size() const { return static_cast<int>(_array.size()); }

    _ShapeLike shape() const { return _ShapeLike(_array.shape().begin(), _array.shape().end()); }

    // Dunder Methods
    template <typename U>
    auto operator== (const ndarray<U>& other) const {
        return ndarray<bool_>(xt::equal(_array, other.get_array()));
    }

    template <typename U>
    auto operator!= (const ndarray<U>& other) const {
        return ndarray<bool_>(xt::not_equal(_array, other.get_array()));
    }

    template <typename U>
    auto operator+ (const ndarray<U>& other) const {
        using result_type = std::common_type_t<T, U>;
        xt::xarray<result_type> result = xt::cast<result_type>(_array) + xt::cast<result_type>(other.get_array());
        return ndarray<result_type>(result);
    }

    template <typename U, typename = std::enable_if_t<!is_ndarray_v<U>>>
    auto operator+ (const U& other) const {
        return binary_operator_add_impl<U>(other);
    }

    template <typename U>
    auto binary_operator_add_impl(const U& other) const {
        if constexpr(std::is_same_v<U, float_>) {
            xt::xarray<float_> result = xt::cast<float_>(_array) + other;
            return ndarray<float_>(result);
        } else {
            using result_type = std::common_type_t<T, U>;
            xt::xarray<result_type> result = xt::cast<result_type>(_array) + other;
            return ndarray<result_type>(result);
        }
    }

    template <typename U>
    auto operator- (const ndarray<U>& other) const {
        using result_type = std::common_type_t<T, U>;
        xt::xarray<result_type> result = xt::cast<result_type>(_array) - xt::cast<result_type>(other.get_array());
        return ndarray<result_type>(result);
    }

    template <typename U, typename = std::enable_if_t<!is_ndarray_v<U>>>
    auto operator- (const U& other) const {
        return binary_operator_sub_impl<U>(other);
    }

    template <typename U>
    auto binary_operator_sub_impl(const U& other) const {
        if constexpr(std::is_same_v<U, float_>) {
            xt::xarray<float_> result = xt::cast<float_>(_array) - other;
            return ndarray<float_>(result);
        } else {
            using result_type = std::common_type_t<T, U>;
            xt::xarray<result_type> result = xt::cast<result_type>(_array) - other;
            return ndarray<result_type>(result);
        }
    }

    template <typename U>
    auto operator* (const ndarray<U>& other) const {
        using result_type = std::common_type_t<T, U>;
        xt::xarray<result_type> result = xt::cast<result_type>(_array) * xt::cast<result_type>(other.get_array());
        return ndarray<result_type>(result);
    }

    template <typename U, typename = std::enable_if_t<!is_ndarray_v<U>>>
    auto operator* (const U& other) const {
        return binary_operator_mul_impl<U>(other);
    }

    template <typename U>
    auto binary_operator_mul_impl(const U& other) const {
        if constexpr(std::is_same_v<U, float_>) {
            xt::xarray<float_> result = xt::cast<float_>(_array) * other;
            return ndarray<float_>(result);
        } else {
            using result_type = std::common_type_t<T, U>;
            xt::xarray<result_type> result = xt::cast<result_type>(_array) * other;
            return ndarray<result_type>(result);
        }
    }

    template <typename U>
    auto operator/ (const ndarray<U>& other) const {
        using result_type = std::conditional_t<std::is_same_v<T, bool> || std::is_same_v<U, bool>, float64, std::common_type_t<T, U>>;
        xt::xarray<result_type> result = xt::cast<result_type>(_array) / xt::cast<result_type>(other.get_array());
        return ndarray<result_type>(result);
    }

    template <typename U, typename = std::enable_if_t<!is_ndarray_v<U>>>
    auto operator/ (const U& other) const {
        return binary_operator_truediv_impl<U>(other);
    }

    template <typename U>
    auto binary_operator_truediv_impl(const U& other) const {
        xt::xarray<float_> result = xt::cast<float_>(_array) / static_cast<float_>(other);
        return ndarray<float_>(result);
    }

    template <typename U>
    auto pow(const ndarray<U>& other) const {
        using result_type = std::common_type_t<T, U>;
        xt::xarray<result_type> result =
            xt::pow(xt::cast<result_type>(_array), xt::cast<result_type>(other.get_array()));
        return ndarray<result_type>(result);
    }

    template <typename U, typename = std::enable_if_t<!is_ndarray_v<U>>>
    auto pow(const U& other) const {
        return pow_impl<U>(other);
    }

    template <typename U>
    auto pow_impl(const U& other) const {
        xt::xarray<float_> result = xt::pow(xt::cast<float_>(_array), other);
        return ndarray<float_>(result);
    }

    template <typename U>
    ndarray operator& (const ndarray<U>& other) const {
        using result_type = std::common_type_t<T, U>;
        xt::xarray<result_type> result = xt::cast<result_type>(_array) & xt::cast<result_type>(other.get_array());
        return ndarray(result);
    }

    template <typename U, typename = std::enable_if_t<!is_ndarray_v<U>>>
    ndarray operator& (const U& other) const {
        xt::xarray<T> result = _array & static_cast<T>(other);
        return ndarray(result);
    }

    template <typename U>
    ndarray operator| (const ndarray<U>& other) const {
        using result_type = std::common_type_t<T, U>;
        xt::xarray<result_type> result = xt::cast<result_type>(_array) | xt::cast<result_type>(other.get_array());
        return ndarray(result);
    }

    template <typename U, typename = std::enable_if_t<!is_ndarray_v<U>>>
    ndarray operator| (const U& other) const {
        xt::xarray<T> result = _array | static_cast<T>(other);
        return ndarray(result);
    }

    template <typename U>
    ndarray operator^ (const ndarray<U>& other) const {
        using result_type = std::common_type_t<T, U>;
        xt::xarray<result_type> result = xt::cast<result_type>(_array) ^ xt::cast<result_type>(other.get_array());
        return ndarray(result);
    }

    template <typename U, typename = std::enable_if_t<!is_ndarray_v<U>>>
    ndarray operator^ (const U& other) const {
        xt::xarray<T> result = _array ^ static_cast<T>(other);
        return ndarray(result);
    }

    ndarray operator~() const { return ndarray(~(_array)); }

    ndarray operator!() const { return ndarray(!(_array)); }

    T operator() (int index) const { return _array(index); }

    ndarray operator[] (int index) const { return ndarray(xt::view(_array, index, xt::all())); }

    ndarray operator[] (const std::vector<int>& indices) const { return ndarray(xt::view(_array, xt::keep(indices))); }

    ndarray operator[] (const std::tuple<int, int, int>& slice) const {
        return ndarray(xt::view(_array, xt::range(std::get<0>(slice), std::get<1>(slice), std::get<2>(slice))));
    }

    template <typename... Args>
    T operator() (Args... args) const {
        return _array(args...);
    }

    void set_item(int index, const ndarray<T>& value) { xt::view(_array, index, xt::all()) = value.get_array(); }

    void set_item(int i1, int i2, const ndarray<T>& value) { xt::view(_array, i1, i2, xt::all()) = value.get_array(); }

    void set_item(int i1, int i2, int i3, const ndarray<T>& value) { xt::view(_array, i1, i2, i3, xt::all()) = value.get_array(); }

    void set_item(int i1, int i2, int i3, int i4, const ndarray<T>& value) { xt::view(_array, i1, i2, i3, i4, xt::all()) = value.get_array(); }

    void set_item(int i1, int i2, int i3, int i4, int i5, const ndarray<T>& value) { xt::view(_array, i1, i2, i3, i4, i5, xt::all()) = value.get_array(); }

    void set_item(const std::vector<int>& indices, const ndarray<T>& value) {
        xt::view(_array, xt::keep(indices)) = value.get_array();
    }

    void set_item(const std::tuple<int, int, int>& slice, const ndarray<T>& value) {
        xt::view(_array, xt::range(std::get<0>(slice), std::get<1>(slice), std::get<2>(slice))) = value.get_array();
    }

    void set_item(int i1, int i2, T value) { xt::view(_array, i1, i2) = value; }

    void set_item(int i1, int i2, int i3, T value) { xt::view(_array, i1, i2, i3) = value; }

    void set_item(int i1, int i2, int i3, int i4, T value) { xt::view(_array, i1, i2, i3, i4) = value; }

    void set_item(int i1, int i2, int i3, int i4, int i5, T value) { xt::view(_array, i1, i2, i3, i4, i5) = value; }

    // Boolean Functions
    bool all() const { return xt::all(_array); }

    bool any() const { return xt::any(_array); }

    // Aggregate Functions
    T sum() const { return (xt::sum(_array))[0]; }

    ndarray<T> sum(int axis) const {
        xt::xarray<T> result = xt::sum(_array, {axis});
        return ndarray<T>(result);
    }

    ndarray<T> sum(const _ShapeLike& axis) const {
        xt::xarray<T> result = xt::sum(_array, axis);
        return ndarray<T>(result);
    }

    T prod() const { return (xt::prod(_array))[0]; }

    ndarray<T> prod(int axis) const {
        xt::xarray<T> result = xt::prod(_array, {axis});
        return ndarray<T>(result);
    }

    ndarray<T> prod(const _ShapeLike& axes) const {
        xt::xarray<T> result = xt::prod(_array, axes);
        return ndarray<T>(result);
    }

    T min() const { return (xt::amin(_array))[0]; }

    ndarray<T> min(int axis) const {
        xt::xarray<T> result = xt::amin(_array, {axis});
        return ndarray<T>(result);
    }

    ndarray<T> min(const _ShapeLike& axes) const {
        xt::xarray<T> result = xt::amin(_array, axes);
        return ndarray<T>(result);
    }

    T max() const { return (xt::amax(_array))[0]; }

    ndarray<T> max(int axis) const {
        xt::xarray<T> result = xt::amax(_array, {axis});
        return ndarray<T>(result);
    }

    ndarray<T> max(const _ShapeLike& axes) const {
        xt::xarray<T> result = xt::amax(_array, axes);
        return ndarray<T>(result);
    }

    pkpy::float64 mean() const { return (xt::mean(_array))[0]; }

    ndarray<pkpy::float64> mean(int axis) const {
        return ndarray<pkpy::float64>(xt::mean(_array, {axis}));
    }

    ndarray<pkpy::float64> mean(const _ShapeLike& axes) const {
        return ndarray<pkpy::float64>(xt::mean(_array, axes));
    }

    pkpy::float64 std() const { return (xt::stddev(_array))[0]; }

    ndarray<pkpy::float64> std(int axis) const {
        return ndarray<pkpy::float64>(xt::stddev(_array, {axis}));
    }

    ndarray<pkpy::float64> std(const _ShapeLike& axes) const {
        return ndarray<pkpy::float64>(xt::stddev(_array, axes));
    }

    pkpy::float64 var() const { return (xt::variance(_array))[0]; }

    ndarray<pkpy::float64> var(int axis) const {
        return ndarray<pkpy::float64>(xt::variance(_array, {axis}));
    }

    ndarray<pkpy::float64> var(const _ShapeLike& axes) const {
        return ndarray<pkpy::float64>(xt::variance(_array, axes));
    }

    // Searching and Sorting Functions
    pkpy::int64 argmin() const { return (xt::argmin(_array))[0]; }

    ndarray<T> argmin(int axis) const {
        xt::xarray<T> result = xt::argmin(_array, {axis});
        return ndarray<T>(result);
    }

    pkpy::int64 argmax() const { return (xt::argmax(_array))[0]; }

    ndarray<T> argmax(int axis) const {
        xt::xarray<T> result = xt::argmax(_array, {axis});
        return ndarray<T>(result);
    }

    ndarray<T> argsort() const { return ndarray<T>(xt::argsort(_array)); }

    ndarray<T> argsort(int axis) const {
        xt::xarray<T> result = xt::argsort(_array, {axis});
        return ndarray<T>(result);
    }

    ndarray<T> sort() const { return ndarray<T>(xt::sort(_array)); }

    ndarray<T> sort(int axis) const {
        xt::xarray<T> result = xt::sort(_array, {axis});
        return ndarray<T>(result);
    }

    // Shape Manipulation Functions
    ndarray<T> reshape(const _ShapeLike& shape) const {
        xt::xarray<T> dummy = _array;
        dummy.reshape(shape);
        return ndarray<T>(dummy);
    }

    // Does not preserve elements if expected size is not equal to the current size.
    // https://github.com/xtensor-stack/xtensor/issues/1445
    ndarray<T> resize(const _ShapeLike& shape) const {
        xt::xarray<T> dummy = _array;
        dummy.resize(shape);
        return ndarray<T>(dummy);
    }

    ndarray<T> squeeze() const { return ndarray<T>(xt::squeeze(_array)); }

    ndarray<T> squeeze(int axis) const {
        xt::xarray<T> result = xt::squeeze(_array, {axis});
        return ndarray<T>(result);
    }

    ndarray<T> transpose() const { return ndarray<T>(xt::transpose(_array)); }

    ndarray<T> transpose(const _ShapeLike& permutation) const { return ndarray<T>(xt::transpose(_array, permutation)); }

    template <typename... Args>
    ndarray<T> transpose(Args... args) const {
        xt::xarray<T> result = xt::transpose(_array, {args...});
        return ndarray<T>(result);
    }

    ndarray<T> repeat(int repeats, int axis) const { return ndarray<T>(xt::repeat(_array, repeats, axis)); }

    ndarray<T> repeat(const std::vector<size_t>& repeats, int axis) const {
        return ndarray<T>(xt::repeat(_array, repeats, axis));
    }

    ndarray<T> flatten() const { return ndarray<T>(xt::flatten(_array)); }

    // Miscellaneous Functions
    ndarray<T> round() const { return ndarray<T>(xt::round(_array)); }

    template <typename U>
    ndarray<U> astype() const {
        xt::xarray<U> result = xt::cast<U>(_array);
        return ndarray<U>(result);
    }

    ndarray<T> copy() const {
        ndarray<T> result = *this;
        return result;
    }

    std::vector<T> to_list() const {
        std::vector<T> vec;
        for(auto &it : _array) {
            vec.push_back(it);
        }
        return vec;
    }

private:
    xt::xarray<T> _array;
};

class random {
public:
    random() {
        auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        xt::random::seed(static_cast<xt::random::seed_type>(seed));
    }

    template <typename T>
    static T rand() {
        random random_instance;
        return (xt::random::rand<T>(std::vector{1}))[0];
    }

    template <typename T>
    static ndarray<T> rand(const _ShapeLike& shape) {
        random random_instance;
        return ndarray<T>(xt::random::rand<T>(shape));
    }

    template <typename T>
    static T randn() {
        random random_instance;
        return (xt::random::randn<T>(std::vector{1}))[0];
    }

    template <typename T>
    static ndarray<T> randn(const _ShapeLike& shape) {
        random random_instance;
        return ndarray<T>(xt::random::randn<T>(shape));
    }

    template <typename T>
    static int randint(T low, T high) {
        random random_instance;
        return (xt::random::randint<T>(std::vector{1}, low, high))[0];
    }

    template <typename T>
    static ndarray<T> randint(T low, T high, const _ShapeLike& shape) {
        random random_instance;
        return ndarray<T>(xt::random::randint<T>(shape, low, high));
    }

    template <typename T>
    static ndarray<T> uniform(T low, T high, const _ShapeLike& shape) {
        random random_instance;
        return ndarray<T>(xt::random::rand<T>(shape, low, high));
    }
};
template<typename T, typename U>
xt::xarray<std::common_type_t<T, U>> matrix_mul(const xt::xarray<T>& a, const xt::xarray<U>& b) {
    using result_type = std::common_type_t<T, U>;
    using Mat = xt::xarray<result_type>;
    
    bool first_is_1d = false;
    bool second_is_1d = false;

    xt::xarray<T> a_copy = a;
    xt::xarray<U> b_copy = b;

    if (a.dimension() == 1) {
        first_is_1d = true;
        a_copy = xt::reshape_view(a_copy, {1, 3});
    }
    if(b_copy.dimension() == 1) {
        second_is_1d = true;
        b_copy = xt::reshape_view(b_copy, {3, 1});
    }
    if (a_copy.dimension() == 2 && b_copy.dimension() == 2) {
        int m = static_cast<int>(a_copy.shape()[0]);
        int n = static_cast<int>(a_copy.shape()[1]);
        int p = static_cast<int>(b_copy.shape()[1]);

        Mat result = xt::zeros<result_type>({m, p});

        for (int i = 0; i < m; i++) {
            for (int j = 0; j < p; j++) {
                for (int k = 0; k < n; k++) {
                    result(i, j) = result(i, j) + a_copy(i, k) * b_copy(k, j);
                }
            }
        }

        if (first_is_1d) {
            result = xt::squeeze(result, std::vector<std::size_t>{result.dimension()-2});
        }
        if (second_is_1d) {
            result = xt::squeeze(result, std::vector<std::size_t>{result.dimension()-1});
        }

        return result;
    }
    else {
        if (a_copy.dimension() == b_copy.dimension()) {
            assert(a_copy.shape()[0] == b_copy.shape()[0]);
            size_t layers = a_copy.shape()[0];
            
            Mat sub;
            {
                Mat a0 = xt::view(a_copy, 0);
                Mat b0 = xt::view(b_copy, 0);
                sub = matrix_mul(a0, b0);
            }

            auto out_shape = sub.shape();
            out_shape.insert(out_shape.begin(), layers);
            auto result = Mat::from_shape(out_shape);
            xt::view(result, 0) = sub;

            for (size_t i = 1; i < layers; i++) {
                Mat ai = xt::view(a_copy, i);
                Mat bi = xt::view(b_copy, i);
                xt::view(result, i) = matrix_mul(ai, bi);
            }

            if (first_is_1d) {
                result = xt::squeeze(result, std::vector<std::size_t>{result.dimension()-2});
            }
            if (second_is_1d) {
                result = xt::squeeze(result, std::vector<std::size_t>{result.dimension()-1});
            }

            return result;
        } else if (a_copy.dimension() > b_copy.dimension()) {
            assert(a_copy.dimension() > b_copy.dimension());
            size_t layers = a_copy.shape()[0];
                
            Mat sub;
            {
                Mat a0 = xt::view(a_copy, 0);
                sub = matrix_mul(a0, b_copy);
            }

            auto out_shape = sub.shape();
            out_shape.insert(out_shape.begin(), layers);
            auto result = Mat::from_shape(out_shape);
            xt::view(result, 0) = sub;

            for (size_t i = 1; i < layers; i++) {
                Mat ai = xt::view(a_copy, i);
                xt::view(result, i) = matrix_mul(ai, b_copy);
            }

            if (first_is_1d) {
                result = xt::squeeze(result, std::vector<std::size_t>{result.dimension()-2});
            }
            if (second_is_1d) {
                result = xt::squeeze(result, std::vector<std::size_t>{result.dimension()-1});
            }

            return result;
        } else {
            assert(a_copy.dimension() < b_copy.dimension());
            size_t layers = b_copy.shape()[0];
                
            Mat sub;
            {
                Mat b0 = xt::view(b_copy, 0);
                sub = matrix_mul(a_copy, b0);
            }

            auto out_shape = sub.shape();
            out_shape.insert(out_shape.begin(), layers);
            auto result = Mat::from_shape(out_shape);
            xt::view(result, 0) = sub;

            for (size_t i = 1; i < layers; i++) {
                Mat bi = xt::view(b_copy, i);
                xt::view(result, i) = matrix_mul(a_copy, bi);
            }

            if (first_is_1d) {
                result = xt::squeeze(result, std::vector<std::size_t>{result.dimension()-2});
            }
            if (second_is_1d) {
                result = xt::squeeze(result, std::vector<std::size_t>{result.dimension()-1});
            }

            return result;
        }
    }
}
    
template <typename T, typename U>
ndarray<std::common_type_t<T, U>> matmul(const ndarray<T>& a, const ndarray<U>& b) {
    return ndarray<std::common_type_t<T, U>>(matrix_mul(a.get_array(), b.get_array()));
}

template <typename T>
ndarray<T> adapt(const std::vector<T>& init_list) {
    return ndarray<T>(xt::adapt(init_list));
}

template <typename T>
ndarray<T> adapt(const std::vector<std::vector<T>>& init_list) {
    std::vector<T> flat_list;
    for(auto row: init_list) {
        for(auto elem: row) {
            flat_list.push_back(elem);
        }
    }
    std::vector<size_t> sh = {init_list.size(), init_list[0].size()};
    return ndarray<T>(xt::adapt(flat_list, sh));
}

template <typename T>
ndarray<T> adapt(const std::vector<std::vector<std::vector<T>>>& init_list) {
    std::vector<T> flat_list;
    for(auto row: init_list) {
        for(auto elem: row) {
            for(auto val: elem) {
                flat_list.push_back(val);
            }
        }
    }
    std::vector<size_t> sh = {init_list.size(), init_list[0].size(), init_list[0][0].size()};
    return ndarray<T>(xt::adapt(flat_list, sh));
}

template <typename T>
ndarray<T> adapt(const std::vector<std::vector<std::vector<std::vector<T>>>>& init_list) {
    std::vector<T> flat_list;
    for(auto row: init_list) {
        for(auto elem: row) {
            for(auto val: elem) {
                for(auto v: val) {
                    flat_list.push_back(v);
                }
            }
        }
    }
    std::vector<size_t> sh = {init_list.size(), init_list[0].size(), init_list[0][0].size(), init_list[0][0][0].size()};
    return ndarray<T>(xt::adapt(flat_list, sh));
}

template <typename T>
ndarray<T> adapt(const std::vector<std::vector<std::vector<std::vector<std::vector<T>>>>>& init_list) {
    std::vector<T> flat_list;
    for(auto row: init_list) {
        for(auto elem: row) {
            for(auto val: elem) {
                for(auto v: val) {
                    for(auto v1: v) {
                        flat_list.push_back(v1);
                    }
                }
            }
        }
    }
    std::vector<size_t> sh = {init_list.size(),
                              init_list[0].size(),
                              init_list[0][0].size(),
                              init_list[0][0][0].size(),
                              init_list[0][0][0][0].size()};
    return ndarray<T>(xt::adapt(flat_list, sh));
}

// Array Creation
template <typename U, typename T>
ndarray<U> array(const std::vector<T>& vec, const _ShapeLike& shape = {}) {
    if(shape.empty()) {
        return ndarray<U>(xt::cast<U>(xt::adapt(vec)));
    } else {
        return ndarray<U>(xt::cast<U>(xt::adapt(vec, shape)));
    }
}

template <typename T>
ndarray<T> zeros(const _ShapeLike& shape) {
    return ndarray<T>(xt::zeros<T>(shape));
}

template <typename T>
ndarray<T> ones(const _ShapeLike& shape) {
    return ndarray<T>(xt::ones<T>(shape));
}

template <typename T>
ndarray<T> full(const _ShapeLike& shape, const T& fill_value) {
    xt::xarray<T> result = xt::ones<T>(shape);
    for(auto it = result.begin(); it != result.end(); ++it) {
        *it = fill_value;
    }
    return ndarray<T>(result);
}

template <typename T>
ndarray<T> identity(int n) {
    return ndarray<T>(xt::eye<T>(n));
}

template <typename T>
ndarray<T> arange(const T& stop) {
    return ndarray<T>(xt::arange<T>(stop));
}

template <typename T>
ndarray<T> arange(const T& start, const T& stop) {
    return ndarray<T>(xt::arange<T>(start, stop));
}

template <typename T>
ndarray<T> arange(const T& start, const T& stop, const T& step) {
    return ndarray<T>(xt::arange<T>(start, stop, step));
}

template <typename T>
ndarray<T> linspace(const T& start, const T& stop, int num = 50, bool endpoint = true) {
    return ndarray<T>(xt::linspace<T>(start, stop, num, endpoint));
}

// Trigonometry
template <typename T>
ndarray<float_> sin(const ndarray<T>& arr) {
    return ndarray<float_>(xt::sin(arr.get_array()));
}

ndarray<complex_> sin(const ndarray<complex64>& arr) { return ndarray<complex_>(xt::sin(arr.get_array())); }

ndarray<complex_> sin(const ndarray<complex128>& arr) { return ndarray<complex_>(xt::sin(arr.get_array())); }

template <typename T>
ndarray<float_> cos(const ndarray<T>& arr) {
    return ndarray<float_>(xt::cos(arr.get_array()));
}

ndarray<complex_> cos(const ndarray<complex64>& arr) { return ndarray<complex_>(xt::cos(arr.get_array())); }

ndarray<complex_> cos(const ndarray<complex128>& arr) { return ndarray<complex_>(xt::cos(arr.get_array())); }

template <typename T>
ndarray<float_> tan(const ndarray<T>& arr) {
    return ndarray<float_>(xt::tan(arr.get_array()));
}

ndarray<complex_> tan(const ndarray<complex64>& arr) { return ndarray<complex_>(xt::tan(arr.get_array())); }

ndarray<complex_> tan(const ndarray<complex128>& arr) { return ndarray<complex_>(xt::tan(arr.get_array())); }

template <typename T>
ndarray<float_> arcsin(const ndarray<T>& arr) {
    return ndarray<float_>(xt::asin(arr.get_array()));
}

ndarray<complex_> arcsin(const ndarray<complex64>& arr) { return ndarray<complex_>(xt::asin(arr.get_array())); }

ndarray<complex_> arcsin(const ndarray<complex128>& arr) { return ndarray<complex_>(xt::asin(arr.get_array())); }

template <typename T>
ndarray<float_> arccos(const ndarray<T>& arr) {
    return ndarray<float_>(xt::acos(arr.get_array()));
}

ndarray<complex_> arccos(const ndarray<complex64>& arr) { return ndarray<complex_>(xt::acos(arr.get_array())); }

ndarray<complex_> arccos(const ndarray<complex128>& arr) { return ndarray<complex_>(xt::acos(arr.get_array())); }

template <typename T>
ndarray<float_> arctan(const ndarray<T>& arr) {
    return ndarray<float_>(xt::atan(arr.get_array()));
}

ndarray<complex_> arctan(const ndarray<complex64>& arr) { return ndarray<complex_>(xt::atan(arr.get_array())); }

ndarray<complex_> arctan(const ndarray<complex128>& arr) { return ndarray<complex_>(xt::atan(arr.get_array())); }

// Exponents and Logarithms
template <typename T>
ndarray<float_> exp(const ndarray<T>& arr) {
    return ndarray<float_>(xt::exp(arr.get_array()));
}

ndarray<complex_> exp(const ndarray<complex64>& arr) { return ndarray<complex_>(xt::exp(arr.get_array())); }

ndarray<complex_> exp(const ndarray<complex128>& arr) { return ndarray<complex_>(xt::exp(arr.get_array())); }

template <typename T>
ndarray<float_> log(const ndarray<T>& arr) {
    return ndarray<float_>(xt::log(arr.get_array()));
}

ndarray<complex_> log(const ndarray<complex64>& arr) { return ndarray<complex_>(xt::log(arr.get_array())); }

ndarray<complex_> log(const ndarray<complex128>& arr) { return ndarray<complex_>(xt::log(arr.get_array())); }

template <typename T>
ndarray<float_> log2(const ndarray<T>& arr) {
    return ndarray<float_>(xt::log2(arr.get_array()));
}

template <typename T>
ndarray<float_> log10(const ndarray<T>& arr) {
    return ndarray<float_>(xt::log10(arr.get_array()));
}

// Miscellanous
template <typename T>
ndarray<T> round(const ndarray<T>& arr) {
    return ndarray<T>(xt::round(arr.get_array()));
}

template <typename T>
ndarray<T> floor(const ndarray<T>& arr) {
    return ndarray<T>(xt::floor(arr.get_array()));
}

template <typename T>
ndarray<T> ceil(const ndarray<T>& arr) {
    return ndarray<T>(xt::ceil(arr.get_array()));
}

template <typename T>
auto abs(const ndarray<T>& arr) {
    if constexpr(std::is_same_v<T, complex64> || std::is_same_v<T, complex128>) {
        return ndarray<float_>(xt::abs(arr.get_array()));
    } else {
        return ndarray<T>(xt::abs(arr.get_array()));
    }
}

// Xtensor only supports concatenation of initialized objects.
// https://github.com/xtensor-stack/xtensor/issues/1450
template <typename T, typename U>
auto concatenate(const ndarray<T>& arr1, const ndarray<U>& arr2, int axis = 0) {
    using result_type = std::common_type_t<T, U>;
    xt::xarray<result_type> xarr1 = xt::cast<result_type>(arr1.get_array());
    xt::xarray<result_type> xarr2 = xt::cast<result_type>(arr2.get_array());
    return ndarray<result_type>(xt::concatenate(xt::xtuple(xarr1, xarr2), axis));
}

// Constants
constexpr float_ pi = xt::numeric_constants<double>::PI;
constexpr double inf = std::numeric_limits<double>::infinity();

// Testing Functions
template <typename T, typename U>
bool allclose(const ndarray<T>& arr1, const ndarray<U>& arr2, float_ rtol = 1e-5, float_ atol = 1e-8) {
    return xt::allclose(arr1.get_array(), arr2.get_array(), rtol, atol);
}

// Reverse Dunder Methods
template <typename T, typename U, typename = std::enable_if_t<!is_ndarray_v<U>>>
auto operator+ (const U& scalar, const ndarray<T>& array) {
    xt::xarray<T> arr = array.get_array();
    if constexpr(std::is_same_v<U, float_>) {
        xt::xarray<float_> result = scalar + xt::cast<float_>(arr);
        return ndarray<float_>(result);
    } else {
        using result_type = std::common_type_t<T, U>;
        xt::xarray<result_type> result = scalar + xt::cast<result_type>(arr);
        return ndarray<result_type>(result);
    }
}

template <typename T, typename U, typename = std::enable_if_t<!is_ndarray_v<U>>>
auto operator- (const U& scalar, const ndarray<T>& array) {
    xt::xarray<T> arr = array.get_array();
    if constexpr(std::is_same_v<U, float_>) {
        xt::xarray<float_> result = scalar - xt::cast<float_>(arr);
        return ndarray<float_>(result);
    } else {
        using result_type = std::common_type_t<T, U>;
        xt::xarray<result_type> result = scalar - xt::cast<result_type>(arr);
        return ndarray<result_type>(result);
    }
}

template <typename T, typename U, typename = std::enable_if_t<!is_ndarray_v<U>>>
auto operator* (const U& scalar, const ndarray<T>& array) {
    xt::xarray<T> arr = array.get_array();
    if constexpr(std::is_same_v<U, float_>) {
        xt::xarray<float_> result = scalar * xt::cast<float_>(arr);
        return ndarray<float_>(result);
    } else {
        using result_type = std::common_type_t<T, U>;
        xt::xarray<result_type> result = scalar * xt::cast<result_type>(arr);
        return ndarray<result_type>(result);
    }
}

template <typename T, typename U, typename = std::enable_if_t<!is_ndarray_v<U>>>
auto operator/ (const U& scalar, const ndarray<T>& array) {
    xt::xarray<T> arr = array.get_array();
    xt::xarray<float_> result = static_cast<float_>(scalar) / xt::cast<float_>(arr);
    return ndarray<float_>(result);
}

template <typename T, typename U, typename = std::enable_if_t<!is_ndarray_v<U>>>
auto pow(const U& scalar, const ndarray<T>& array) {
    xt::xarray<T> arr = array.get_array();
    xt::xarray<float_> result = xt::pow(scalar, xt::cast<float_>(arr));
    return ndarray<float_>(result);
}

template <typename T, typename U, typename = std::enable_if_t<!is_ndarray_v<U>>>
auto operator& (const U& scalar, const ndarray<T>& array) {
    return array & scalar;
}

template <typename T, typename U, typename = std::enable_if_t<!is_ndarray_v<U>>>
auto operator| (const U& scalar, const ndarray<T>& array) {
    return array | scalar;
}

template <typename T, typename U, typename = std::enable_if_t<!is_ndarray_v<U>>>
auto operator^ (const U& scalar, const ndarray<T>& array) {
    return array ^ scalar;
}

template <typename T>
std::ostream& operator<< (std::ostream& os, const ndarray<T>& arr) {
    os << arr.get_array();
    return os;
}

}  // namespace numpy
}  // namespace pkpy
