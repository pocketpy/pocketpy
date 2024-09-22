#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <numpy.hpp>
#include <typeinfo>

namespace py = pybind11;

using bool_ = pkpy::bool_;
using int8 = pkpy::int8;
using int16 = pkpy::int16;
using int32 = pkpy::int32;
using int64 = pkpy::int64;
using int_ = pkpy::int_;
using float32 = pkpy::float32;
using float64 = pkpy::float64;
using float_ = pkpy::float_;

// Function to parse attributes
int parseAttr(const py::object& obj) {
    if(py::isinstance<py::none>(obj)) {
        return INT_MAX;
    } else if(py::isinstance<py::int_>(obj)) {
        return obj.cast<int>();
    } else {
        throw std::runtime_error("Unsupported type");
    }
};

class ndarray_base {
public:
    virtual ~ndarray_base() = default;

    virtual int ndim() const = 0;

    virtual int size() const = 0;

    virtual std::string dtype() const = 0;

    virtual py::tuple shape() const = 0;

    virtual bool all() const = 0;

    virtual bool any() const = 0;

    virtual py::object sum() const = 0;

    virtual py::object sum_axis(int axis) const = 0;

    virtual py::object sum_axes(py::tuple axes) const = 0;

    virtual py::object prod() const = 0;

    virtual py::object prod_axis(int axis) const = 0;

    virtual py::object prod_axes(py::tuple axes) const = 0;

    virtual py::object min() const = 0;

    virtual py::object min_axis(int axis) const = 0;

    virtual py::object min_axes(py::tuple axes) const = 0;

    virtual py::object max() const = 0;

    virtual py::object max_axis(int axis) const = 0;

    virtual py::object max_axes(py::tuple axes) const = 0;

    virtual py::object mean() const = 0;

    virtual py::object mean_axis(int axis) const = 0;

    virtual py::object mean_axes(py::tuple axes) const = 0;

    virtual py::object std() const = 0;

    virtual py::object std_axis(int axis) const = 0;

    virtual py::object std_axes(py::tuple axes) const = 0;

    virtual py::object var() const = 0;

    virtual py::object var_axis(int axis) const = 0;

    virtual py::object var_axes(py::tuple axes) const = 0;

    virtual py::object argmin() const = 0;

    virtual ndarray_base* argmin_axis(int axis) const = 0;

    virtual py::object argmax() const = 0;

    virtual ndarray_base* argmax_axis(int axis) const = 0;

    virtual ndarray_base* argsort() const = 0;

    virtual ndarray_base* argsort_axis(int axis) const = 0;

    virtual void sort() = 0;

    virtual void sort_axis(int axis) = 0;

    virtual ndarray_base* reshape(const std::vector<int>& shape) const = 0;

    virtual void resize(const std::vector<int>& shape) = 0;

    virtual ndarray_base* squeeze() const = 0;

    virtual ndarray_base* squeeze_axis(int axis) const = 0;

    virtual ndarray_base* transpose() const = 0;

    virtual ndarray_base* transpose_tuple(py::tuple permutations) const = 0;

    virtual ndarray_base* transpose_args(py::args args) const = 0;

    virtual ndarray_base* repeat(int repeats, int axis) const = 0;

    virtual ndarray_base* repeat_axis(const std::vector<size_t>& repeats, int axis) const = 0;

    virtual ndarray_base* round() const = 0;

    virtual ndarray_base* flatten() const = 0;

    virtual ndarray_base* copy() const = 0;

    virtual ndarray_base* astype(const std::string& dtype) const = 0;

    virtual py::list tolist() const = 0;

    virtual ndarray_base* eq(const ndarray_base& other) const = 0;

    virtual ndarray_base* ne(const ndarray_base& other) const = 0;

    virtual ndarray_base* add(const ndarray_base& other) const = 0;

    virtual ndarray_base* add_bool(bool_ other) const = 0;

    virtual ndarray_base* add_int(int_ other) const = 0;

    virtual ndarray_base* add_float(float64 other) const = 0;

    virtual ndarray_base* sub(const ndarray_base& other) const = 0;

    virtual ndarray_base* sub_int(int_ other) const = 0;

    virtual ndarray_base* sub_float(float64 other) const = 0;

    virtual ndarray_base* rsub_int(int_ other) const = 0;

    virtual ndarray_base* rsub_float(float64 other) const = 0;

    virtual ndarray_base* mul(const ndarray_base& other) const = 0;

    virtual ndarray_base* mul_bool(bool_ other) const = 0;

    virtual ndarray_base* mul_int(int_ other) const = 0;

    virtual ndarray_base* mul_float(float64 other) const = 0;

    virtual ndarray_base* div(const ndarray_base& other) const = 0;

    virtual ndarray_base* div_bool(bool_ other) const = 0;

    virtual ndarray_base* div_int(int_ other) const = 0;

    virtual ndarray_base* div_float(float64 other) const = 0;

    virtual ndarray_base* rdiv_bool(bool_ other) const = 0;

    virtual ndarray_base* rdiv_int(int_ other) const = 0;

    virtual ndarray_base* rdiv_float(float64 other) const = 0;

    virtual ndarray_base* matmul(const ndarray_base& other) const = 0;

    virtual ndarray_base* pow(const ndarray_base& other) const = 0;

    virtual ndarray_base* pow_int(int_ other) const = 0;

    virtual ndarray_base* pow_float(float64 other) const = 0;

    virtual ndarray_base* rpow_int(int_ other) const = 0;

    virtual ndarray_base* rpow_float(float64 other) const = 0;

    virtual ndarray_base* and_array(const ndarray_base& other) const = 0;

    virtual ndarray_base* and_bool(bool_ other) const = 0;

    virtual ndarray_base* and_int(int_ other) const = 0;

    virtual ndarray_base* or_array(const ndarray_base& other) const = 0;

    virtual ndarray_base* or_bool(bool_ other) const = 0;

    virtual ndarray_base* or_int(int_ other) const = 0;

    virtual ndarray_base* xor_array(const ndarray_base& other) const = 0;

    virtual ndarray_base* xor_bool(bool_ other) const = 0;

    virtual ndarray_base* xor_int(int_ other) const = 0;

    virtual ndarray_base* invert() const = 0;

    virtual py::object get_item_int(int index) const = 0;

    virtual py::object get_item_tuple(py::tuple indices) const = 0;

    virtual ndarray_base* get_item_vector(const std::vector<int>& indices) const = 0;

    virtual ndarray_base* get_item_slice(py::slice slice) const = 0;

    virtual void set_item_int(int index, int_ value) = 0;

    virtual void set_item_index_int(int index, const std::vector<int_>& value) = 0;

    virtual void set_item_index_int_2d(int index, const std::vector<std::vector<int_>>& value) = 0;

    virtual void set_item_index_int_3d(int index, const std::vector<std::vector<std::vector<int_>>>& value) = 0;

    virtual void set_item_index_int_4d(int index, const std::vector<std::vector<std::vector<std::vector<int_>>>>& value) = 0;

    virtual void set_item_float(int index, float64 value) = 0;

    virtual void set_item_index_float(int index, const std::vector<float64>& value) = 0;

    virtual void set_item_index_float_2d(int index, const std::vector<std::vector<float64>>& value) = 0;

    virtual void set_item_index_float_3d(int index, const std::vector<std::vector<std::vector<float64>>>& value) = 0;

    virtual void set_item_index_float_4d(int index, const std::vector<std::vector<std::vector<std::vector<float64>>>>& value) = 0;

    virtual void set_item_tuple_int1(py::tuple args, int_ value) = 0;

    virtual void set_item_tuple_int2(py::tuple args, const std::vector<int_>& value) = 0;

    virtual void set_item_tuple_int3(py::tuple args, const std::vector<std::vector<int_>>& value) = 0;

    virtual void set_item_tuple_int4(py::tuple args, const std::vector<std::vector<std::vector<int_>>>& value) = 0;

    virtual void set_item_tuple_int5(py::tuple args, const std::vector<std::vector<std::vector<std::vector<int_>>>>& value) = 0;

    virtual void set_item_tuple_float1(py::tuple args, float64 value) = 0;

    virtual void set_item_tuple_float2(py::tuple args, const std::vector<float64>& value) = 0;

    virtual void set_item_tuple_float3(py::tuple args, const std::vector<std::vector<float64>>& value) = 0;

    virtual void set_item_tuple_float4(py::tuple args, const std::vector<std::vector<std::vector<float64>>>& value) = 0;

    virtual void set_item_tuple_float5(py::tuple args, const std::vector<std::vector<std::vector<std::vector<float64>>>>& value) = 0;

    virtual void set_item_vector_int1(const std::vector<int>& indices, int_ value) = 0;

    virtual void set_item_vector_int2(const std::vector<int>& indices, const std::vector<int_>& value) = 0;

    virtual void set_item_vector_int3(const std::vector<int>& indices, const std::vector<std::vector<int_>>& value) = 0;

    virtual void set_item_vector_int4(const std::vector<int>& indices, const std::vector<std::vector<std::vector<int_>>>& value) = 0;

    virtual void set_item_vector_int5(const std::vector<int>& indices, const std::vector<std::vector<std::vector<std::vector<int_>>>>& value) = 0;

    virtual void set_item_vector_float1(const std::vector<int>& indices, float64 value) = 0;

    virtual void set_item_vector_float2(const std::vector<int>& indices, const std::vector<float64>& value) = 0;

    virtual void set_item_vector_float3(const std::vector<int>& indices, const std::vector<std::vector<float64>>& value) = 0;

    virtual void set_item_vector_float4(const std::vector<int>& indices, const std::vector<std::vector<std::vector<float64>>>& value) = 0;

    virtual void set_item_vector_float5(const std::vector<int>& indices, const std::vector<std::vector<std::vector<std::vector<float64>>>>& value) = 0;

    virtual void set_item_slice_int1(py::slice slice, int_ value) = 0;

    virtual void set_item_slice_int2(py::slice slice, const std::vector<int_>& value) = 0;

    virtual void set_item_slice_int3(py::slice slice, const std::vector<std::vector<int_>>& value) = 0;

    virtual void set_item_slice_int4(py::slice slice, const std::vector<std::vector<std::vector<int_>>>& value) = 0;

    virtual void set_item_slice_int5(py::slice slice, const std::vector<std::vector<std::vector<std::vector<int_>>>>& value) = 0;

    virtual void set_item_slice_float1(py::slice slice, float64 value) = 0;

    virtual void set_item_slice_float2(py::slice slice, const std::vector<float64>& value) = 0;

    virtual void set_item_slice_float3(py::slice slice, const std::vector<std::vector<float64>>& value) = 0;

    virtual void set_item_slice_float4(py::slice slice, const std::vector<std::vector<std::vector<float64>>>& value) = 0;

    virtual void set_item_slice_float5(py::slice slice, const std::vector<std::vector<std::vector<std::vector<float64>>>>& value) = 0;

    virtual int len() const = 0;

    virtual std::string to_string() const = 0;
};

template <typename T>
class ndarray : public ndarray_base {
public:
    pkpy::numpy::ndarray<T> data;
    // Constructors
    ndarray() = default;

    ndarray(const bool_ value) : data(value) {}

    ndarray(const int8 value) : data(value) {}

    ndarray(const int16 value) : data(value) {}

    ndarray(const int32 value) : data(value) {}

    ndarray(const int_ value) : data(static_cast<T>(value)) {}

    ndarray(const float32 value) : data(value) {}

    ndarray(const float64 value) : data(static_cast<T>(value)) {}

    ndarray(const pkpy::numpy::ndarray<T>& _arr) : data(_arr) {}

    ndarray(const std::vector<T>& init_list) : data(pkpy::numpy::adapt<T>(init_list)) {}

    ndarray(const std::vector<std::vector<T>>& init_list) : data(pkpy::numpy::adapt<T>(init_list)) {}

    ndarray(const std::vector<std::vector<std::vector<T>>>& init_list) : data(pkpy::numpy::adapt<T>(init_list)) {}

    ndarray(const std::vector<std::vector<std::vector<std::vector<T>>>>& init_list) :
        data(pkpy::numpy::adapt<T>(init_list)) {}

    ndarray(const std::vector<std::vector<std::vector<std::vector<std::vector<T>>>>>& init_list) :
        data(pkpy::numpy::adapt<T>(init_list)) {}

    // Properties
    int ndim() const override { return data.ndim(); }

    int size() const override { return data.size(); }

    std::string dtype() const override { return data.dtype(); }

    py::tuple shape() const override { return py::cast(data.shape()); }

    // Boolean Functions
    bool all() const override { return data.all(); }

    bool any() const override { return data.any(); }

    // Aggregation Functions
    py::object sum() const override {
        if constexpr (std::is_same_v<T, bool_> || std::is_same_v<T, int8> || std::is_same_v<T, int16> ||
                      std::is_same_v<T, int32> || std::is_same_v<T, int64>) {
            return py::int_(data.sum());
        } else if constexpr(std::is_same_v<T, float32> || std::is_same_v<T, float64>) {
            return py::float_(data.sum());
        } else {
            throw std::runtime_error("Unsupported type");
        }
    }

    py::object sum_axis(int axis) const override {
        if ((data.sum(axis)).ndim() == 0) {
            return py::cast((data.sum(axis))());
        } else {
            return py::cast(ndarray<T>(data.sum(axis)));
        }
    }

    py::object sum_axes(py::tuple axes) const override {
        std::vector<int> axes_;
        for(auto item: axes) {
            axes_.push_back(py::cast<int>(item));
        }
        if ((data.sum(axes_)).ndim() == 0) {
            return py::cast((data.sum(axes_))());
        } else {
            return py::cast(ndarray<T>(data.sum(axes_)));
        }
    }

    py::object prod() const override {
        if constexpr (std::is_same_v<T, bool_> || std::is_same_v<T, int8> || std::is_same_v<T, int16> ||
                      std::is_same_v<T, int32> || std::is_same_v<T, int64>) {
            return py::int_(data.prod());
        } else if constexpr(std::is_same_v<T, float32> || std::is_same_v<T, float64>) {
            return py::float_(data.prod());
        } else {
            throw std::runtime_error("Unsupported type");
        }
    }

    py::object prod_axis(int axis) const override {
        if ((data.prod(axis)).ndim() == 0) {
            return py::cast((data.prod(axis))());
        } else {
            return py::cast(ndarray<T>(data.prod(axis)));
        }
    }

    py::object prod_axes(py::tuple axes) const override {
        std::vector<int> axes_;
        for(auto item: axes) {
            axes_.push_back(py::cast<int>(item));
        }
        if ((data.prod(axes_)).ndim() == 0) {
            return py::cast((data.prod(axes_))());
        } else {
            return py::cast(ndarray<T>(data.prod(axes_)));
        }
    }

    py::object min() const override {
        if constexpr (std::is_same_v<T, bool_>) {
            return py::bool_(data.min());
        } else if constexpr (std::is_same_v<T, int8> || std::is_same_v<T, int16> ||
                      std::is_same_v<T, int32> || std::is_same_v<T, int64>) {
            return py::int_(data.min());
        } else if constexpr(std::is_same_v<T, float32> || std::is_same_v<T, float64>) {
            return py::float_(data.min());
        } else {
            throw std::runtime_error("Unsupported type");
        }
    }

    py::object min_axis(int axis) const override {
        if ((data.min(axis)).ndim() == 0) {
            return py::cast((data.min(axis))());
        } else {
            return py::cast(ndarray<T>(data.min(axis)));
        }

    }

    py::object min_axes(py::tuple axes) const override {
        std::vector<int> axes_;
        for(auto item: axes) {
            axes_.push_back(py::cast<int>(item));
        }
        if ((data.min(axes_)).ndim() == 0) {
            return py::cast((data.min(axes_))());
        } else {
            return py::cast(ndarray<T>(data.min(axes_)));
        }
    }

    py::object max() const override {
        if constexpr (std::is_same_v<T, bool_>) {
            return py::bool_(data.max());
        } else if constexpr (std::is_same_v<T, int8> || std::is_same_v<T, int16> ||
                             std::is_same_v<T, int32> || std::is_same_v<T, int64>) {
            return py::int_(data.max());
        } else if constexpr(std::is_same_v<T, float32> || std::is_same_v<T, float64>) {
            return py::float_(data.max());
        } else {
            throw std::runtime_error("Unsupported type");
        }
    }

    py::object max_axis(int axis) const override {
        if ((data.max(axis)).ndim() == 0) {
            return py::cast((data.max(axis))());
        } else {
            return py::cast(ndarray<T>(data.max(axis)));
        }
    }

    py::object max_axes(py::tuple axes) const override {
        std::vector<int> axes_;
        for(auto item: axes) {
            axes_.push_back(py::cast<int>(item));
        }
        if ((data.max(axes_)).ndim() == 0) {
            return py::cast((data.max(axes_))());
        } else {
            return py::cast(ndarray<T>(data.max(axes_)));
        }
    }

    py::object mean() const override {
        if constexpr (std::is_same_v<T, bool_> || std::is_same_v<T, int8> || std::is_same_v<T, int16> ||
                      std::is_same_v<T, int32> || std::is_same_v<T, int64> || std::is_same_v<T, float32> ||
                        std::is_same_v<T, float64>) {
            return py::float_(data.mean());
        } else {
            throw std::runtime_error("Unsupported type");
        }
    }

    py::object mean_axis(int axis) const override {
        if ((data.mean(axis)).ndim() == 0) {
            return py::cast((data.mean(axis))());
        } else {
            return py::cast(ndarray<float64>(data.mean(axis)));
        }
    }

    py::object mean_axes(py::tuple axes) const override {
        std::vector<int> axes_;
        for(auto item: axes)
            axes_.push_back(py::cast<int>(item));
        if ((data.mean(axes_)).ndim() == 0) {
            return py::cast((data.mean(axes_))());
        } else {
            return py::cast(ndarray<float64>(data.mean(axes_)));
        }
    }

    py::object std() const override {
        if constexpr (std::is_same_v<T, bool_> || std::is_same_v<T, int8> || std::is_same_v<T, int16> ||
                      std::is_same_v<T, int32> || std::is_same_v<T, int64> || std::is_same_v<T, float32> ||
                        std::is_same_v<T, float64>) {
            return py::float_(data.std());
        } else {
            throw std::runtime_error("Unsupported type");
        }
    }

    py::object std_axis(int axis) const override {
        if ((data.std(axis)).ndim() == 0) {
            return py::cast((data.std(axis))());
        } else {
            return py::cast(ndarray<float64>(data.std(axis)));
        }
    }

    py::object std_axes(py::tuple axes) const override {
        std::vector<int> axes_;
        for(auto item: axes)
            axes_.push_back(py::cast<int>(item));
        if ((data.std(axes_)).ndim() == 0) {
            return py::cast((data.std(axes_))());
        } else {
            return py::cast(ndarray<float64>(data.std(axes_)));
        }
    }

    py::object var() const override {
        if constexpr (std::is_same_v<T, bool_> || std::is_same_v<T, int8> || std::is_same_v<T, int16> ||
                      std::is_same_v<T, int32> || std::is_same_v<T, int64> || std::is_same_v<T, float32> ||
                        std::is_same_v<T, float64>) {
            return py::float_(data.var());
        } else {
            throw std::runtime_error("Unsupported type");
        }
    }

    py::object var_axis(int axis) const override {
        if ((data.var(axis)).ndim() == 0) {
            return py::cast((data.var(axis))());
        } else {
            return py::cast(ndarray<float64>(data.var(axis)));
        }
    }

    py::object var_axes(py::tuple axes) const override {
        std::vector<int> axes_;
        for(auto item: axes)
            axes_.push_back(py::cast<int>(item));
        if ((data.var(axes_)).ndim() == 0) {
            return py::cast((data.var(axes_))());
        } else {
            return py::cast(ndarray<float64>(data.var(axes_)));
        }
    }

    py::object argmin() const override {
        if constexpr (std::is_same_v<T, bool_> || std::is_same_v<T, int8> || std::is_same_v<T, int16> ||
                      std::is_same_v<T, int32> || std::is_same_v<T, int64>) {
            return py::int_(data.argmin());
        } else if constexpr(std::is_same_v<T, float32> || std::is_same_v<T, float64>) {
            return py::int_(data.argmin());
        } else {
            throw std::runtime_error("Unsupported type");
        }
    }

    ndarray_base* argmin_axis(int axis) const override { return new ndarray<T>(data.argmin(axis)); }

    py::object argmax() const override {
        if constexpr (std::is_same_v<T, bool_> || std::is_same_v<T, int8> || std::is_same_v<T, int16> ||
                      std::is_same_v<T, int32> || std::is_same_v<T, int64>) {
            return py::int_(data.argmax());
        } else if constexpr(std::is_same_v<T, float32> || std::is_same_v<T, float64>) {
            return py::int_(data.argmax());
        } else {
            throw std::runtime_error("Unsupported type");
        }
    }

    ndarray_base* argmax_axis(int axis) const override { return new ndarray<T>(data.argmax(axis)); }

    ndarray_base* argsort() const override { return new ndarray<T>(data.argsort()); }

    ndarray_base* argsort_axis(int axis) const override { return new ndarray<T>(data.argsort(axis)); }

    void sort() override { data = data.sort(); }

    void sort_axis(int axis) override { data = data.sort(axis); }

    ndarray_base* reshape(const std::vector<int>& shape) const override { return new ndarray<T>(data.reshape(shape)); }

    void resize(const std::vector<int>& shape) override { data = data.resize(shape); }

    ndarray_base* squeeze() const override { return new ndarray<T>(data.squeeze()); }

    ndarray_base* squeeze_axis(int axis) const override { return new ndarray<T>(data.squeeze(axis)); }

    ndarray_base* transpose() const override { return new ndarray<T>(data.transpose()); }

    ndarray_base* transpose_tuple(py::tuple permutations) const override {
        std::vector<int> perm;
        for(auto item: permutations)
            perm.push_back(py::cast<int>(item));
        return new ndarray<T>(data.transpose(perm));
    }

    ndarray_base* transpose_args(py::args args) const override {
        std::vector<int> perm;
        for(auto item: args)
            perm.push_back(py::cast<int>(item));
        return new ndarray<T>(data.transpose(perm));
    }

    ndarray_base* repeat(int repeats, int axis) const override {
        if (axis == INT_MAX) {
            return new ndarray<T>(data.repeat(repeats, data.ndim() - 1));
        }
        return new ndarray<T>(data.repeat(repeats, axis));
    }

    ndarray_base* repeat_axis(const std::vector<size_t>& repeats, int axis) const override {
        return new ndarray<T>(data.repeat(repeats, axis));
    }

    ndarray_base* round() const override { return new ndarray<T>(data.round()); }

    ndarray_base* flatten() const override { return new ndarray<T>(data.flatten()); }

    ndarray_base* copy() const override { return new ndarray<T>(data.copy()); }

    ndarray_base* astype(const std::string& dtype) const override {
        if(dtype == "bool_") {
            return new ndarray<bool_>(data.template astype<bool_>());
        } else if(dtype == "int8") {
            return new ndarray<int8>(data.template astype<int8>());
        } else if(dtype == "int16") {
            return new ndarray<int16>(data.template astype<int16>());
        } else if(dtype == "int32") {
            return new ndarray<int32>(data.template astype<int32>());
        } else if(dtype == "int_") {
            return new ndarray<int_>(data.template astype<int_>());
        } else if(dtype == "float32") {
            return new ndarray<float32>(data.template astype<float32>());
        } else if(dtype == "float64") {
            return new ndarray<float64>(data.template astype<float64>());
        } else {
            throw std::invalid_argument("Invalid dtype");
        }
    }

    py::list tolist() const override {
        py::list list;
        if(data.ndim() == 1) {
            return py::cast(data.to_list());
        } else {
            for(int i = 0; i < data.shape()[0]; i++) {
                list.append(ndarray<T>(data[i]).tolist());
            }
        }
        return list;
    }

    // Dunder Methods
    ndarray_base* eq(const ndarray_base& other) const override {
        if constexpr(std::is_same_v<T, int8>) {
            if(auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* int8 == int8 */
                return new ndarray<bool_>(data == p->data);
            } else if(auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* int8 == int16 */
                return new ndarray<bool_>(data == p->data);
            } else if(auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* int8 == int32 */
                return new ndarray<bool_>(data == p->data);
            } else if(auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* int8 == int64 */
                return new ndarray<bool_>(data == p->data);
            } else if(auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* int8 == float32 */
                return new ndarray<bool_>(data == p->data);
            } else if(auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* int8 == float64 */
                return new ndarray<bool_>(data == p->data);
            }
        } else if constexpr(std::is_same_v<T, int16>) {
            if(auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* int16 == int8 */
                return new ndarray<bool_>(data == p->data);
            } else if(auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* int16 == int16 */
                return new ndarray<bool_>(data == p->data);
            } else if(auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* int16 == int32 */
                return new ndarray<bool_>(data == p->data);
            } else if(auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* int16 == int64 */
                return new ndarray<bool_>(data == p->data);
            } else if(auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* int16 == float32 */
                return new ndarray<bool_>(data == p->data);
            } else if(auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* int16 == float64 */
                return new ndarray<bool_>(data == p->data);
            }
        } else if constexpr(std::is_same_v<T, int32>) {
            if(auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* int32 == int8 */
                return new ndarray<bool_>(data == p->data);
            } else if(auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* int32 == int16 */
                return new ndarray<bool_>(data == p->data);
            } else if(auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* int32 == int32 */
                return new ndarray<bool_>(data == p->data);
            } else if(auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* int32 == int64 */
                return new ndarray<bool_>(data == p->data);
            } else if(auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* int32 == float32 */
                return new ndarray<bool_>(data == p->data);
            } else if(auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* int32 == float64 */
                return new ndarray<bool_>(data == p->data);
            }
        } else if constexpr(std::is_same_v<T, int_>) {
            if(auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* int64 == int8 */
                return new ndarray<bool_>(data == p->data);
            } else if(auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* int64 == int16 */
                return new ndarray<bool_>(data == p->data);
            } else if(auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* int64 == int32 */
                return new ndarray<bool_>(data == p->data);
            } else if(auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* int64 == int64 */
                return new ndarray<bool_>(data == p->data);
            } else if(auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* int64 == float32 */
                return new ndarray<bool_>(data == p->data);
            } else if(auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* int64 == float64 */
                return new ndarray<bool_>(data == p->data);
            }
        } else if constexpr(std::is_same_v<T, float32>) {
            if(auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* float32 == int8 */
                return new ndarray<bool_>(data == p->data);
            } else if(auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* float32 == int16 */
                return new ndarray<bool_>(data == p->data);
            } else if(auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* float32 == int32 */
                return new ndarray<bool_>(data == p->data);
            } else if(auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* float32 == int64 */
                return new ndarray<bool_>(data == p->data);
            } else if(auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* float32 == float32 */
                return new ndarray<bool_>(data == p->data);
            } else if(auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* float32 == float64 */
                return new ndarray<bool_>(data == p->data);
            }
        } else if constexpr(std::is_same_v<T, float64>) {
            if(auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* float64 == int8 */
                return new ndarray<bool_>(data == p->data);
            } else if(auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* float64 == int16 */
                return new ndarray<bool_>(data == p->data);
            } else if(auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* float64 == int32 */
                return new ndarray<bool_>(data == p->data);
            } else if(auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* float64 == int64 */
                return new ndarray<bool_>(data == p->data);
            } else if(auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* float64 == float32 */
                return new ndarray<bool_>(data == p->data);
            } else if(auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* float64 == float64 */
                return new ndarray<bool_>(data == p->data);
            }
        }

        const ndarray<T>& other_ = dynamic_cast<const ndarray<T>&>(other);
        return new ndarray<bool_>(data == other_.data);
    }

    ndarray_base* ne(const ndarray_base& other) const override {
        if constexpr(std::is_same_v<T, int8>) {
            if(auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* int8 != int8 */
                return new ndarray<bool_>(data != p->data);
            } else if(auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* int8 != int16 */
                return new ndarray<bool_>(data != p->data);
            } else if(auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* int8 != int32 */
                return new ndarray<bool_>(data != p->data);
            } else if(auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* int8 != int64 */
                return new ndarray<bool_>(data != p->data);
            } else if(auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* int8 != float32 */
                return new ndarray<bool_>(data != p->data);
            } else if(auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* int8 != float64 */
                return new ndarray<bool_>(data != p->data);
            }
        } else if constexpr(std::is_same_v<T, int16>) {
            if(auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* int16 != int8 */
                return new ndarray<bool_>(data != p->data);
            } else if(auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* int16 != int16 */
                return new ndarray<bool_>(data != p->data);
            } else if(auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* int16 != int32 */
                return new ndarray<bool_>(data != p->data);
            } else if(auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* int16 != int64 */
                return new ndarray<bool_>(data != p->data);
            } else if(auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* int16 != float32 */
                return new ndarray<bool_>(data != p->data);
            } else if(auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* int16 != float64 */
                return new ndarray<bool_>(data != p->data);
            }
        } else if constexpr(std::is_same_v<T, int32>) {
            if(auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* int32 != int8 */
                return new ndarray<bool_>(data != p->data);
            } else if(auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* int32 != int16 */
                return new ndarray<bool_>(data != p->data);
            } else if(auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* int32 != int32 */
                return new ndarray<bool_>(data != p->data);
            } else if(auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* int32 != int64 */
                return new ndarray<bool_>(data != p->data);
            } else if(auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* int32 != float32 */
                return new ndarray<bool_>(data != p->data);
            } else if(auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* int32 != float64 */
                return new ndarray<bool_>(data != p->data);
            }
        } else if constexpr(std::is_same_v<T, int_>) {
            if(auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* int64 != int8 */
                return new ndarray<bool_>(data != p->data);
            } else if(auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* int64 != int16 */
                return new ndarray<bool_>(data != p->data);
            } else if(auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* int64 != int32 */
                return new ndarray<bool_>(data != p->data);
            } else if(auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* int64 != int64 */
                return new ndarray<bool_>(data != p->data);
            } else if(auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* int64 != float32 */
                return new ndarray<bool_>(data != p->data);
            } else if(auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* int64 != float64 */
                return new ndarray<bool_>(data != p->data);
            }
        } else if constexpr(std::is_same_v<T, float32>) {
            if(auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* float32 != int8 */
                return new ndarray<bool_>(data != p->data);
            } else if(auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* float32 != int16 */
                return new ndarray<bool_>(data != p->data);
            } else if(auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* float32 != int32 */
                return new ndarray<bool_>(data != p->data);
            } else if(auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* float32 != int64 */
                return new ndarray<bool_>(data != p->data);
            } else if(auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* float32 != float32 */
                return new ndarray<bool_>(data != p->data);
            } else if(auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* float32 != float64 */
                return new ndarray<bool_>(data != p->data);
            }
        } else if constexpr(std::is_same_v<T, float64>) {
            if(auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* float64 != int8 */
                return new ndarray<bool_>(data != p->data);
            } else if(auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* float64 != int16 */
                return new ndarray<bool_>(data != p->data);
            } else if(auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* float64 != int32 */
                return new ndarray<bool_>(data != p->data);
            } else if(auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* float64 != int64 */
                return new ndarray<bool_>(data != p->data);
            } else if(auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* float64 != float32 */
                return new ndarray<bool_>(data != p->data);
            } else if(auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* float64 != float64 */
                return new ndarray<bool_>(data != p->data);
            }
        }

        const ndarray<T>& other_ = dynamic_cast<const ndarray<T>&>(other);
        return new ndarray<bool_>(data != other_.data);
    }

    ndarray_base* add(const ndarray_base& other) const override {
        if constexpr(std::is_same_v<T, int8>) {
            if(auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* int8 + int8 */
                return new ndarray<int8>(data + p->data);
            } else if(auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* int8 + int16 */
                return new ndarray<int16>((data + p->data).template astype<int16>());
            } else if(auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* int8 + int32 */
                return new ndarray<int32>(data + p->data);
            } else if(auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* int8 + int64 */
                return new ndarray<int_>(data + p->data);
            } else if(auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* int8 + float32 */
                return new ndarray<float32>(data + p->data);
            } else if(auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* int8 + float64 */
                return new ndarray<float64>(data + p->data);
            }
        } else if constexpr(std::is_same_v<T, int16>) {
            if(auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* int16 + int8 */
                return new ndarray<int16>((data + p->data).template astype<int16>());
            } else if(auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* int16 + int16 */
                return new ndarray<int16>(data + p->data);
            } else if(auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* int16 + int32 */
                return new ndarray<int32>(data + p->data);
            } else if(auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* int16 + int64 */
                return new ndarray<int_>(data + p->data);
            } else if(auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* int16 + float32 */
                return new ndarray<float32>(data + p->data);
            } else if(auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* int16 + float64 */
                return new ndarray<float64>(data + p->data);
            }
        } else if constexpr(std::is_same_v<T, int32>) {
            if(auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* int32 + int8 */
                return new ndarray<int32>(data + p->data);
            } else if(auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* int32 + int16 */
                return new ndarray<int32>(data + p->data);
            } else if(auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* int32 + int32 */
                return new ndarray<int32>(data + p->data);
            } else if(auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* int32 + int64 */
                return new ndarray<int_>(data + p->data);
            } else if(auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* int32 + float32 */
                return new ndarray<float32>(data + p->data);
            } else if(auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* int32 + float64 */
                return new ndarray<float64>(data + p->data);
            }
        } else if constexpr(std::is_same_v<T, int_>) {
            if (auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* int64 + int8 */
                return new ndarray<int_>(data + p->data);
            } else if (auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* int64 + int16 */
                return new ndarray<int_>(data + p->data);
            } else if (auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* int64 + int32 */
                return new ndarray<int_>(data + p->data);
            } else if (auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* int64 + int64 */
                return new ndarray<int_>(data + p->data);
            } else if (auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* int64 + float32 */
                return new ndarray<float32>(data + p->data);
            } else if (auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* int64 + float64 */
                return new ndarray<float64>(data + p->data);
            }
        } else if constexpr(std::is_same_v<T, float32>) {
            if (auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* float32 + int8 */
                return new ndarray<float32>(data + p->data);
            } else if (auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* float32 + int16 */
                return new ndarray<float32>(data + p->data);
            } else if (auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* float32 + int32 */
                return new ndarray<float32>(data + p->data);
            } else if (auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* float32 + int64 */
                return new ndarray<float32>(data + p->data);
            } else if (auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* float32 + float32 */
                return new ndarray<float32>(data + p->data);
            } else if (auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* float32 + float64 */
                return new ndarray<float64>(data + p->data);
            }
        } else if constexpr(std::is_same_v<T, float64>) {
            if (auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* float64 + int8 */
                return new ndarray<float64>(data + p->data);
            } else if (auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* float64 + int16 */
                return new ndarray<float64>(data + p->data);
            } else if (auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* float64 + int32 */
                return new ndarray<float64>(data + p->data);
            } else if (auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* float64 + int64 */
                return new ndarray<float64>(data + p->data);
            } else if (auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* float64 + float32 */
                return new ndarray<float64>(data + p->data);
            } else if (auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* float64 + float64 */
                return new ndarray<float64>(data + p->data);
            }
        }

        const ndarray<T>& other_ = dynamic_cast<const ndarray<T>&>(other);
        return new ndarray<T>(data + other_.data);
    }

    ndarray_base* add_bool(bool_ other) const override {
        if constexpr(std::is_same_v<T, int8>) {
            return new ndarray<int8>((data + other).template astype<int8>());
        } else if constexpr(std::is_same_v<T, int16>) {
            return new ndarray<int16>((data + other).template astype<int16>());
        } else {
            return new ndarray<T>(data + other);
        }
    }

    ndarray_base* add_int(int_ other) const override {
        if constexpr(std::is_same_v<T, bool_>) {
            return new ndarray<int_>(data + other);
        } else if constexpr(std::is_same_v<T, int8>) {
            return new ndarray<int8>((data + other).template astype<int8>());
        } else if constexpr(std::is_same_v<T, int16>) {
            return new ndarray<int16>((data + other).template astype<int16>());
        } else if constexpr(std::is_same_v<T, int32>) {
            return new ndarray<int32>((data + other).template astype<int32>());
        } else if constexpr(std::is_same_v<T, int_>) {
            return new ndarray<int64>(data + other);
        } else if constexpr(std::is_same_v<T, float32>) {
            return new ndarray<float32>(data + other);
        } else if constexpr(std::is_same_v<T, float64>) {
            return new ndarray<float64>(data + other);
        }
    }

    ndarray_base* add_float(float64 other) const override { return new ndarray<float64>(data + other); }

    ndarray_base* sub(const ndarray_base& other) const override {
        if constexpr(std::is_same_v<T, int8>) {
            if(auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* int8 - int8 */
                return new ndarray<int8>(data - p->data);
            } else if(auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* int8 - int16 */
                return new ndarray<int16>((data - p->data).template astype<int16>());
            } else if(auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* int8 - int32 */
                return new ndarray<int32>(data - p->data);
            } else if(auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* int8 - int64 */
                return new ndarray<int_>(data - p->data);
            } else if(auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* int8 - float32 */
                return new ndarray<float32>(data - p->data);
            } else if(auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* int8 - float64 */
                return new ndarray<float64>(data - p->data);
            }
        } else if constexpr(std::is_same_v<T, int16>) {
            if(auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* int16 - int8 */
                return new ndarray<int16>((data - p->data).template astype<int16>());
            } else if(auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* int16 - int16 */
                return new ndarray<int16>(data - p->data);
            } else if(auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* int16 - int32 */
                return new ndarray<int32>(data - p->data);
            } else if(auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* int16 - int64 */
                return new ndarray<int_>(data - p->data);
            } else if(auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* int16 - float32 */
                return new ndarray<float32>(data - p->data);
            } else if(auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* int16 - float64 */
                return new ndarray<float64>(data - p->data);
            }
        } else if constexpr(std::is_same_v<T, int32>) {
            if(auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* int32 - int8 */
                return new ndarray<int32>(data - p->data);
            } else if(auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* int32 - int16 */
                return new ndarray<int32>(data - p->data);
            } else if(auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* int32 - int32 */
                return new ndarray<int32>(data - p->data);
            } else if(auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* int32 - int64 */
                return new ndarray<int_>(data - p->data);
            } else if(auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* int32 - float32 */
                return new ndarray<float32>(data - p->data);
            } else if(auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* int32 - float64 */
                return new ndarray<float64>(data - p->data);
            }
        } else if constexpr(std::is_same_v<T, int_>) {
            if (auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* int64 - int8 */
                return new ndarray<int_>(data - p->data);
            } else if (auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* int64 - int16 */
                return new ndarray<int_>(data - p->data);
            } else if (auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* int64 - int32 */
                return new ndarray<int_>(data - p->data);
            } else if (auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* int64 - int64 */
                return new ndarray<int_>(data - p->data);
            } else if (auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* int64 - float32 */
                return new ndarray<float32>(data - p->data);
            } else if (auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* int64 - float64 */
                return new ndarray<float64>(data - p->data);
            }
        } else if constexpr(std::is_same_v<T, float32>) {
            if (auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* float32 - int8 */
                return new ndarray<float32>(data - p->data);
            } else if (auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* float32 - int16 */
                return new ndarray<float32>(data - p->data);
            } else if (auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* float32 - int32 */
                return new ndarray<float32>(data - p->data);
            } else if (auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* float32 - int64 */
                return new ndarray<float32>(data - p->data);
            } else if (auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* float32 - float32 */
                return new ndarray<float32>(data - p->data);
            } else if (auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* float32 - float64 */
                return new ndarray<float64>(data - p->data);
            }
        } else if constexpr(std::is_same_v<T, float64>) {
            if (auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* float64 - int8 */
                return new ndarray<float64>(data - p->data);
            } else if (auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* float64 - int16 */
                return new ndarray<float64>(data - p->data);
            } else if (auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* float64 - int32 */
                return new ndarray<float64>(data - p->data);
            } else if (auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* float64 - int64 */
                return new ndarray<float64>(data - p->data);
            } else if (auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* float64 - float32 */
                return new ndarray<float64>(data - p->data);
            } else if (auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* float64 - float64 */
                return new ndarray<float64>(data - p->data);
            }
        }

        const ndarray<T>& other_ = dynamic_cast<const ndarray<T>&>(other);
        return new ndarray<T>(data - other_.data);
    }

    ndarray_base* sub_int(int_ other) const override {
        if constexpr(std::is_same_v<T, bool_>) {
            return new ndarray<int_>(data - other);
        } else if constexpr(std::is_same_v<T, int8>) {
            return new ndarray<int8>((data - other).template astype<int8>());
        } else if constexpr(std::is_same_v<T, int16>) {
            return new ndarray<int16>((data - other).template astype<int16>());
        } else if constexpr(std::is_same_v<T, int32>) {
            return new ndarray<int32>((data - other).template astype<int32>());
        } else if constexpr(std::is_same_v<T, int_>) {
            return new ndarray<int64>(data - other);
        } else if constexpr(std::is_same_v<T, float32>) {
            return new ndarray<float32>(data - other);
        } else if constexpr(std::is_same_v<T, float64>) {
            return new ndarray<float64>(data - other);
        }
    }

    ndarray_base* sub_float(float64 other) const override { return new ndarray<float64>(data - other); }

    ndarray_base* rsub_int(int_ other) const override {
        if constexpr(std::is_same_v<T, bool_>) {
            return new ndarray<int_>(other - data);
        } else if constexpr(std::is_same_v<T, int8>) {
            return new ndarray<int8>((other - data).template astype<int8>());
        } else if constexpr(std::is_same_v<T, int16>) {
            return new ndarray<int16>((other - data).template astype<int16>());
        } else if constexpr(std::is_same_v<T, int32>) {
            return new ndarray<int32>((other - data).template astype<int32>());
        } else if constexpr(std::is_same_v<T, int_>) {
            return new ndarray<int64>(other - data);
        } else if constexpr(std::is_same_v<T, float32>) {
            return new ndarray<float32>(other - data);
        } else if constexpr(std::is_same_v<T, float64>) {
            return new ndarray<float64>(other - data);
        }
    }

    ndarray_base* rsub_float(float64 other) const override { return new ndarray<float64>(other - data); }

    ndarray_base* mul(const ndarray_base& other) const override {
        if constexpr(std::is_same_v<T, int8>) {
            if(auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* int8 * int8 */
                return new ndarray<int8>(data * p->data);
            } else if(auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* int8 * int16 */
                return new ndarray<int16>((data * p->data).template astype<int16>());
            } else if(auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* int8 * int32 */
                return new ndarray<int32>(data * p->data);
            } else if(auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* int8 * int64 */
                return new ndarray<int_>(data * p->data);
            } else if(auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* int8 * float32 */
                return new ndarray<float32>(data * p->data);
            } else if(auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* int8 * float64 */
                return new ndarray<float64>(data * p->data);
            }
        } else if constexpr(std::is_same_v<T, int16>) {
            if(auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* int16 * int8 */
                return new ndarray<int16>((data * p->data).template astype<int16>());
            } else if(auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* int16 * int16 */
                return new ndarray<int16>(data * p->data);
            } else if(auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* int16 * int32 */
                return new ndarray<int32>(data * p->data);
            } else if(auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* int16 * int64 */
                return new ndarray<int_>(data * p->data);
            } else if(auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* int16 * float32 */
                return new ndarray<float32>(data * p->data);
            } else if(auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* int16 * float64 */
                return new ndarray<float64>(data * p->data);
            }
        } else if constexpr(std::is_same_v<T, int32>) {
            if(auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* int32 * int8 */
                return new ndarray<int32>(data * p->data);
            } else if(auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* int32 * int16 */
                return new ndarray<int32>(data * p->data);
            } else if(auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* int32 * int32 */
                return new ndarray<int32>(data * p->data);
            } else if(auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* int32 * int64 */
                return new ndarray<int_>(data * p->data);
            } else if(auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* int32 * float32 */
                return new ndarray<float32>(data * p->data);
            } else if(auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* int32 * float64 */
                return new ndarray<float64>(data * p->data);
            }
        } else if constexpr(std::is_same_v<T, int_>) {
            if (auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* int64 * int8 */
                return new ndarray<int_>(data * p->data);
            } else if (auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* int64 * int16 */
                return new ndarray<int_>(data * p->data);
            } else if (auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* int64 * int32 */
                return new ndarray<int_>(data * p->data);
            } else if (auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* int64 * int64 */
                return new ndarray<int_>(data * p->data);
            } else if (auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* int64 * float32 */
                return new ndarray<float32>(data * p->data);
            } else if (auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* int64 * float64 */
                return new ndarray<float64>(data * p->data);
            }
        } else if constexpr(std::is_same_v<T, float32>) {
            if (auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* float32 * int8 */
                return new ndarray<float32>(data * p->data);
            } else if (auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* float32 * int16 */
                return new ndarray<float32>(data * p->data);
            } else if (auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* float32 * int32 */
                return new ndarray<float32>(data * p->data);
            } else if (auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* float32 * int64 */
                return new ndarray<float32>(data * p->data);
            } else if (auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* float32 * float32 */
                return new ndarray<float32>(data * p->data);
            } else if (auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* float32 * float64 */
                return new ndarray<float64>(data * p->data);
            }
        } else if constexpr(std::is_same_v<T, float64>) {
            if (auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* float64 * int8 */
                return new ndarray<float64>(data * p->data);
            } else if (auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* float64 * int16 */
                return new ndarray<float64>(data * p->data);
            } else if (auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* float64 * int32 */
                return new ndarray<float64>(data * p->data);
            } else if (auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* float64 * int64 */
                return new ndarray<float64>(data * p->data);
            } else if (auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* float64 * float32 */
                return new ndarray<float64>(data * p->data);
            } else if (auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* float64 * float64 */
                return new ndarray<float64>(data * p->data);
            }
        }

        const ndarray<T>& other_ = dynamic_cast<const ndarray<T>&>(other);
        return new ndarray<T>(data * other_.data);
    }

    ndarray_base* mul_bool(bool_ other) const override {
        if constexpr(std::is_same_v<T, int8>) {
            return new ndarray<int8>((data * other).template astype<int8>());
        } else if constexpr(std::is_same_v<T, int16>) {
            return new ndarray<int16>((data * other).template astype<int16>());
        } else {
            return new ndarray<T>(data * other);
        }
    }

    ndarray_base* mul_int(int_ other) const override {
        if constexpr(std::is_same_v<T, bool_>) {
            return new ndarray<int_>(data * other);
        } else if constexpr(std::is_same_v<T, int8>) {
            return new ndarray<int8>((data * other).template astype<int8>());
        } else if constexpr(std::is_same_v<T, int16>) {
            return new ndarray<int16>((data * other).template astype<int16>());
        } else if constexpr(std::is_same_v<T, int32>) {
            return new ndarray<int32>((data * other).template astype<int32>());
        } else if constexpr(std::is_same_v<T, int_>) {
            return new ndarray<int64>(data * other);
        } else if constexpr(std::is_same_v<T, float32>) {
            return new ndarray<float32>(data * other);
        } else if constexpr(std::is_same_v<T, float64>) {
            return new ndarray<float64>(data * other);
        }
    }

    ndarray_base* mul_float(float64 other) const override { return new ndarray<float64>(data * other); }

    ndarray_base* div(const ndarray_base& other) const override {
        if constexpr(std::is_same_v<T, bool_>) {
            if(auto p = dynamic_cast<const ndarray<bool_>*>(&other)) { /* bool / bool */
                return new ndarray<float64>(data / p->data);
            } else if(auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* bool / int8 */
                return new ndarray<float64>(data / p->data);
            } else if(auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* bool / int16 */
                return new ndarray<float64>(data / p->data);
            } else if(auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* bool / int32 */
                return new ndarray<float64>(data / p->data);
            } else if(auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* bool / int64 */
                return new ndarray<float64>(data / p->data);
            } else if(auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* bool / float32 */
                return new ndarray<float64>(data / p->data);
            } else if(auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* bool / float64 */
                return new ndarray<float64>(data / p->data);
            }
        } else if constexpr(std::is_same_v<T, int8>) {
            if (auto p = dynamic_cast<const ndarray<bool_>*>(&other)) { /* int8 / bool */
                return new ndarray<float64>(data / p->data);
            } else if(auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* int8 / int8 */
                return new ndarray<int8>(data / p->data);
            } else if(auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* int8 / int16 */
                return new ndarray<int16>((data / p->data).template astype<int16>());
            } else if(auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* int8 / int32 */
                return new ndarray<int32>(data / p->data);
            } else if(auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* int8 / int64 */
                return new ndarray<int_>(data / p->data);
            } else if(auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* int8 / float32 */
                return new ndarray<float32>(data / p->data);
            } else if(auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* int8 / float64 */
                return new ndarray<float64>(data / p->data);
            }
        } else if constexpr(std::is_same_v<T, int16>) {
            if (auto p = dynamic_cast<const ndarray<bool_>*>(&other)) { /* int16 / bool */
                return new ndarray<float64>(data / p->data);
            } else if(auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* int16 / int8 */
                return new ndarray<int16>((data / p->data).template astype<int16>());
            } else if(auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* int16 / int16 */
                return new ndarray<int16>(data / p->data);
            } else if(auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* int16 / int32 */
                return new ndarray<int32>(data / p->data);
            } else if(auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* int16 / int64 */
                return new ndarray<int_>(data / p->data);
            } else if(auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* int16 / float32 */
                return new ndarray<float32>(data / p->data);
            } else if(auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* int16 / float64 */
                return new ndarray<float64>(data / p->data);
            }
        } else if constexpr(std::is_same_v<T, int32>) {
            if (auto p = dynamic_cast<const ndarray<bool_>*>(&other)) { /* int32 / bool */
                return new ndarray<float64>(data / p->data);
            } else if(auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* int32 / int8 */
                return new ndarray<int32>(data / p->data);
            } else if(auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* int32 / int16 */
                return new ndarray<int32>(data / p->data);
            } else if(auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* int32 / int32 */
                return new ndarray<int32>(data / p->data);
            } else if(auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* int32 / int64 */
                return new ndarray<int_>(data / p->data);
            } else if(auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* int32 / float32 */
                return new ndarray<float32>(data / p->data);
            } else if(auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* int32 / float64 */
                return new ndarray<float64>(data / p->data);
            }
        } else if constexpr(std::is_same_v<T, int_>) {
            if (auto p = dynamic_cast<const ndarray<bool_>*>(&other)) { /* int64 / bool */
                return new ndarray<float64>(data / p->data);
            } else if (auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* int64 / int8 */
                return new ndarray<int_>(data / p->data);
            } else if (auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* int64 / int16 */
                return new ndarray<int_>(data / p->data);
            } else if (auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* int64 / int32 */
                return new ndarray<int_>(data / p->data);
            } else if (auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* int64 / int64 */
                return new ndarray<int_>(data / p->data);
            } else if (auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* int64 / float32 */
                return new ndarray<float32>(data / p->data);
            } else if (auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* int64 / float64 */
                return new ndarray<float64>(data / p->data);
            }
        } else if constexpr(std::is_same_v<T, float32>) {
            if (auto p = dynamic_cast<const ndarray<bool_>*>(&other)) { /* float32 / bool */
                return new ndarray<float64>(data / p->data);
            } else if (auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* float32 / int8 */
                return new ndarray<float32>(data / p->data);
            } else if (auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* float32 / int16 */
                return new ndarray<float32>(data / p->data);
            } else if (auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* float32 / int32 */
                return new ndarray<float32>(data / p->data);
            } else if (auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* float32 / int64 */
                return new ndarray<float32>(data / p->data);
            } else if (auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* float32 / float32 */
                return new ndarray<float32>(data / p->data);
            } else if (auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* float32 / float64 */
                return new ndarray<float64>(data / p->data);
            }
        } else if constexpr(std::is_same_v<T, float64>) {
            if (auto p = dynamic_cast<const ndarray<bool_>*>(&other)) { /* float64 / bool */
                return new ndarray<float64>(data / p->data);
            } else if (auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* float64 / int8 */
                return new ndarray<float64>(data / p->data);
            } else if (auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* float64 / int16 */
                return new ndarray<float64>(data / p->data);
            } else if (auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* float64 / int32 */
                return new ndarray<float64>(data / p->data);
            } else if (auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* float64 / int64 */
                return new ndarray<float64>(data / p->data);
            } else if (auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* float64 / float32 */
                return new ndarray<float64>(data / p->data);
            } else if (auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* float64 / float64 */
                return new ndarray<float64>(data / p->data);
            }
        }

        const ndarray<float64>& other_ = dynamic_cast<const ndarray<float64>&>(other);
        return new ndarray<float64>(data / other_.data);
    }

    ndarray_base* div_bool(bool_ other) const override { return new ndarray<float64>(data / other); }

    ndarray_base* div_int(int_ other) const override { return new ndarray<float64>(data / other); }

    ndarray_base* div_float(float64 other) const override { return new ndarray<float64>(data / other); }

    ndarray_base* rdiv_bool(bool_ other) const override { return new ndarray<float64>(other / data); }

    ndarray_base* rdiv_int(int_ other) const override { return new ndarray<float64>(other / data); }

    ndarray_base* rdiv_float(float64 other) const override { return new ndarray<float64>(other / data); }

    ndarray_base* matmul(const ndarray_base& other) const override {
        if constexpr(std::is_same_v<T, int8>) {
            if(auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* int8 @ int8 */
                return new ndarray<int8>(pkpy::numpy::matmul(data, p->data));
            } else if(auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* int8 @ int16 */
                return new ndarray<int16>(pkpy::numpy::matmul(data, p->data).template astype<int16>());
            } else if(auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* int8 @ int32 */
                return new ndarray<int32>(pkpy::numpy::matmul(data, p->data));
            } else if(auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* int8 @ int64 */
                return new ndarray<int_>(pkpy::numpy::matmul(data, p->data));
            } else if(auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* int8 @ float32 */
                return new ndarray<float32>(pkpy::numpy::matmul(data, p->data));
            } else if(auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* int8 @ float64 */
                return new ndarray<float64>(pkpy::numpy::matmul(data, p->data));
            }
        } else if constexpr(std::is_same_v<T, int16>) {
            if(auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* int16 @ int8 */
                return new ndarray<int16>(pkpy::numpy::matmul(data, p->data).template astype<int16>());
            } else if(auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* int16 @ int16 */
                return new ndarray<int16>(pkpy::numpy::matmul(data, p->data));
            } else if(auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* int16 @ int32 */
                return new ndarray<int32>(pkpy::numpy::matmul(data, p->data));
            } else if(auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* int16 @ int64 */
                return new ndarray<int_>(pkpy::numpy::matmul(data, p->data));
            } else if(auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* int16 @ float32 */
                return new ndarray<float32>(pkpy::numpy::matmul(data, p->data));
            } else if(auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* int16 @ float64 */
                return new ndarray<float64>(pkpy::numpy::matmul(data, p->data));
            }
        } else if constexpr(std::is_same_v<T, int32>) {
            if(auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* int32 @ int8 */
                return new ndarray<int32>(pkpy::numpy::matmul(data, p->data));
            } else if(auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* int32 @ int16 */
                return new ndarray<int32>(pkpy::numpy::matmul(data, p->data));
            } else if(auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* int32 @ int32 */
                return new ndarray<int32>(pkpy::numpy::matmul(data, p->data));
            } else if(auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* int32 @ int64 */
                return new ndarray<int_>(pkpy::numpy::matmul(data, p->data));
            } else if(auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* int32 @ float32 */
                return new ndarray<float32>(pkpy::numpy::matmul(data, p->data));
            } else if(auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* int32 @ float64 */
                return new ndarray<float64>(pkpy::numpy::matmul(data, p->data));
            }
        } else if constexpr(std::is_same_v<T, int_>) {
            if (auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* int64 @ int8 */
                return new ndarray<int_>(pkpy::numpy::matmul(data, p->data));
            } else if (auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* int64 @ int16 */
                return new ndarray<int_>(pkpy::numpy::matmul(data, p->data));
            } else if (auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* int64 @ int32 */
                return new ndarray<int_>(pkpy::numpy::matmul(data, p->data));
            } else if (auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* int64 @ int64 */
                return new ndarray<int_>(pkpy::numpy::matmul(data, p->data));
            } else if (auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* int64 @ float32 */
                return new ndarray<float32>(pkpy::numpy::matmul(data, p->data));
            } else if (auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* int64 @ float64 */
                return new ndarray<float64>(pkpy::numpy::matmul(data, p->data));
            }
        } else if constexpr(std::is_same_v<T, float32>) {
            if (auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* float32 @ int8 */
                return new ndarray<float32>(pkpy::numpy::matmul(data, p->data));
            } else if (auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* float32 @ int16 */
                return new ndarray<float32>(pkpy::numpy::matmul(data, p->data));
            } else if (auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* float32 @ int32 */
                return new ndarray<float32>(pkpy::numpy::matmul(data, p->data));
            } else if (auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* float32 @ int64 */
                return new ndarray<float32>(pkpy::numpy::matmul(data, p->data));
            } else if (auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* float32 @ float32 */
                return new ndarray<float32>(pkpy::numpy::matmul(data, p->data));
            } else if (auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* float32 @ float64 */
                return new ndarray<float64>(pkpy::numpy::matmul(data, p->data));
            }
        } else if constexpr(std::is_same_v<T, float64>) {
            if (auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* float64 @ int8 */
                return new ndarray<float64>(pkpy::numpy::matmul(data, p->data));
            } else if (auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* float64 @ int16 */
                return new ndarray<float64>(pkpy::numpy::matmul(data, p->data));
            } else if (auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* float64 @ int32 */
                return new ndarray<float64>(pkpy::numpy::matmul(data, p->data));
            } else if (auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* float64 @ int64 */
                return new ndarray<float64>(pkpy::numpy::matmul(data, p->data));
            } else if (auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* float64 @ float32 */
                return new ndarray<float64>(pkpy::numpy::matmul(data, p->data));
            } else if (auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* float64 @ float64 */
                return new ndarray<float64>(pkpy::numpy::matmul(data, p->data));
            }
        }

        const ndarray<T>& other_ = dynamic_cast<const ndarray<T>&>(other);
        return new ndarray<T>(pkpy::numpy::matmul(data, other_.data));
    }

    ndarray_base* pow(const ndarray_base& other) const override {
        if constexpr(std::is_same_v<T, int8>) {
            if(auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* int8 ** int8 */
                return new ndarray<int8>(data.pow(p->data));
            } else if(auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* int8 ** int16 */
                return new ndarray<int16>(data.pow(p->data).template astype<int16>());
            } else if(auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* int8 ** int32 */
                return new ndarray<int32>(data.pow(p->data));
            } else if(auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* int8 ** int64 */
                return new ndarray<int_>(data.pow(p->data));
            } else if(auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* int8 ** float32 */
                return new ndarray<float32>(data.pow(p->data));
            } else if(auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* int8 ** float64 */
                return new ndarray<float64>(data.pow(p->data));
            }
        } else if constexpr(std::is_same_v<T, int16>) {
            if(auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* int16 ** int8 */
                return new ndarray<int16>(data.pow(p->data).template astype<int16>());
            } else if(auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* int16 ** int16 */
                return new ndarray<int16>(data.pow(p->data));
            } else if(auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* int16 ** int32 */
                return new ndarray<int32>(data.pow(p->data));
            } else if(auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* int16 ** int64 */
                return new ndarray<int_>(data.pow(p->data));
            } else if(auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* int16 ** float32 */
                return new ndarray<float32>(data.pow(p->data));
            } else if(auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* int16 ** float64 */
                return new ndarray<float64>(data.pow(p->data));
            }
        } else if constexpr(std::is_same_v<T, int32>) {
            if(auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* int32 ** int8 */
                return new ndarray<int32>(data.pow(p->data));
            } else if(auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* int32 ** int16 */
                return new ndarray<int32>(data.pow(p->data));
            } else if(auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* int32 ** int32 */
                return new ndarray<int32>(data.pow(p->data));
            } else if(auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* int32 ** int64 */
                return new ndarray<int_>(data.pow(p->data));
            } else if(auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* int32 ** float32 */
                return new ndarray<float32>(data.pow(p->data));
            } else if(auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* int32 ** float64 */
                return new ndarray<float64>(data.pow(p->data));
            }
        } else if constexpr(std::is_same_v<T, int_>) {
            if (auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* int64 ** int8 */
                return new ndarray<int_>(data.pow(p->data));
            } else if (auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* int64 ** int16 */
                return new ndarray<int_>(data.pow(p->data));
            } else if (auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* int64 ** int32 */
                return new ndarray<int_>(data.pow(p->data));
            } else if (auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* int64 ** int64 */
                return new ndarray<int_>(data.pow(p->data));
            } else if (auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* int64 ** float32 */
                return new ndarray<float32>(data.pow(p->data));
            } else if (auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* int64 ** float64 */
                return new ndarray<float64>(data.pow(p->data));
            }
        } else if constexpr(std::is_same_v<T, float32>) {
            if (auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* float32 ** int8 */
                return new ndarray<float32>(data.pow(p->data));
            } else if (auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* float32 ** int16 */
                return new ndarray<float32>(data.pow(p->data));
            } else if (auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* float32 ** int32 */
                return new ndarray<float32>(data.pow(p->data));
            } else if (auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* float32 ** int64 */
                return new ndarray<float32>(data.pow(p->data));
            } else if (auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* float32 ** float32 */
                return new ndarray<float32>(data.pow(p->data));
            } else if (auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* float32 ** float64 */
                return new ndarray<float64>(data.pow(p->data));
            }
        } else if constexpr(std::is_same_v<T, float64>) {
            if (auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* float64 ** int8 */
                return new ndarray<float64>(data.pow(p->data));
            } else if (auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* float64 ** int16 */
                return new ndarray<float64>(data.pow(p->data));
            } else if (auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* float64 ** int32 */
                return new ndarray<float64>(data.pow(p->data));
            } else if (auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* float64 ** int64 */
                return new ndarray<float64>(data.pow(p->data));
            } else if (auto p = dynamic_cast<const ndarray<float32>*>(&other)) { /* float64 ** float32 */
                return new ndarray<float64>(data.pow(p->data));
            } else if (auto p = dynamic_cast<const ndarray<float64>*>(&other)) { /* float64 ** float64 */
                return new ndarray<float64>(data.pow(p->data));
            }
        }

        const ndarray<T>& other_ = dynamic_cast<const ndarray<T>&>(other);
        return new ndarray<T>(data.pow(other_.data));
    }

    ndarray_base* pow_int(int_ other) const override { return new ndarray<float64>(data.pow(other)); }

    ndarray_base* pow_float(float64 other) const override { return new ndarray<float64>(data.pow(other)); }

    ndarray_base* rpow_int(int_ other) const override { return new ndarray<float64>(pkpy::numpy::pow(other, data)); }

    ndarray_base* rpow_float(float64 other) const override {
        return new ndarray<float64>(pkpy::numpy::pow(other, data));
    }

    int len() const override { return data.shape()[0]; }

    ndarray_base* and_array(const ndarray_base& other) const override {
        if constexpr(std::is_same_v<T, bool_>) {
            if(auto p = dynamic_cast<const ndarray<bool_>*>(&other)) { /* bool & bool */
                return new ndarray<bool_>(data & p->data);
            } else if(auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* bool & int8 */
                return new ndarray<int8>((data & p->data).template astype<int8>());
            } else if(auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* bool & int16 */
                return new ndarray<int16>((data & p->data).template astype<int16>());
            } else if(auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* bool & int32 */
                return new ndarray<int32>((data & p->data).template astype<int32>());
            } else if(auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* bool & int64 */
                return new ndarray<int_>((data & p->data).template astype<int_>());
            }
        } else if constexpr (std::is_same_v<T, int8>) {
            if(auto p = dynamic_cast<const ndarray<bool_>*>(&other)) { /* int8 & bool */
                return new ndarray<int8>(data & p->data);
            } else if(auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* int8 & int8 */
                return new ndarray<int8>(data & p->data);
            } else if(auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* int8 & int16 */
                return new ndarray<int16>((data & p->data).template astype<int16>());
            } else if(auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* int8 & int32 */
                return new ndarray<int32>((data & p->data).template astype<int32>());
            } else if(auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* int8 & int64 */
                return new ndarray<int_>((data & p->data).template astype<int_>());
            }
        } else if constexpr (std::is_same_v<T, int16>) {
            if(auto p = dynamic_cast<const ndarray<bool_>*>(&other)) { /* int16 & bool */
                return new ndarray<int16>(data & p->data);
            } else if(auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* int16 & int8 */
                return new ndarray<int16>(data & p->data);
            } else if(auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* int16 & int16 */
                return new ndarray<int16>(data & p->data);
            } else if(auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* int16 & int32 */
                return new ndarray<int32>((data & p->data).template astype<int32>());
            } else if(auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* int16 & int64 */
                return new ndarray<int_>((data & p->data).template astype<int_>());
            }
        } else if constexpr (std::is_same_v<T, int32>) {
            if(auto p = dynamic_cast<const ndarray<bool_>*>(&other)) { /* int32 & bool */
                return new ndarray<int32>(data & p->data);
            } else if(auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* int32 & int8 */
                return new ndarray<int32>(data & p->data);
            } else if(auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* int32 & int16 */
                return new ndarray<int32>(data & p->data);
            } else if(auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* int32 & int32 */
                return new ndarray<int32>(data & p->data);
            } else if(auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* int32 & int64 */
                return new ndarray<int_>((data & p->data).template astype<int_>());
            }
        } else if constexpr (std::is_same_v<T, int_>) {
            if (auto p = dynamic_cast<const ndarray<bool_> *>(&other)) { /* int64 & bool */
                return new ndarray<int_>(data & p->data);
            } else if (auto p = dynamic_cast<const ndarray<int8> *>(&other)) { /* int64 & int8 */
                return new ndarray<int_>(data & p->data);
            } else if (auto p = dynamic_cast<const ndarray<int16> *>(&other)) { /* int64 & int16 */
                return new ndarray<int_>(data & p->data);
            } else if (auto p = dynamic_cast<const ndarray<int32> *>(&other)) { /* int64 & int32 */
                return new ndarray<int_>(data & p->data);
            } else if (auto p = dynamic_cast<const ndarray<int_> *>(&other)) { /* int64 & int64 */
                return new ndarray<int_>(data & p->data);
            }
        }

        throw std::runtime_error("& operator is not compatible with floating types");
    }

    ndarray_base* and_bool(bool_ other) const override {
        if constexpr(std::is_same_v<T, bool_>) {
            return new ndarray<bool_>(data & other);
        } else if constexpr(std::is_same_v<T, int8>) {
            return new ndarray<int8>(data & other);
        } else if constexpr(std::is_same_v<T, int16>) {
            return new ndarray<int16>(data & other);
        } else if constexpr(std::is_same_v<T, int32>) {
            return new ndarray<int32>(data & other);
        } else if constexpr(std::is_same_v<T, int_>) {
            return new ndarray<int_>(data & other);
        }

        throw std::runtime_error("& operator is not compatible with floating types");
    }

    ndarray_base* and_int(int_ other) const override {
        if constexpr(std::is_same_v<T, bool_>) {
            return new ndarray<int_>((data & other).template astype<int_>());
        } else if constexpr(std::is_same_v<T, int8>) {
            return new ndarray<int8>(data & other);
        } else if constexpr(std::is_same_v<T, int16>) {
            return new ndarray<int16>(data & other);
        } else if constexpr(std::is_same_v<T, int32>) {
            return new ndarray<int32>(data & other);
        } else if constexpr(std::is_same_v<T, int_>) {
            return new ndarray<int_>(data & other);
        }

        throw std::runtime_error("& operator is not compatible with floating types");
    }

    ndarray_base* or_array(const ndarray_base& other) const override {
        if constexpr(std::is_same_v<T, bool_>) {
            if(auto p = dynamic_cast<const ndarray<bool_>*>(&other)) { /* bool | bool */
                return new ndarray<bool_>(data | p->data);
            } else if(auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* bool | int8 */
                return new ndarray<int8>((data | p->data).template astype<int8>());
            } else if(auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* bool | int16 */
                return new ndarray<int16>((data | p->data).template astype<int16>());
            } else if(auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* bool | int32 */
                return new ndarray<int32>((data | p->data).template astype<int32>());
            } else if(auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* bool | int64 */
                return new ndarray<int_>((data | p->data).template astype<int_>());
            }
        } else if constexpr (std::is_same_v<T, int8>) {
            if(auto p = dynamic_cast<const ndarray<bool_>*>(&other)) { /* int8 | bool */
                return new ndarray<int8>(data | p->data);
            } else if(auto p = dynamic_cast<const ndarray<int8>*>(&other)) { /* int8 | int8 */
                return new ndarray<int8>(data | p->data);
            } else if(auto p = dynamic_cast<const ndarray<int16>*>(&other)) { /* int8 | int16 */
                return new ndarray<int16>((data | p->data).template astype<int16>());
            } else if(auto p = dynamic_cast<const ndarray<int32>*>(&other)) { /* int8 | int32 */
                return new ndarray<int32>((data | p->data).template astype<int32>());
            } else if(auto p = dynamic_cast<const ndarray<int_>*>(&other)) { /* int8 | int64 */
                return new ndarray<int_>((data | p->data).template astype<int_>());
            }
        } else if constexpr (std::is_same_v<T, int16>) {
            if (auto p = dynamic_cast<const ndarray<bool_> *>(&other)) { /* int16 | bool */
                return new ndarray<int16>(data | p->data);
            } else if (auto p = dynamic_cast<const ndarray<int8> *>(&other)) { /* int16 | int8 */
                return new ndarray<int16>(data | p->data);
            } else if (auto p = dynamic_cast<const ndarray<int16> *>(&other)) { /* int16 | int16 */
                return new ndarray<int16>(data | p->data);
            } else if (auto p = dynamic_cast<const ndarray<int32> *>(&other)) { /* int16 | int32 */
                return new ndarray<int32>((data | p->data).template astype<int32>());
            } else if (auto p = dynamic_cast<const ndarray<int_> *>(&other)) { /* int16 | int64 */
                return new ndarray<int_>((data | p->data).template astype<int_>());
            }
        } else if constexpr (std::is_same_v<T, int32>) {
            if (auto p = dynamic_cast<const ndarray<bool_> *>(&other)) { /* int32 | bool */
                return new ndarray<int32>(data | p->data);
            } else if (auto p = dynamic_cast<const ndarray<int8> *>(&other)) { /* int32 | int8 */
                return new ndarray<int32>(data | p->data);
            } else if (auto p = dynamic_cast<const ndarray<int16> *>(&other)) { /* int32 | int16 */
                return new ndarray<int32>(data | p->data);
            } else if (auto p = dynamic_cast<const ndarray<int32> *>(&other)) { /* int32 | int32 */
                return new ndarray<int32>(data | p->data);
            } else if (auto p = dynamic_cast<const ndarray<int_> *>(&other)) { /* int32 | int64 */
                return new ndarray<int_>((data | p->data).template astype<int_>());
            }
        } else if constexpr (std::is_same_v<T, int_>) {
            if (auto p = dynamic_cast<const ndarray<bool_> *>(&other)) { /* int64 | bool */
                return new ndarray<int_>(data | p->data);
            } else if (auto p = dynamic_cast<const ndarray<int8> *>(&other)) { /* int64 | int8 */
                return new ndarray<int_>(data | p->data);
            } else if (auto p = dynamic_cast<const ndarray<int16> *>(&other)) { /* int64 | int16 */
                return new ndarray<int_>(data | p->data);
            } else if (auto p = dynamic_cast<const ndarray<int32> *>(&other)) { /* int64 | int32 */
                return new ndarray<int_>(data | p->data);
            } else if (auto p = dynamic_cast<const ndarray<int_> *>(&other)) { /* int64 | int64 */
                return new ndarray<int_>(data | p->data);
            }
        }

        throw std::runtime_error("| operator is not compatible with floating types");
    }

    ndarray_base* or_bool(bool_ other) const override {
        if constexpr(std::is_same_v<T, bool_>) {
            return new ndarray<bool_>(data | other);
        } else if constexpr(std::is_same_v<T, int8>) {
            return new ndarray<int8>(data | other);
        } else if constexpr(std::is_same_v<T, int16>) {
            return new ndarray<int16>(data | other);
        } else if constexpr(std::is_same_v<T, int32>) {
            return new ndarray<int32>(data | other);
        } else if constexpr(std::is_same_v<T, int_>) {
            return new ndarray<int_>(data | other);
        }

        throw std::runtime_error("| operator is not compatible with floating types");
    }

    ndarray_base* or_int(int_ other) const override {
        if constexpr(std::is_same_v<T, bool_>) {
            return new ndarray<int_>((data | other).template astype<int_>());
        } else if constexpr(std::is_same_v<T, int8>) {
            return new ndarray<int8>(data | other);
        } else if constexpr(std::is_same_v<T, int16>) {
            return new ndarray<int16>(data | other);
        } else if constexpr(std::is_same_v<T, int32>) {
            return new ndarray<int32>(data | other);
        } else if constexpr(std::is_same_v<T, int_>) {
            return new ndarray<int_>(data | other);
        }

        throw std::runtime_error("| operator is not compatible with floating types");
    }

    ndarray_base* xor_array(const ndarray_base& other) const override {
        if constexpr (std::is_same_v<T, bool_>) {
            if (auto p = dynamic_cast<const ndarray<bool_> *>(&other)) { /* bool ^ bool */
                return new ndarray<bool_>(data ^ p->data);
            } else if (auto p = dynamic_cast<const ndarray<int8> *>(&other)) { /* bool ^ int8 */
                return new ndarray<int8>((data ^ p->data).template astype<int8>());
            } else if (auto p = dynamic_cast<const ndarray<int16> *>(&other)) { /* bool ^ int16 */
                return new ndarray<int16>((data ^ p->data).template astype<int16>());
            } else if (auto p = dynamic_cast<const ndarray<int32> *>(&other)) { /* bool ^ int32 */
                return new ndarray<int32>((data ^ p->data).template astype<int32>());
            } else if (auto p = dynamic_cast<const ndarray<int_> *>(&other)) { /* bool ^ int64 */
                return new ndarray<int_>((data ^ p->data).template astype<int_>());
            }
        } else if constexpr (std::is_same_v<T, int8>) {
            if (auto p = dynamic_cast<const ndarray<bool_> *>(&other)) { /* int8 ^ bool */
                return new ndarray<int8>(data ^ p->data);
            } else if (auto p = dynamic_cast<const ndarray<int8> *>(&other)) { /* int8 ^ int8 */
                return new ndarray<int8>(data ^ p->data);
            } else if (auto p = dynamic_cast<const ndarray<int16> *>(&other)) { /* int8 ^ int16 */
                return new ndarray<int16>((data ^ p->data).template astype<int16>());
            } else if (auto p = dynamic_cast<const ndarray<int32> *>(&other)) { /* int8 ^ int32 */
                return new ndarray<int32>((data ^ p->data).template astype<int32>());
            } else if (auto p = dynamic_cast<const ndarray<int_> *>(&other)) { /* int8 ^ int64 */
                return new ndarray<int_>((data ^ p->data).template astype<int_>());
            }
        } else if constexpr (std::is_same_v<T, int16>) {
            if (auto p = dynamic_cast<const ndarray<bool_> *>(&other)) { /* int16 ^ bool */
                return new ndarray<int16>(data ^ p->data);
            } else if (auto p = dynamic_cast<const ndarray<int8> *>(&other)) { /* int16 ^ int8 */
                return new ndarray<int16>(data ^ p->data);
            } else if (auto p = dynamic_cast<const ndarray<int16> *>(&other)) { /* int16 ^ int16 */
                return new ndarray<int16>(data ^ p->data);
            } else if (auto p = dynamic_cast<const ndarray<int32> *>(&other)) { /* int16 ^ int32 */
                return new ndarray<int32>((data ^ p->data).template astype<int32>());
            } else if (auto p = dynamic_cast<const ndarray<int_> *>(&other)) { /* int16 ^ int64 */
                return new ndarray<int_>((data ^ p->data).template astype<int_>());
            }
        } else if constexpr (std::is_same_v<T, int32>) {
            if (auto p = dynamic_cast<const ndarray<bool_> *>(&other)) { /* int32 ^ bool */
                return new ndarray<int32>(data ^ p->data);
            } else if (auto p = dynamic_cast<const ndarray<int8> *>(&other)) { /* int32 ^ int8 */
                return new ndarray<int32>(data ^ p->data);
            } else if (auto p = dynamic_cast<const ndarray<int16> *>(&other)) { /* int32 ^ int16 */
                return new ndarray<int32>(data ^ p->data);
            } else if (auto p = dynamic_cast<const ndarray<int32> *>(&other)) { /* int32 ^ int32 */
                return new ndarray<int32>(data ^ p->data);
            } else if (auto p = dynamic_cast<const ndarray<int_> *>(&other)) { /* int32 ^ int64 */
                return new ndarray<int_>((data ^ p->data).template astype<int_>());
            }
        } else if constexpr (std::is_same_v<T, int_>) {
            if (auto p = dynamic_cast<const ndarray<bool_> *>(&other)) { /* int64 ^ bool */
                return new ndarray<int_>(data ^ p->data);
            } else if (auto p = dynamic_cast<const ndarray<int8> *>(&other)) { /* int64 ^ int8 */
                return new ndarray<int_>(data ^ p->data);
            } else if (auto p = dynamic_cast<const ndarray<int16> *>(&other)) { /* int64 ^ int16 */
                return new ndarray<int_>(data ^ p->data);
            } else if (auto p = dynamic_cast<const ndarray<int32> *>(&other)) { /* int64 ^ int32 */
                return new ndarray<int_>(data ^ p->data);
            } else if (auto p = dynamic_cast<const ndarray<int_> *>(&other)) { /* int64 ^ int64 */
                return new ndarray<int_>(data ^ p->data);
            }
        }

        throw std::runtime_error("^ operator is not compatible with floating types");
    }

    ndarray_base* xor_bool(bool_ other) const override {
        if constexpr(std::is_same_v<T, bool_>) {
            return new ndarray<bool_>(data ^ other);
        } else if constexpr(std::is_same_v<T, int8>) {
            return new ndarray<int8>(data ^ other);
        } else if constexpr(std::is_same_v<T, int16>) {
            return new ndarray<int16>(data ^ other);
        } else if constexpr(std::is_same_v<T, int32>) {
            return new ndarray<int32>(data ^ other);
        } else if constexpr(std::is_same_v<T, int_>) {
            return new ndarray<int_>(data ^ other);
        }

        throw std::runtime_error("^ operator is not compatible with floating types");
    }

    ndarray_base* xor_int(int_ other) const override {
        if constexpr(std::is_same_v<T, bool_>) {
            return new ndarray<int_>((data ^ other).template astype<int_>());
        } else if constexpr(std::is_same_v<T, int8>) {
            return new ndarray<int8>(data ^ other);
        } else if constexpr(std::is_same_v<T, int16>) {
            return new ndarray<int16>(data ^ other);
        } else if constexpr(std::is_same_v<T, int32>) {
            return new ndarray<int32>(data ^ other);
        } else if constexpr(std::is_same_v<T, int_>) {
            return new ndarray<int_>(data ^ other);
        }

        throw std::runtime_error("^ operator is not compatible with floating types");
    }

    ndarray_base* invert() const override {
        if constexpr(std::is_same_v<T, bool_>) {
            return new ndarray<bool_>(!data);
        } else if constexpr(std::is_same_v<T, int8>) {
            return new ndarray<int8>(!data);
        } else if constexpr(std::is_same_v<T, int16>) {
            return new ndarray<int16>(!data);
        } else if constexpr(std::is_same_v<T, int32>) {
            return new ndarray<int32>(!data);
        } else if constexpr(std::is_same_v<T, int_>) {
            return new ndarray<int_>(!data);
        }

        throw std::runtime_error("~ operator is not compatible with floating types");
    }

    py::object get_item_int(int index) const override {
        if(index < 0) index += data.shape()[0];
        if(data.ndim() == 1) {
            if constexpr(std::is_same_v<T, bool_>) {
                return py::bool_(data(index));
            } else if constexpr(std::is_same_v<T, int_>) {
                return py::int_(data(index));
            } else if constexpr(std::is_same_v<T, float64>) {
                return py::float_(data(index));
            }
        } 
        return py::cast(ndarray<T>(data[index]));
    }

    py::object get_item_tuple(py::tuple args) const override {
        pkpy::numpy::ndarray<T> store = data;
        std::vector<int> indices;
        for(auto item: args) {
            indices.push_back(py::cast<int>(item));
        }
        for(int i = 0; i < indices.size() - 1; i++) {
            if(indices[i] < 0) indices[i] += store.shape()[0];
            pkpy::numpy::ndarray<T> temp = store[indices[i]];
            store = temp;
        }

        if(indices[indices.size() - 1] < 0) indices[indices.size() - 1] += store.shape()[0];
        if(store.ndim() == 1) {
            if constexpr(std::is_same_v<T, bool_>) {
                return py::bool_(store(indices[indices.size() - 1]));
            } else if constexpr(std::is_same_v<T, int_>) {
                return py::int_(store(indices[indices.size() - 1]));
            } else if constexpr(std::is_same_v<T, float64>) {
                return py::float_(store(indices[indices.size() - 1]));
            }
        } 
        return py::cast(ndarray<T>(store[indices[indices.size() - 1]]));
    }

    ndarray_base* get_item_vector(const std::vector<int>& indices) const override {
        return new ndarray<T>(data[indices]);
    }

    ndarray_base* get_item_slice(py::slice slice) const override {
        int start = parseAttr(getattr(slice, "start"));
        int stop = parseAttr(getattr(slice, "stop"));
        int step = parseAttr(getattr(slice, "step"));

        if(step == INT_MAX) step = 1;
        if(step > 0) {
            if(start == INT_MAX) start = 0;
            if(stop == INT_MAX) stop = data.shape()[0];
        } else if(step < 0) {
            if(start == INT_MAX) start = data.shape()[0] - 1;
            if(stop == INT_MAX) stop = -(1 + data.shape()[0]);
        }

        return new ndarray<T>(data[std::make_tuple(start, stop, step)]);
    }

    void set_item_int(int index, int_ value) override {
        if constexpr(std::is_same_v<T, int_>) {
            if (data.ndim() == 1) {
                data.set_item(index, value);
            } else {
                data.set_item(index, pkpy::numpy::adapt<int_>(std::vector{value}));
            }
        } else if constexpr(std::is_same_v<T, float64>) {
            if (data.ndim() == 1) {
                data.set_item(index, static_cast<T>(value));
            } else {
                data.set_item(index, (pkpy::numpy::adapt<int_>(std::vector{value})).astype<float64>());
            }
        }
    }

    void set_item_index_int(int index, const std::vector<int_>& value) override {
        if constexpr(std::is_same_v<T, int_>) {
            data.set_item(index, pkpy::numpy::adapt<int_>(value));
        } else if constexpr(std::is_same_v<T, float64>) {
            data.set_item(index, (pkpy::numpy::adapt<int_>(value)).astype<float64>());
        }
    }

    void set_item_index_int_2d(int index, const std::vector<std::vector<int_>>& value) override {
        if constexpr(std::is_same_v<T, int_>) {
            data.set_item(index, pkpy::numpy::adapt<int_>(value));
        } else if constexpr(std::is_same_v<T, float64>) {
            data.set_item(index, (pkpy::numpy::adapt<int_>(value)).astype<float64>());
        }
    }

    void set_item_index_int_3d(int index, const std::vector<std::vector<std::vector<int_>>>& value) override {
        if constexpr(std::is_same_v<T, int_>) {
            data.set_item(index, pkpy::numpy::adapt<int_>(value));
        } else if constexpr(std::is_same_v<T, float64>) {
            data.set_item(index, (pkpy::numpy::adapt<int_>(value)).astype<float64>());
        }
    }

    void set_item_index_int_4d(int index, const std::vector<std::vector<std::vector<std::vector<int_>>>>& value) override {
        if constexpr(std::is_same_v<T, int_>) {
            data.set_item(index, pkpy::numpy::adapt<int_>(value));
        } else if constexpr(std::is_same_v<T, float64>) {
            data.set_item(index, (pkpy::numpy::adapt<int_>(value)).astype<float64>());
        }
    }

    void set_item_float(int index, float64 value) override {
        if constexpr(std::is_same_v<T, float64>) {
            if (data.ndim() == 1) {
                data.set_item(index, value);
            } else {
                data.set_item(index, pkpy::numpy::adapt<float64>(std::vector{value}));
            }
        } else if constexpr(std::is_same_v<T, int_>) {
            if (data.ndim() == 1) {
                data.set_item(index, static_cast<T>(value));
            } else {
                data.set_item(index, (pkpy::numpy::adapt<float64>(std::vector{value})).astype<int_>());
            }
        }
    }

    void set_item_index_float(int index, const std::vector<float64>& value) override {
        if constexpr(std::is_same_v<T, float64>) {
            data.set_item(index, pkpy::numpy::adapt<float64>(value));
        } else if constexpr(std::is_same_v<T, int_>) {
            data.set_item(index, (pkpy::numpy::adapt<float64>(value)).astype<int_>());
        }
    }

    void set_item_index_float_2d(int index, const std::vector<std::vector<float64>>& value) override {
        if constexpr(std::is_same_v<T, float64>) {
            data.set_item(index, pkpy::numpy::adapt<float64>(value));
        } else if constexpr(std::is_same_v<T, int_>) {
            data.set_item(index, (pkpy::numpy::adapt<float64>(value)).astype<int_>());
        }
    }

    void set_item_index_float_3d(int index, const std::vector<std::vector<std::vector<float64>>>& value) override {
        if constexpr(std::is_same_v<T, float64>) {
            data.set_item(index, pkpy::numpy::adapt<float64>(value));
        } else if constexpr(std::is_same_v<T, int_>) {
            data.set_item(index, (pkpy::numpy::adapt<float64>(value)).astype<int_>());
        }
    }

    void set_item_index_float_4d(int index, const std::vector<std::vector<std::vector<std::vector<float64>>>>& value) override {
        if constexpr(std::is_same_v<T, float64>) {
            data.set_item(index, pkpy::numpy::adapt<float64>(value));
        } else if constexpr(std::is_same_v<T, int_>) {
            data.set_item(index, (pkpy::numpy::adapt<float64>(value)).astype<int_>());
        }
    }

    void set_item_tuple_int1(py::tuple args, int_ value) override {
        std::vector<int> indices;
        for(auto item: args) {
            indices.push_back(py::cast<int>(item));
        }
        if(indices.size() == 1) {
            int index = indices[0];
            if constexpr(std::is_same_v<T, int_>) {
                data.set_item(index, pkpy::numpy::adapt<int_>(std::vector{value}));
            } else if constexpr(std::is_same_v<T, float64>) {
                data.set_item(index, (pkpy::numpy::adapt<int_>(std::vector{value})).astype<float64>());
            }
        } else if(indices.size() == 2 && indices.size() <= data.ndim())
            data.set_item(indices[0], indices[1], static_cast<T>(value));
        else if(indices.size() == 3 && indices.size() <= data.ndim())
            data.set_item(indices[0], indices[1], indices[2], static_cast<T>(value));
        else if(indices.size() == 4 && indices.size() <= data.ndim())
            data.set_item(indices[0], indices[1], indices[2], indices[3], static_cast<T>(value));
        else if(indices.size() == 5 && indices.size() <= data.ndim())
            data.set_item(indices[0], indices[1], indices[2], indices[3], indices[4], static_cast<T>(value));
    }

    void set_item_tuple_int2(py::tuple args, const std::vector<int_>& value) override {
        std::vector<int> indices;
        for(auto item: args) {
            indices.push_back(py::cast<int>(item));
        }
        if(indices.size() == 1) {
            int index = indices[0];
            if constexpr(std::is_same_v<T, int_>) {
                data.set_item(index, pkpy::numpy::adapt<int_>(value));
            } else if constexpr(std::is_same_v<T, float64>) {
                data.set_item(index, (pkpy::numpy::adapt<int_>(value)).astype<float64>());
            }
        } else if(indices.size() == 2 && indices.size() <= data.ndim()) {
            if constexpr(std::is_same_v<T, int_>) {
                data.set_item(indices[0], indices[1], pkpy::numpy::adapt<int_>(value));
            } else if constexpr(std::is_same_v<T, float64>) {
                data.set_item(indices[0], indices[1], (pkpy::numpy::adapt<int_>(value)).astype<float64>());
            }
        } else if(indices.size() == 3 && indices.size() <= data.ndim()) {
            if constexpr(std::is_same_v<T, int_>) {
                data.set_item(indices[0], indices[1], indices[2], pkpy::numpy::adapt<int_>(value));
            } else if constexpr(std::is_same_v<T, float64>) {
                data.set_item(indices[0], indices[1], indices[2], (pkpy::numpy::adapt<int_>(value)).astype<float64>());
            }
        } else if(indices.size() == 4 && indices.size() <= data.ndim()) {
            if constexpr(std::is_same_v<T, int_>) {
                data.set_item(indices[0], indices[1], indices[2], indices[3], pkpy::numpy::adapt<int_>(value));
            } else if constexpr(std::is_same_v<T, float64>) {
                data.set_item(indices[0], indices[1], indices[2], indices[3], (pkpy::numpy::adapt<int_>(value)).astype<float64>());
            }
        } else if(indices.size() == 5 && indices.size() <= data.ndim()) {
            if constexpr(std::is_same_v<T, int_>) {
                data.set_item(indices[0], indices[1], indices[2], indices[3], indices[4], pkpy::numpy::adapt<int_>(value));
            } else if constexpr(std::is_same_v<T, float64>) {
                data.set_item(indices[0], indices[1], indices[2], indices[3], indices[4], (pkpy::numpy::adapt<int_>(value)).astype<float64>());
            }
        }
    }

    void set_item_tuple_int3(py::tuple args, const std::vector<std::vector<int_>>& value) override {
        std::vector<int> indices;
        for(auto item: args) {
            indices.push_back(py::cast<int>(item));
        }
        if(indices.size() == 1) {
            int index = indices[0];
            if constexpr(std::is_same_v<T, int_>) {
                data.set_item(index, pkpy::numpy::adapt<int_>(value));
            } else if constexpr(std::is_same_v<T, float64>) {
                data.set_item(index, (pkpy::numpy::adapt<int_>(value)).astype<float64>());
            }
        } else if(indices.size() == 2 && indices.size() <= data.ndim()) {
            if constexpr(std::is_same_v<T, int_>) {
                data.set_item(indices[0], indices[1], pkpy::numpy::adapt<int_>(value));
            } else if constexpr(std::is_same_v<T, float64>) {
                data.set_item(indices[0], indices[1], (pkpy::numpy::adapt<int_>(value)).astype<float64>());
            }
        } else if(indices.size() == 3 && indices.size() <= data.ndim()) {
            if constexpr(std::is_same_v<T, int_>) {
                data.set_item(indices[0], indices[1], indices[2], pkpy::numpy::adapt<int_>(value));
            } else if constexpr(std::is_same_v<T, float64>) {
                data.set_item(indices[0], indices[1], indices[2], (pkpy::numpy::adapt<int_>(value)).astype<float64>());
            }
        } else if(indices.size() == 4 && indices.size() <= data.ndim()) {
            if constexpr(std::is_same_v<T, int_>) {
                data.set_item(indices[0], indices[1], indices[2], indices[3], pkpy::numpy::adapt<int_>(value));
            } else if constexpr(std::is_same_v<T, float64>) {
                data.set_item(indices[0], indices[1], indices[2], indices[3], (pkpy::numpy::adapt<int_>(value)).astype<float64>());
            }
        } else if(indices.size() == 5 && indices.size() <= data.ndim()) {
            if constexpr(std::is_same_v<T, int_>) {
                data.set_item(indices[0], indices[1], indices[2], indices[3], indices[4], pkpy::numpy::adapt<int_>(value));
            } else if constexpr(std::is_same_v<T, float64>) {
                data.set_item(indices[0], indices[1], indices[2], indices[3], indices[4], (pkpy::numpy::adapt<int_>(value)).astype<float64>());
            }
        }
    }

    void set_item_tuple_int4(py::tuple args, const std::vector<std::vector<std::vector<int_>>>& value) override {
        std::vector<int> indices;
        for(auto item: args) {
            indices.push_back(py::cast<int>(item));
        }
        if(indices.size() == 1) {
            int index = indices[0];
            if constexpr(std::is_same_v<T, int_>) {
                data.set_item(index, pkpy::numpy::adapt<int_>(value));
            } else if constexpr(std::is_same_v<T, float64>) {
                data.set_item(index, (pkpy::numpy::adapt<int_>(value)).astype<float64>());
            }
        } else if(indices.size() == 2 && indices.size() <= data.ndim()) {
            if constexpr(std::is_same_v<T, int_>) {
                data.set_item(indices[0], indices[1], pkpy::numpy::adapt<int_>(value));
            } else if constexpr(std::is_same_v<T, float64>) {
                data.set_item(indices[0], indices[1], (pkpy::numpy::adapt<int_>(value)).astype<float64>());
            }
        } else if(indices.size() == 3 && indices.size() <= data.ndim()) {
            if constexpr(std::is_same_v<T, int_>) {
                data.set_item(indices[0], indices[1], indices[2], pkpy::numpy::adapt<int_>(value));
            } else if constexpr(std::is_same_v<T, float64>) {
                data.set_item(indices[0], indices[1], indices[2], (pkpy::numpy::adapt<int_>(value)).astype<float64>());
            }
        } else if(indices.size() == 4 && indices.size() <= data.ndim()) {
            if constexpr(std::is_same_v<T, int_>) {
                data.set_item(indices[0], indices[1], indices[2], indices[3], pkpy::numpy::adapt<int_>(value));
            } else if constexpr(std::is_same_v<T, float64>) {
                data.set_item(indices[0], indices[1], indices[2], indices[3], (pkpy::numpy::adapt<int_>(value)).astype<float64>());
            }
        } else if(indices.size() == 5 && indices.size() <= data.ndim()) {
            if constexpr(std::is_same_v<T, int_>) {
                data.set_item(indices[0], indices[1], indices[2], indices[3], indices[4], pkpy::numpy::adapt<int_>(value));
            } else if constexpr(std::is_same_v<T, float64>) {
                data.set_item(indices[0], indices[1], indices[2], indices[3], indices[4], (pkpy::numpy::adapt<int_>(value)).astype<float64>());
            }
        }
    }

    void set_item_tuple_int5(py::tuple args, const std::vector<std::vector<std::vector<std::vector<int_>>>>& value) override {
        std::vector<int> indices;
        for(auto item: args) {
            indices.push_back(py::cast<int>(item));
        }
        if(indices.size() == 1) {
            int index = indices[0];
            if constexpr(std::is_same_v<T, int_>) {
                data.set_item(index, pkpy::numpy::adapt<int_>(value));
            } else if constexpr(std::is_same_v<T, float64>) {
                data.set_item(index, (pkpy::numpy::adapt<int_>(value)).astype<float64>());
            }
        } else if(indices.size() == 2 && indices.size() <= data.ndim()) {
            if constexpr(std::is_same_v<T, int_>) {
                data.set_item(indices[0], indices[1], pkpy::numpy::adapt<int_>(value));
            } else if constexpr(std::is_same_v<T, float64>) {
                data.set_item(indices[0], indices[1], (pkpy::numpy::adapt<int_>(value)).astype<float64>());
            }
        } else if(indices.size() == 3 && indices.size() <= data.ndim()) {
            if constexpr(std::is_same_v<T, int_>) {
                data.set_item(indices[0], indices[1], indices[2], pkpy::numpy::adapt<int_>(value));
            } else if constexpr(std::is_same_v<T, float64>) {
                data.set_item(indices[0], indices[1], indices[2], (pkpy::numpy::adapt<int_>(value)).astype<float64>());
            }
        } else if(indices.size() == 4 && indices.size() <= data.ndim()) {
            if constexpr(std::is_same_v<T, int_>) {
                data.set_item(indices[0], indices[1], indices[2], indices[3], pkpy::numpy::adapt<int_>(value));
            } else if constexpr(std::is_same_v<T, float64>) {
                data.set_item(indices[0], indices[1], indices[2], indices[3], (pkpy::numpy::adapt<int_>(value)).astype<float64>());
            }
        } else if(indices.size() == 5 && indices.size() <= data.ndim()) {
            if constexpr(std::is_same_v<T, int_>) {
                data.set_item(indices[0], indices[1], indices[2], indices[3], indices[4], pkpy::numpy::adapt<int_>(value));
            } else if constexpr(std::is_same_v<T, float64>) {
                data.set_item(indices[0], indices[1], indices[2], indices[3], indices[4], (pkpy::numpy::adapt<int_>(value)).astype<float64>());
            }
        }
    }

    void set_item_tuple_float1(py::tuple args, float64 value) override {
        std::vector<int> indices;
        for(auto item: args) {
            indices.push_back(py::cast<int>(item));
        }
        if(indices.size() == 1) {
            int index = indices[0];
            if constexpr(std::is_same_v<T, float64>) {
                data.set_item(index, pkpy::numpy::adapt<float64>(std::vector{value}));
            } else if constexpr(std::is_same_v<T, int_>) {
                data.set_item(index, (pkpy::numpy::adapt<float64>(std::vector{value})).astype<int_>());
            }
        } else if(indices.size() == 2 && indices.size() <= data.ndim())
            data.set_item(indices[0], indices[1], static_cast<T>(value));
        else if(indices.size() == 3 && indices.size() <= data.ndim())
            data.set_item(indices[0], indices[1], indices[2], static_cast<T>(value));
        else if(indices.size() == 4 && indices.size() <= data.ndim())
            data.set_item(indices[0], indices[1], indices[2], indices[3], static_cast<T>(value));
        else if(indices.size() == 5 && indices.size() <= data.ndim())
            data.set_item(indices[0], indices[1], indices[2], indices[3], indices[4], static_cast<T>(value));
    }

    void set_item_tuple_float2(py::tuple args, const std::vector<float64>& value) override {
        std::vector<int> indices;
        for(auto item: args) {
            indices.push_back(py::cast<int>(item));
        }
        if(indices.size() == 1) {
            int index = indices[0];
            if constexpr(std::is_same_v<T, float64>) {
                data.set_item(index, pkpy::numpy::adapt<float64>(value));
            } else if constexpr(std::is_same_v<T, int_>) {
                data.set_item(index, (pkpy::numpy::adapt<float64>(value)).astype<int_>());
            }
        } else if(indices.size() == 2 && indices.size() <= data.ndim()) {
            if constexpr (std::is_same_v<T, float64>) {
                data.set_item(indices[0], indices[1], pkpy::numpy::adapt<float64>(value));
            } else if constexpr (std::is_same_v<T, int_>) {
                data.set_item(indices[0], indices[1], (pkpy::numpy::adapt<float64>(value)).astype<int_>());
            }
        }
        else if(indices.size() == 3 && indices.size() <= data.ndim()) {
            if constexpr (std::is_same_v<T, float64>) {
                data.set_item(indices[0], indices[1], indices[2], pkpy::numpy::adapt<float64>(value));
            } else if constexpr (std::is_same_v<T, int_>) {
                data.set_item(indices[0], indices[1], indices[2], (pkpy::numpy::adapt<float64>(value)).astype<int_>());
            }
        }
        else if(indices.size() == 4 && indices.size() <= data.ndim()) {
            if constexpr (std::is_same_v<T, float64>) {
                data.set_item(indices[0], indices[1], indices[2], indices[3], pkpy::numpy::adapt<float64>(value));
            } else if constexpr (std::is_same_v<T, int_>) {
                data.set_item(indices[0], indices[1], indices[2], indices[3], (pkpy::numpy::adapt<float64>(value)).astype<int_>());
            }
        }
        else if(indices.size() == 5 && indices.size() <= data.ndim()) {
            if constexpr (std::is_same_v<T, float64>) {
                data.set_item(indices[0], indices[1], indices[2], indices[3], indices[4], pkpy::numpy::adapt<float64>(value));
            } else if constexpr (std::is_same_v<T, int_>) {
                data.set_item(indices[0], indices[1], indices[2], indices[3], indices[4], (pkpy::numpy::adapt<float64>(value)).astype<int_>());
            }
        }
    }

    void set_item_tuple_float3(py::tuple args, const std::vector<std::vector<float64>>& value) override {
        std::vector<int> indices;
        for(auto item: args) {
            indices.push_back(py::cast<int>(item));
        }
        if(indices.size() == 1) {
            int index = indices[0];
            if constexpr(std::is_same_v<T, float64>) {
                data.set_item(index, pkpy::numpy::adapt<float64>(value));
            } else if constexpr(std::is_same_v<T, int_>) {
                data.set_item(index, (pkpy::numpy::adapt<float64>(value)).astype<int_>());
            }
        } else if(indices.size() == 2 && indices.size() <= data.ndim()) {
            if constexpr (std::is_same_v<T, float64>) {
                data.set_item(indices[0], indices[1], pkpy::numpy::adapt<float64>(value));
            } else if constexpr (std::is_same_v<T, int_>) {
                data.set_item(indices[0], indices[1], (pkpy::numpy::adapt<float64>(value)).astype<int_>());
            }
        }
        else if(indices.size() == 3 && indices.size() <= data.ndim()) {
            if constexpr (std::is_same_v<T, float64>) {
                data.set_item(indices[0], indices[1], indices[2], pkpy::numpy::adapt<float64>(value));
            } else if constexpr (std::is_same_v<T, int_>) {
                data.set_item(indices[0], indices[1], indices[2], (pkpy::numpy::adapt<float64>(value)).astype<int_>());
            }
        }
        else if(indices.size() == 4 && indices.size() <= data.ndim()) {
            if constexpr (std::is_same_v<T, float64>) {
                data.set_item(indices[0], indices[1], indices[2], indices[3], pkpy::numpy::adapt<float64>(value));
            } else if constexpr (std::is_same_v<T, int_>) {
                data.set_item(indices[0], indices[1], indices[2], indices[3], (pkpy::numpy::adapt<float64>(value)).astype<int_>());
            }
        }
        else if(indices.size() == 5 && indices.size() <= data.ndim()) {
            if constexpr (std::is_same_v<T, float64>) {
                data.set_item(indices[0], indices[1], indices[2], indices[3], indices[4], pkpy::numpy::adapt<float64>(value));
            } else if constexpr (std::is_same_v<T, int_>) {
                data.set_item(indices[0], indices[1], indices[2], indices[3], indices[4], (pkpy::numpy::adapt<float64>(value)).astype<int_>());
            }
        }
    }

    void set_item_tuple_float4(py::tuple args, const std::vector<std::vector<std::vector<float64>>>& value) override {
        std::vector<int> indices;
        for(auto item: args) {
            indices.push_back(py::cast<int>(item));
        }
        if(indices.size() == 1) {
            int index = indices[0];
            if constexpr(std::is_same_v<T, float64>) {
                data.set_item(index, pkpy::numpy::adapt<float64>(value));
            } else if constexpr(std::is_same_v<T, int_>) {
                data.set_item(index, (pkpy::numpy::adapt<float64>(value)).astype<int_>());
            }
        } else if(indices.size() == 2 && indices.size() <= data.ndim()) {
            if constexpr (std::is_same_v<T, float64>) {
                data.set_item(indices[0], indices[1], pkpy::numpy::adapt<float64>(value));
            } else if constexpr (std::is_same_v<T, int_>) {
                data.set_item(indices[0], indices[1], (pkpy::numpy::adapt<float64>(value)).astype<int_>());
            }
        }
        else if(indices.size() == 3 && indices.size() <= data.ndim()) {
            if constexpr (std::is_same_v<T, float64>) {
                data.set_item(indices[0], indices[1], indices[2], pkpy::numpy::adapt<float64>(value));
            } else if constexpr (std::is_same_v<T, int_>) {
                data.set_item(indices[0], indices[1], indices[2], (pkpy::numpy::adapt<float64>(value)).astype<int_>());
            }
        }
        else if(indices.size() == 4 && indices.size() <= data.ndim()) {
            if constexpr (std::is_same_v<T, float64>) {
                data.set_item(indices[0], indices[1], indices[2], indices[3], pkpy::numpy::adapt<float64>(value));
            } else if constexpr (std::is_same_v<T, int_>) {
                data.set_item(indices[0], indices[1], indices[2], indices[3], (pkpy::numpy::adapt<float64>(value)).astype<int_>());
            }
        }
        else if(indices.size() == 5 && indices.size() <= data.ndim()) {
            if constexpr (std::is_same_v<T, float64>) {
                data.set_item(indices[0], indices[1], indices[2], indices[3], indices[4], pkpy::numpy::adapt<float64>(value));
            } else if constexpr (std::is_same_v<T, int_>) {
                data.set_item(indices[0], indices[1], indices[2], indices[3], indices[4], (pkpy::numpy::adapt<float64>(value)).astype<int_>());
            }
        }
    }

    void set_item_tuple_float5(py::tuple args, const std::vector<std::vector<std::vector<std::vector<float64>>>>& value) override {
        std::vector<int> indices;
        for(auto item: args) {
            indices.push_back(py::cast<int>(item));
        }
        if(indices.size() == 1) {
            int index = indices[0];
            if constexpr(std::is_same_v<T, float64>) {
                data.set_item(index, pkpy::numpy::adapt<float64>(value));
            } else if constexpr(std::is_same_v<T, int_>) {
                data.set_item(index, (pkpy::numpy::adapt<float64>(value)).astype<int_>());
            }
        } else if(indices.size() == 2 && indices.size() <= data.ndim()) {
            if constexpr (std::is_same_v<T, float64>) {
                data.set_item(indices[0], indices[1], pkpy::numpy::adapt<float64>(value));
            } else if constexpr (std::is_same_v<T, int_>) {
                data.set_item(indices[0], indices[1], (pkpy::numpy::adapt<float64>(value)).astype<int_>());
            }
        }
        else if(indices.size() == 3 && indices.size() <= data.ndim()) {
            if constexpr (std::is_same_v<T, float64>) {
                data.set_item(indices[0], indices[1], indices[2], pkpy::numpy::adapt<float64>(value));
            } else if constexpr (std::is_same_v<T, int_>) {
                data.set_item(indices[0], indices[1], indices[2], (pkpy::numpy::adapt<float64>(value)).astype<int_>());
            }
        }
        else if(indices.size() == 4 && indices.size() <= data.ndim()) {
            if constexpr (std::is_same_v<T, float64>) {
                data.set_item(indices[0], indices[1], indices[2], indices[3], pkpy::numpy::adapt<float64>(value));
            } else if constexpr (std::is_same_v<T, int_>) {
                data.set_item(indices[0], indices[1], indices[2], indices[3], (pkpy::numpy::adapt<float64>(value)).astype<int_>());
            }
        }
        else if(indices.size() == 5 && indices.size() <= data.ndim()) {
            if constexpr (std::is_same_v<T, float64>) {
                data.set_item(indices[0], indices[1], indices[2], indices[3], indices[4], pkpy::numpy::adapt<float64>(value));
            } else if constexpr (std::is_same_v<T, int_>) {
                data.set_item(indices[0], indices[1], indices[2], indices[3], indices[4], (pkpy::numpy::adapt<float64>(value)).astype<int_>());
            }
        }
    }

    void set_item_vector_int1(const std::vector<int>& indices, int_ value) override {
        if constexpr(std::is_same_v<T, int_>) {
            data.set_item(indices, pkpy::numpy::adapt<int_>(std::vector{value}));
        } else if constexpr(std::is_same_v<T, float64>) {
            data.set_item(indices, (pkpy::numpy::adapt<int_>(std::vector{value})).astype<float64>());
        }
    }

    void set_item_vector_int2(const std::vector<int>& indices, const std::vector<int_>& value) override {
        if constexpr(std::is_same_v<T, int_>) {
            data.set_item(indices, pkpy::numpy::adapt<int_>(value));
        } else if constexpr(std::is_same_v<T, float64>) {
            data.set_item(indices, (pkpy::numpy::adapt<int_>(value)).astype<float64>());
        }
    }

    void set_item_vector_int3(const std::vector<int>& indices, const std::vector<std::vector<int_>>& value) override {
        if constexpr(std::is_same_v<T, int_>) {
            data.set_item(indices, pkpy::numpy::adapt<int_>(value));
        } else if constexpr(std::is_same_v<T, float64>) {
            data.set_item(indices, (pkpy::numpy::adapt<int_>(value)).astype<float64>());
        }
    }

    void set_item_vector_int4(const std::vector<int>& indices, const std::vector<std::vector<std::vector<int_>>>& value) override {
        if constexpr(std::is_same_v<T, int_>) {
            data.set_item(indices, pkpy::numpy::adapt<int_>(value));
        } else if constexpr(std::is_same_v<T, float64>) {
            data.set_item(indices, (pkpy::numpy::adapt<int_>(value)).astype<float64>());
        }
    }

    void set_item_vector_int5(const std::vector<int>& indices, const std::vector<std::vector<std::vector<std::vector<int_>>>>& value) override {
        if constexpr(std::is_same_v<T, int_>) {
            data.set_item(indices, pkpy::numpy::adapt<int_>(value));
        } else if constexpr(std::is_same_v<T, float64>) {
            data.set_item(indices, (pkpy::numpy::adapt<int_>(value)).astype<float64>());
        }
    }

    void set_item_vector_float1(const std::vector<int>& indices, float64 value) override {
        if constexpr(std::is_same_v<T, float64>) {
            data.set_item(indices, pkpy::numpy::adapt<float64>(std::vector{value}));
        } else if constexpr(std::is_same_v<T, int_>) {
            data.set_item(indices, (pkpy::numpy::adapt<float64>(std::vector{value})).astype<int_>());
        }
    }

    void set_item_vector_float2(const std::vector<int>& indices, const std::vector<float64>& value) override {
        if constexpr(std::is_same_v<T, float64>) {
            data.set_item(indices, pkpy::numpy::adapt<float64>(value));
        } else if constexpr(std::is_same_v<T, int_>) {
            data.set_item(indices, (pkpy::numpy::adapt<float64>(value)).astype<int_>());
        }
    }

    void set_item_vector_float3(const std::vector<int>& indices, const std::vector<std::vector<float64>>& value) override {
        if constexpr(std::is_same_v<T, float64>) {
            data.set_item(indices, pkpy::numpy::adapt<float64>(value));
        } else if constexpr(std::is_same_v<T, int_>) {
            data.set_item(indices, (pkpy::numpy::adapt<float64>(value)).astype<int_>());
        }
    }

    void set_item_vector_float4(const std::vector<int>& indices, const std::vector<std::vector<std::vector<float64>>>& value) override {
        if constexpr(std::is_same_v<T, float64>) {
            data.set_item(indices, pkpy::numpy::adapt<float64>(value));
        } else if constexpr(std::is_same_v<T, int_>) {
            data.set_item(indices, (pkpy::numpy::adapt<float64>(value)).astype<int_>());
        }
    }

    void set_item_vector_float5(const std::vector<int>& indices, const std::vector<std::vector<std::vector<std::vector<float64>>>>& value) override {
        if constexpr(std::is_same_v<T, float64>) {
            data.set_item(indices, pkpy::numpy::adapt<float64>(value));
        } else if constexpr(std::is_same_v<T, int_>) {
            data.set_item(indices, (pkpy::numpy::adapt<float64>(value)).astype<int_>());
        }
    }

    void set_item_slice_int1(py::slice slice, int_ value) override {
        int start = parseAttr(getattr(slice, "start"));
        int stop = parseAttr(getattr(slice, "stop"));
        int step = parseAttr(getattr(slice, "step"));

        if(step == INT_MAX) step = 1;
        if(step > 0) {
            if(start == INT_MAX) start = 0;
            if(stop == INT_MAX) stop = data.shape()[0];
        } else if(step < 0) {
            if(start == INT_MAX) start = data.shape()[0] - 1;
            if(stop == INT_MAX) stop = -(1 + data.shape()[0]);
        }
        if constexpr(std::is_same_v<T, int_>) {
            data.set_item(std::make_tuple(start, stop, step), pkpy::numpy::adapt<int_>(std::vector{value}));
        } else if constexpr(std::is_same_v<T, float64>) {
            data.set_item(std::make_tuple(start, stop, step),
                          (pkpy::numpy::adapt<int_>(std::vector{value})).astype<float64>());
        }
    }

    void set_item_slice_int2(py::slice slice, const std::vector<int_>& value) override {
        int start = parseAttr(getattr(slice, "start"));
        int stop = parseAttr(getattr(slice, "stop"));
        int step = parseAttr(getattr(slice, "step"));

        if(step == INT_MAX) step = 1;
        if(step > 0) {
            if(start == INT_MAX) start = 0;
            if(stop == INT_MAX) stop = data.shape()[0];
        } else if(step < 0) {
            if(start == INT_MAX) start = data.shape()[0] - 1;
            if(stop == INT_MAX) stop = -(1 + data.shape()[0]);
        }
        if constexpr(std::is_same_v<T, int_>) {
            data.set_item(std::make_tuple(start, stop, step), pkpy::numpy::adapt<int_>(value));
        } else if constexpr(std::is_same_v<T, float64>) {
            data.set_item(std::make_tuple(start, stop, step), (pkpy::numpy::adapt<int_>(value)).astype<float64>());
        }
    }

    void set_item_slice_int3(py::slice slice, const std::vector<std::vector<int_>>& value) override {
        int start = parseAttr(getattr(slice, "start"));
        int stop = parseAttr(getattr(slice, "stop"));
        int step = parseAttr(getattr(slice, "step"));

        if(step == INT_MAX) step = 1;
        if(step > 0) {
            if(start == INT_MAX) start = 0;
            if(stop == INT_MAX) stop = data.shape()[0];
        } else if(step < 0) {
            if(start == INT_MAX) start = data.shape()[0] - 1;
            if(stop == INT_MAX) stop = -(1 + data.shape()[0]);
        }
        if constexpr(std::is_same_v<T, int_>) {
            data.set_item(std::make_tuple(start, stop, step), pkpy::numpy::adapt<int_>(value));
        } else if constexpr(std::is_same_v<T, float64>) {
            data.set_item(std::make_tuple(start, stop, step), (pkpy::numpy::adapt<int_>(value)).astype<float64>());
        }
    }

    void set_item_slice_int4(py::slice slice, const std::vector<std::vector<std::vector<int_>>>& value) override {
        int start = parseAttr(getattr(slice, "start"));
        int stop = parseAttr(getattr(slice, "stop"));
        int step = parseAttr(getattr(slice, "step"));

        if(step == INT_MAX) step = 1;
        if(step > 0) {
            if(start == INT_MAX) start = 0;
            if(stop == INT_MAX) stop = data.shape()[0];
        } else if(step < 0) {
            if(start == INT_MAX) start = data.shape()[0] - 1;
            if(stop == INT_MAX) stop = -(1 + data.shape()[0]);
        }
        if constexpr(std::is_same_v<T, int_>) {
            data.set_item(std::make_tuple(start, stop, step), pkpy::numpy::adapt<int_>(value));
        } else if constexpr(std::is_same_v<T, float64>) {
            data.set_item(std::make_tuple(start, stop, step), (pkpy::numpy::adapt<int_>(value)).astype<float64>());
        }
    }

    void set_item_slice_int5(py::slice slice, const std::vector<std::vector<std::vector<std::vector<int_>>>>& value) override {
        int start = parseAttr(getattr(slice, "start"));
        int stop = parseAttr(getattr(slice, "stop"));
        int step = parseAttr(getattr(slice, "step"));

        if(step == INT_MAX) step = 1;
        if(step > 0) {
            if(start == INT_MAX) start = 0;
            if(stop == INT_MAX) stop = data.shape()[0];
        } else if(step < 0) {
            if(start == INT_MAX) start = data.shape()[0] - 1;
            if(stop == INT_MAX) stop = -(1 + data.shape()[0]);
        }
        if constexpr(std::is_same_v<T, int_>) {
            data.set_item(std::make_tuple(start, stop, step), pkpy::numpy::adapt<int_>(value));
        } else if constexpr(std::is_same_v<T, float64>) {
            data.set_item(std::make_tuple(start, stop, step), (pkpy::numpy::adapt<int_>(value)).astype<float64>());
        }
    }

    void set_item_slice_float1(py::slice slice, float64 value) override {
        int start = parseAttr(getattr(slice, "start"));
        int stop = parseAttr(getattr(slice, "stop"));
        int step = parseAttr(getattr(slice, "step"));

        if(step == INT_MAX) step = 1;
        if(step > 0) {
            if(start == INT_MAX) start = 0;
            if(stop == INT_MAX) stop = data.shape()[0];
        } else if(step < 0) {
            if(start == INT_MAX) start = data.shape()[0] - 1;
            if(stop == INT_MAX) stop = -(1 + data.shape()[0]);
        }
        if constexpr(std::is_same_v<T, float64>) {
            data.set_item(std::make_tuple(start, stop, step), pkpy::numpy::adapt<float64>(std::vector{value}));
        } else if constexpr(std::is_same_v<T, int_>) {
            data.set_item(std::make_tuple(start, stop, step),
                          (pkpy::numpy::adapt<float64>(std::vector{value})).astype<int_>());
        }
    }

    void set_item_slice_float2(py::slice slice, const std::vector<float64>& value) override {
        int start = parseAttr(getattr(slice, "start"));
        int stop = parseAttr(getattr(slice, "stop"));
        int step = parseAttr(getattr(slice, "step"));

        if(step == INT_MAX) step = 1;
        if(step > 0) {
            if(start == INT_MAX) start = 0;
            if(stop == INT_MAX) stop = data.shape()[0];
        } else if(step < 0) {
            if(start == INT_MAX) start = data.shape()[0] - 1;
            if(stop == INT_MAX) stop = -(1 + data.shape()[0]);
        }
        if constexpr(std::is_same_v<T, float64>) {
            data.set_item(std::make_tuple(start, stop, step), pkpy::numpy::adapt<float64>(value));
        } else if constexpr(std::is_same_v<T, int_>) {
            data.set_item(std::make_tuple(start, stop, step), (pkpy::numpy::adapt<float64>(value)).astype<int_>());
        }
    }

    void set_item_slice_float3(py::slice slice, const std::vector<std::vector<float64>>& value) override {
        int start = parseAttr(getattr(slice, "start"));
        int stop = parseAttr(getattr(slice, "stop"));
        int step = parseAttr(getattr(slice, "step"));

        if(step == INT_MAX) step = 1;
        if(step > 0) {
            if(start == INT_MAX) start = 0;
            if(stop == INT_MAX) stop = data.shape()[0];
        } else if(step < 0) {
            if(start == INT_MAX) start = data.shape()[0] - 1;
            if(stop == INT_MAX) stop = -(1 + data.shape()[0]);
        }
        if constexpr(std::is_same_v<T, float64>) {
            data.set_item(std::make_tuple(start, stop, step), pkpy::numpy::adapt<float64>(value));
        } else if constexpr(std::is_same_v<T, int_>) {
            data.set_item(std::make_tuple(start, stop, step), (pkpy::numpy::adapt<float64>(value)).astype<int_>());
        }
    }

    void set_item_slice_float4(py::slice slice, const std::vector<std::vector<std::vector<float64>>>& value) override {
        int start = parseAttr(getattr(slice, "start"));
        int stop = parseAttr(getattr(slice, "stop"));
        int step = parseAttr(getattr(slice, "step"));

        if(step == INT_MAX) step = 1;
        if(step > 0) {
            if(start == INT_MAX) start = 0;
            if(stop == INT_MAX) stop = data.shape()[0];
        } else if(step < 0) {
            if(start == INT_MAX) start = data.shape()[0] - 1;
            if(stop == INT_MAX) stop = -(1 + data.shape()[0]);
        }
        if constexpr(std::is_same_v<T, float64>) {
            data.set_item(std::make_tuple(start, stop, step), pkpy::numpy::adapt<float64>(value));
        } else if constexpr(std::is_same_v<T, int_>) {
            data.set_item(std::make_tuple(start, stop, step), (pkpy::numpy::adapt<float64>(value)).astype<int_>());
        }
    }

    void set_item_slice_float5(py::slice slice, const std::vector<std::vector<std::vector<std::vector<float64>>>>& value) override {
        int start = parseAttr(getattr(slice, "start"));
        int stop = parseAttr(getattr(slice, "stop"));
        int step = parseAttr(getattr(slice, "step"));

        if(step == INT_MAX) step = 1;
        if(step > 0) {
            if(start == INT_MAX) start = 0;
            if(stop == INT_MAX) stop = data.shape()[0];
        } else if(step < 0) {
            if(start == INT_MAX) start = data.shape()[0] - 1;
            if(stop == INT_MAX) stop = -(1 + data.shape()[0]);
        }
        if constexpr(std::is_same_v<T, float64>) {
            data.set_item(std::make_tuple(start, stop, step), pkpy::numpy::adapt<float64>(value));
        } else if constexpr(std::is_same_v<T, int_>) {
            data.set_item(std::make_tuple(start, stop, step), (pkpy::numpy::adapt<float64>(value)).astype<int_>());
        }
    }

    std::string to_string() const override {
        std::ostringstream os;
        os << data;
        std::string result = os.str();

        size_t pos = 0;
        while ((pos = result.find('{', pos)) != std::string::npos) {
            result.replace(pos, 1, "[");
            pos += 1;
        }
        pos = 0;
        while ((pos = result.find('}', pos)) != std::string::npos) {
            result.replace(pos, 1, "]");
            pos += 1;
        }

        if constexpr(std::is_same_v<T, bool_>) {
            size_t pos = 0;
            while ((pos = result.find("true", pos)) != std::string::npos) {
                result.replace(pos, 4, "True");
                pos += 4;
            }
            pos = 0;
            while ((pos = result.find("false", pos)) != std::string::npos) {
                result.replace(pos, 5, "False");
                pos += 5;
            }
        }

        for(int i = 0; i < result.size(); i++) {
            if(result[i] == '\n') {
                result.insert(i + 1, "      ");
            }
        }
        return result;
    }
};

class Random {
public:
    static py::object rand() { return py::float_(pkpy::numpy::random::rand<float64>()); }

    static ndarray_base* rand_shape(py::args args) {
        std::vector<int> shape;
        for(auto item: args)
            shape.push_back(py::cast<int>(item));
        return new ndarray<float64>(pkpy::numpy::random::rand<float64>(shape));
    }

    static py::object randn() { return py::float_(pkpy::numpy::random::randn<float64>()); }

    static ndarray_base* randn_shape(py::args args) {
        std::vector<int> shape;
        for(auto item: args)
            shape.push_back(py::cast<int>(item));
        return new ndarray<float64>(pkpy::numpy::random::randn<float64>(shape));
    }

    static py::object randint(int low, int high) { return py::int_(pkpy::numpy::random::randint<int>(low, high)); }

    static ndarray_base* randint_shape(int_ low, int_ high, const std::vector<int>& shape) {
        return new ndarray<int_>(pkpy::numpy::random::randint<int_>(low, high, shape));
    }

    static ndarray_base* uniform(float64 low, float64 high, const std::vector<int>& shape) {
        return new ndarray<float64>(pkpy::numpy::random::uniform<float64>(low, high, shape));
    }
};

// Declare ndarray types
using ndarray_bool = ndarray<bool_>;
using ndarray_int8 = ndarray<int8>;
using ndarray_int16 = ndarray<int16>;
using ndarray_int32 = ndarray<int32>;
using ndarray_int = ndarray<int_>;
using ndarray_int = ndarray<int64>;
using ndarray_float32 = ndarray<float32>;
using ndarray_float = ndarray<float64>;
using ndarray_float = ndarray<float_>;

// Define template for creating n-dimensional vectors
template <typename T, std::size_t N>
struct nvector_impl {
    using type = std::vector<typename nvector_impl<T, N - 1>::type>;
};
template <typename T>
struct nvector_impl<T, 0> {
    using type = T;
};
template <typename T, std::size_t N>
using nvector = typename nvector_impl<T, N>::type;

// Transform nvector<U, N> to nvector<T, N>
template <typename U, typename T, std::size_t N>
nvector<T, N> transform(const nvector<U, N>& values) {
    nvector<T, N> result;
    if constexpr(N != 0) {
        for (const auto& value : values) {
            result.push_back(transform<U, T, N - 1>(value));
        }
    } else {
        result = static_cast<T>(values);
    }
    return result;
}

void register_array_int(py::module_& m) {
    m.def("array", [](int_ value, const std::string& dtype) {
        if (dtype == "bool") {
            return std::unique_ptr<ndarray_base>(new ndarray_bool(value));
        } else if (dtype == "int8") {
            return std::unique_ptr<ndarray_base>(new ndarray_int8(value));
        } else if (dtype == "int16") {
            return std::unique_ptr<ndarray_base>(new ndarray_int16(value));
        } else if (dtype == "int32") {
            return std::unique_ptr<ndarray_base>(new ndarray_int32(value));
        } else if (dtype == "float32") {
            return std::unique_ptr<ndarray_base>(new ndarray_float32(value));
        } else if (dtype == "float64") {
            return std::unique_ptr<ndarray_base>(new ndarray_float(value));
        }
        return std::unique_ptr<ndarray_base>(new ndarray_int(value));
    }, py::arg("value"), py::arg("dtype") = "int64");
}

template<std::size_t N>
void register_array_int(py::module_& m) {
    m.def("array", [](const nvector<int_, N>& values, const std::string& dtype) {
        if (dtype == "bool") {
            return std::unique_ptr<ndarray_base>(new ndarray<bool_>(transform<int_, bool_, N>(values)));
        } else if (dtype == "int8") {
            return std::unique_ptr<ndarray_base>(new ndarray<int8>(transform<int_, int8, N>(values)));
        } else if (dtype == "int16") {
            return std::unique_ptr<ndarray_base>(new ndarray<int16>(transform<int_, int16, N>(values)));
        } else if (dtype == "int32") {
            return std::unique_ptr<ndarray_base>(new ndarray<int32>(transform<int_, int32, N>(values)));
        } else if (dtype == "float32") {
            return std::unique_ptr<ndarray_base>(new ndarray<float32>(transform<int_, float32, N>(values)));
        } else if (dtype == "float64") {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(transform<int_, float64, N>(values)));
        }
        return std::unique_ptr<ndarray_base>(new ndarray<int_>(values));
    }, py::arg("values"), py::arg("dtype") = "int64");
}

void register_array_float(py::module_& m) {
    m.def("array", [](float64 value, const std::string& dtype) {
        if (dtype == "bool") {
            return std::unique_ptr<ndarray_base>(new ndarray_bool(value));
        } else if (dtype == "int8") {
            return std::unique_ptr<ndarray_base>(new ndarray_int8(value));
        } else if (dtype == "int16") {
            return std::unique_ptr<ndarray_base>(new ndarray_int16(value));
        } else if (dtype == "int32") {
            return std::unique_ptr<ndarray_base>(new ndarray_int32(value));
        } else if (dtype == "int64") {
            return std::unique_ptr<ndarray_base>(new ndarray_int(value));
        } else if (dtype == "float32") {
            return std::unique_ptr<ndarray_base>(new ndarray_float32(value));
        }
        return std::unique_ptr<ndarray_base>(new ndarray_float(value));
    }, py::arg("value"), py::arg("dtype") = "float64");
}

template<std::size_t N>
void register_array_float(py::module_& m) {
    m.def("array", [](const nvector<float64, N>& values, const std::string& dtype) {
        if (dtype == "bool") {
            return std::unique_ptr<ndarray_base>(new ndarray<bool_>(transform<float64, bool_, N>(values)));
        } else if (dtype == "int8") {
            return std::unique_ptr<ndarray_base>(new ndarray<int8>(transform<float64, int8, N>(values)));
        } else if (dtype == "int16") {
            return std::unique_ptr<ndarray_base>(new ndarray<int16>(transform<float64, int16, N>(values)));
        } else if (dtype == "int32") {
            return std::unique_ptr<ndarray_base>(new ndarray<int32>(transform<float64, int32, N>(values)));
        } else if (dtype == "int64") {
            return std::unique_ptr<ndarray_base>(new ndarray<int_>(transform<float64, int_, N>(values)));
        } else if (dtype == "float32") {
            return std::unique_ptr<ndarray_base>(new ndarray<float32>(transform<float64, float32, N>(values)));
        }
        return std::unique_ptr<ndarray_base>(new ndarray<float64>(values));
    }, py::arg("values"), py::arg("dtype") = "float64");
}

// Register array creation functions.
void array_creation_registry(py::module_& m) {
    register_array_int(m);
    register_array_int<1>(m);
    register_array_int<2>(m);
    register_array_int<3>(m);
    register_array_int<4>(m);
    register_array_int<5>(m);

    register_array_float(m);
    register_array_float<1>(m);
    register_array_float<2>(m);
    register_array_float<3>(m);
    register_array_float<4>(m);
    register_array_float<5>(m);
}


PYBIND11_MODULE(numpy, m) {
    m.doc() = "Python bindings for pkpy::numpy::ndarray using pybind11";

    m.attr("bool_") = "bool";
    m.attr("int8") = "int8";
    m.attr("int16") = "int16";
    m.attr("int32") = "int32";
    m.attr("int64") = "int64";
    m.attr("int_") = "int64";
    m.attr("float32") = "float32";
    m.attr("float64") = "float64";
    m.attr("float_") = "float64";

    py::class_<ndarray_base>(m, "ndarray")
        .def_property_readonly("ndim", &ndarray_base::ndim)
        .def_property_readonly("size", &ndarray_base::size)
        .def_property_readonly("dtype", &ndarray_base::dtype)
        .def_property_readonly("shape", &ndarray_base::shape)
        .def("all", &ndarray_base::all)
        .def("any", &ndarray_base::any)
        .def("sum", &ndarray_base::sum)
        .def("sum", &ndarray_base::sum_axis)
        .def("sum", &ndarray_base::sum_axes)
        .def("prod", &ndarray_base::prod)
        .def("prod", &ndarray_base::prod_axis)
        .def("prod", &ndarray_base::prod_axes)
        .def("min", &ndarray_base::min)
        .def("min", &ndarray_base::min_axis)
        .def("min", &ndarray_base::min_axes)
        .def("max", &ndarray_base::max)
        .def("max", &ndarray_base::max_axis)
        .def("max", &ndarray_base::max_axes)
        .def("mean", &ndarray_base::mean)
        .def("mean", &ndarray_base::mean_axis)
        .def("mean", &ndarray_base::mean_axes)
        .def("std", &ndarray_base::std)
        .def("std", &ndarray_base::std_axis)
        .def("std", &ndarray_base::std_axes)
        .def("var", &ndarray_base::var)
        .def("var", &ndarray_base::var_axis)
        .def("var", &ndarray_base::var_axes)
        .def("argmin", &ndarray_base::argmin)
        .def("argmin", &ndarray_base::argmin_axis)
        .def("argmax", &ndarray_base::argmax)
        .def("argmax", &ndarray_base::argmax_axis)
        .def("argsort", &ndarray_base::argsort)
        .def("argsort", &ndarray_base::argsort_axis)
        .def("sort", &ndarray_base::sort)
        .def("sort", &ndarray_base::sort_axis)
        .def("reshape", &ndarray_base::reshape)
        .def("resize", &ndarray_base::resize)
        .def("squeeze", &ndarray_base::squeeze)
        .def("squeeze", &ndarray_base::squeeze_axis)
        .def("transpose", &ndarray_base::transpose)
        .def("transpose", &ndarray_base::transpose_tuple)
        .def("transpose", &ndarray_base::transpose_args)
        .def("repeat", &ndarray_base::repeat, py::arg("repeats"), py::arg("axis") = INT_MAX)
        .def("repeat", &ndarray_base::repeat_axis)
        .def("round", &ndarray_base::round)
        .def("flatten", &ndarray_base::flatten)
        .def("copy", &ndarray_base::copy)
        .def("astype", &ndarray_base::astype)
        .def("tolist", &ndarray_base::tolist)
        .def("__eq__", &ndarray_base::eq)
        .def("__ne__", &ndarray_base::ne)
        .def("__add__", &ndarray_base::add)
        .def("__add__", &ndarray_base::add_bool)
        .def("__add__", &ndarray_base::add_int)
        .def("__add__", &ndarray_base::add_float)
        .def("__radd__", &ndarray_base::add_bool)
        .def("__radd__", &ndarray_base::add_int)
        .def("__radd__", &ndarray_base::add_float)
        .def("__sub__", &ndarray_base::sub)
        .def("__sub__", &ndarray_base::sub_int)
        .def("__sub__", &ndarray_base::sub_float)
        .def("__rsub__", &ndarray_base::rsub_int)
        .def("__rsub__", &ndarray_base::rsub_float)
        .def("__mul__", &ndarray_base::mul)
        .def("__mul__", &ndarray_base::mul_bool)
        .def("__mul__", &ndarray_base::mul_int)
        .def("__mul__", &ndarray_base::mul_float)
        .def("__rmul__", &ndarray_base::mul_bool)
        .def("__rmul__", &ndarray_base::mul_int)
        .def("__rmul__", &ndarray_base::mul_float)
        .def("__truediv__", &ndarray_base::div)
        .def("__truediv__", &ndarray_base::div_bool)
        .def("__truediv__", &ndarray_base::div_int)
        .def("__truediv__", &ndarray_base::div_float)
        .def("__rtruediv__", &ndarray_base::rdiv_bool)
        .def("__rtruediv__", &ndarray_base::rdiv_int)
        .def("__rtruediv__", &ndarray_base::rdiv_float)
        .def("__matmul__", &ndarray_base::matmul)
        .def("__pow__", &ndarray_base::pow)
        .def("__pow__", &ndarray_base::pow_int)
        .def("__pow__", &ndarray_base::pow_float)
        .def("__rpow__", &ndarray_base::rpow_int)
        .def("__rpow__", &ndarray_base::rpow_float)
        .def("__len__", &ndarray_base::len)
        .def("__and__", &ndarray_base::and_array)
        .def("__and__", &ndarray_base::and_bool)
        .def("__and__", &ndarray_base::and_int)
        .def("__rand__", &ndarray_base::and_bool)
        .def("__rand__", &ndarray_base::and_int)
        .def("__or__", &ndarray_base::or_array)
        .def("__or__", &ndarray_base::or_bool)
        .def("__or__", &ndarray_base::or_int)
        .def("__ror__", &ndarray_base::or_bool)
        .def("__ror__", &ndarray_base::or_int)
        .def("__xor__", &ndarray_base::xor_array)
        .def("__xor__", &ndarray_base::xor_bool)
        .def("__xor__", &ndarray_base::xor_int)
        .def("__rxor__", &ndarray_base::xor_bool)
        .def("__rxor__", &ndarray_base::xor_int)
        .def("__invert__", &ndarray_base::invert)
        .def("__getitem__", &ndarray_base::get_item_int)
        .def("__getitem__", &ndarray_base::get_item_tuple)
        .def("__getitem__", &ndarray_base::get_item_vector)
        .def("__getitem__", &ndarray_base::get_item_slice)
        .def("__setitem__", &ndarray_base::set_item_int)
        .def("__setitem__", &ndarray_base::set_item_index_int)
        .def("__setitem__", &ndarray_base::set_item_index_int_2d)
        .def("__setitem__", &ndarray_base::set_item_index_int_3d)
        .def("__setitem__", &ndarray_base::set_item_index_int_4d)
        .def("__setitem__", &ndarray_base::set_item_float)
        .def("__setitem__", &ndarray_base::set_item_index_float)
        .def("__setitem__", &ndarray_base::set_item_index_float_2d)
        .def("__setitem__", &ndarray_base::set_item_index_float_3d)
        .def("__setitem__", &ndarray_base::set_item_index_float_4d)
        .def("__setitem__", &ndarray_base::set_item_tuple_int1)
        .def("__setitem__", &ndarray_base::set_item_tuple_int2)
        .def("__setitem__", &ndarray_base::set_item_tuple_int3)
        .def("__setitem__", &ndarray_base::set_item_tuple_int4)
        .def("__setitem__", &ndarray_base::set_item_tuple_int5)
        .def("__setitem__", &ndarray_base::set_item_tuple_float1)
        .def("__setitem__", &ndarray_base::set_item_tuple_float2)
        .def("__setitem__", &ndarray_base::set_item_tuple_float3)
        .def("__setitem__", &ndarray_base::set_item_tuple_float4)
        .def("__setitem__", &ndarray_base::set_item_tuple_float5)
        .def("__setitem__", &ndarray_base::set_item_vector_int1)
        .def("__setitem__", &ndarray_base::set_item_vector_int2)
        .def("__setitem__", &ndarray_base::set_item_vector_int3)
        .def("__setitem__", &ndarray_base::set_item_vector_int4)
        .def("__setitem__", &ndarray_base::set_item_vector_int5)
        .def("__setitem__", &ndarray_base::set_item_vector_float1)
        .def("__setitem__", &ndarray_base::set_item_vector_float2)
        .def("__setitem__", &ndarray_base::set_item_vector_float3)
        .def("__setitem__", &ndarray_base::set_item_vector_float4)
        .def("__setitem__", &ndarray_base::set_item_vector_float5)
        .def("__setitem__", &ndarray_base::set_item_slice_int1)
        .def("__setitem__", &ndarray_base::set_item_slice_int2)
        .def("__setitem__", &ndarray_base::set_item_slice_int3)
        .def("__setitem__", &ndarray_base::set_item_slice_int4)
        .def("__setitem__", &ndarray_base::set_item_slice_int5)
        .def("__setitem__", &ndarray_base::set_item_slice_float1)
        .def("__setitem__", &ndarray_base::set_item_slice_float2)
        .def("__setitem__", &ndarray_base::set_item_slice_float3)
        .def("__setitem__", &ndarray_base::set_item_slice_float4)
        .def("__setitem__", &ndarray_base::set_item_slice_float5)

        .def("__str__",
             [](const ndarray_base& self) {
                 std::ostringstream os;
                 os << self.to_string();
                 return os.str();
             })
        .def("__repr__", [](const ndarray_base& self) {
            std::ostringstream os;
            os << "array(" << self.to_string() << ")";
            return os.str();
        });

    py::class_<ndarray<int8>, ndarray_base>(m, "ndarray_int8")
        .def(py::init<>())
        .def(py::init<int8>())
        .def(py::init<const std::vector<int8>&>())
        .def(py::init<const std::vector<std::vector<int8>>&>())
        .def(py::init<const std::vector<std::vector<std::vector<int8>>>&>())
        .def(py::init<const std::vector<std::vector<std::vector<std::vector<int8>>>>&>())
        .def(py::init<const std::vector<std::vector<std::vector<std::vector<std::vector<int8>>>>>&>());

    py::class_<ndarray<int16>, ndarray_base>(m, "ndarray_int16")
        .def(py::init<>())
        .def(py::init<int16>())
        .def(py::init<const std::vector<int16>&>())
        .def(py::init<const std::vector<std::vector<int16>>&>())
        .def(py::init<const std::vector<std::vector<std::vector<int16>>>&>())
        .def(py::init<const std::vector<std::vector<std::vector<std::vector<int16>>>>&>())
        .def(py::init<const std::vector<std::vector<std::vector<std::vector<std::vector<int16>>>>>&>());

    py::class_<ndarray<int32>, ndarray_base>(m, "ndarray_int32")
        .def(py::init<>())
        .def(py::init<int32>())
        .def(py::init<const std::vector<int32>&>())
        .def(py::init<const std::vector<std::vector<int32>>&>())
        .def(py::init<const std::vector<std::vector<std::vector<int32>>>&>())
        .def(py::init<const std::vector<std::vector<std::vector<std::vector<int32>>>>&>())
        .def(py::init<const std::vector<std::vector<std::vector<std::vector<std::vector<int32>>>>>&>());

    py::class_<ndarray<bool_>, ndarray_base>(m, "ndarray_bool")
        .def(py::init<>())
        .def(py::init<bool_>())
        .def(py::init<const std::vector<bool_>&>())
        .def(py::init<const std::vector<std::vector<bool_>>&>())
        .def(py::init<const std::vector<std::vector<std::vector<bool_>>>&>())
        .def(py::init<const std::vector<std::vector<std::vector<std::vector<bool_>>>>&>())
        .def(py::init<const std::vector<std::vector<std::vector<std::vector<std::vector<bool_>>>>>&>());

    py::class_<ndarray<int_>, ndarray_base>(m, "ndarray_int")
        .def(py::init<>())
        .def(py::init<int_>())
        .def(py::init<const std::vector<int_>&>())
        .def(py::init<const std::vector<std::vector<int_>>&>())
        .def(py::init<const std::vector<std::vector<std::vector<int_>>>&>())
        .def(py::init<const std::vector<std::vector<std::vector<std::vector<int_>>>>&>())
        .def(py::init<const std::vector<std::vector<std::vector<std::vector<std::vector<int_>>>>>&>());

    py::class_<ndarray<float32>, ndarray_base>(m, "ndarray_float32")
        .def(py::init<>())
        .def(py::init<float32>())
        .def(py::init<const std::vector<float32>&>())
        .def(py::init<const std::vector<std::vector<float32>>&>())
        .def(py::init<const std::vector<std::vector<std::vector<float32>>>&>())
        .def(py::init<const std::vector<std::vector<std::vector<std::vector<float32>>>>&>())
        .def(py::init<const std::vector<std::vector<std::vector<std::vector<std::vector<float32>>>>>&>());

    py::class_<ndarray<float64>, ndarray_base>(m, "ndarray_float")
        .def(py::init<>())
        .def(py::init<float64>())
        .def(py::init<const std::vector<float64>&>())
        .def(py::init<const std::vector<std::vector<float64>>&>())
        .def(py::init<const std::vector<std::vector<std::vector<float64>>>&>())
        .def(py::init<const std::vector<std::vector<std::vector<std::vector<float64>>>>&>())
        .def(py::init<const std::vector<std::vector<std::vector<std::vector<std::vector<float64>>>>>&>());

    py::class_<Random>(m, "random")
        .def_static("rand", &Random::rand)
        .def_static("rand_shape", &Random::rand_shape)
        .def_static("randn", &Random::randn)
        .def_static("randn_shape", &Random::randn_shape)
        .def_static("randint", &Random::randint)
        .def_static("randint_shape", &Random::randint_shape)
        .def_static("uniform", &Random::uniform);

    array_creation_registry(m);

    m.def("array", [](bool_ value) {
    return std::unique_ptr<ndarray_base>(new ndarray_bool(value));
    });
    m.def("array", [](const std::vector<bool_>& values) {
    return std::unique_ptr<ndarray_base>(new ndarray_bool(values));
    });
    m.def("array", [](const std::vector<std::vector<bool_>>& values) {
    return std::unique_ptr<ndarray_base>(new ndarray_bool(values));
    });
    m.def("array", [](const std::vector<std::vector<std::vector<bool_>>>& values) {
    return std::unique_ptr<ndarray_base>(new ndarray_bool(values));
    });
    m.def("array", [](const std::vector<std::vector<std::vector<std::vector<bool_>>>>& values) {
    return std::unique_ptr<ndarray_base>(new ndarray_bool(values));
    });
    m.def("array", [](const std::vector<std::vector<std::vector<std::vector<std::vector<bool_>>>>>& values) {
    return std::unique_ptr<ndarray_base>(new ndarray_bool(values));
    });

    m.def("array", [](int8 value) {
    return std::unique_ptr<ndarray_base>(new ndarray_int8(value));
    });
    m.def("array", [](const std::vector<int8>& values) {
    return std::unique_ptr<ndarray_base>(new ndarray_int8(values));
    });
    m.def("array", [](const std::vector<std::vector<int8>>& values) {
    return std::unique_ptr<ndarray_base>(new ndarray_int8(values));
    });
    m.def("array", [](const std::vector<std::vector<std::vector<int8>>>& values) {
    return std::unique_ptr<ndarray_base>(new ndarray_int8(values));
    });
    m.def("array", [](const std::vector<std::vector<std::vector<std::vector<int8>>>>& values) {
    return std::unique_ptr<ndarray_base>(new ndarray_int8(values));
    });
    m.def("array", [](const std::vector<std::vector<std::vector<std::vector<std::vector<int8>>>>>& values) {
    return std::unique_ptr<ndarray_base>(new ndarray_int8(values));
    });

    m.def("array", [](int16 value) {
    return std::unique_ptr<ndarray_base>(new ndarray_int16(value));
    });
    m.def("array", [](const std::vector<int16>& values) {
    return std::unique_ptr<ndarray_base>(new ndarray_int16(values));
    });
    m.def("array", [](const std::vector<std::vector<int16>>& values) {
    return std::unique_ptr<ndarray_base>(new ndarray_int16(values));
    });
    m.def("array", [](const std::vector<std::vector<std::vector<int16>>>& values) {
    return std::unique_ptr<ndarray_base>(new ndarray_int16(values));
    });
    m.def("array", [](const std::vector<std::vector<std::vector<std::vector<int16>>>>& values) {
    return std::unique_ptr<ndarray_base>(new ndarray_int16(values));
    });
    m.def("array", [](const std::vector<std::vector<std::vector<std::vector<std::vector<int16>>>>>& values) {
    return std::unique_ptr<ndarray_base>(new ndarray_int16(values));
    });

    m.def("array", [](int32 value) {
    return std::unique_ptr<ndarray_base>(new ndarray_int32(value));
    });
    m.def("array", [](const std::vector<int32>& values) {
    return std::unique_ptr<ndarray_base>(new ndarray_int32(values));
    });
    m.def("array", [](const std::vector<std::vector<int32>>& values) {
    return std::unique_ptr<ndarray_base>(new ndarray_int32(values));
    });
    m.def("array", [](const std::vector<std::vector<std::vector<int32>>>& values) {
    return std::unique_ptr<ndarray_base>(new ndarray_int32(values));
    });
    m.def("array", [](const std::vector<std::vector<std::vector<std::vector<int32>>>>& values) {
    return std::unique_ptr<ndarray_base>(new ndarray_int32(values));
    });
    m.def("array", [](const std::vector<std::vector<std::vector<std::vector<std::vector<int32>>>>>& values) {
    return std::unique_ptr<ndarray_base>(new ndarray_int32(values));
    });

    m.def("array", [](float32 value) {
    return std::unique_ptr<ndarray_base>(new ndarray_float32(value));
    });
    m.def("array", [](const std::vector<float32>& values) {
    return std::unique_ptr<ndarray_base>(new ndarray_float32(values));
    });
    m.def("array", [](const std::vector<std::vector<float32>>& values) {
    return std::unique_ptr<ndarray_base>(new ndarray_float32(values));
    });
    m.def("array", [](const std::vector<std::vector<std::vector<float32>>>& values) {
    return std::unique_ptr<ndarray_base>(new ndarray_float32(values));
    });
    m.def("array", [](const std::vector<std::vector<std::vector<std::vector<float32>>>>& values) {
    return std::unique_ptr<ndarray_base>(new ndarray_float32(values));
    });
    m.def("array", [](const std::vector<std::vector<std::vector<std::vector<std::vector<float32>>>>>& values) {
    return std::unique_ptr<ndarray_base>(new ndarray_float32(values));
    });

    // Array Creation Functions
    m.def("ones", [](const std::vector<int>& shape) {
        return std::unique_ptr<ndarray_base>(new ndarray_float(pkpy::numpy::ones<float64>(shape)));
    });
    m.def("zeros", [](const std::vector<int>& shape) {
        return std::unique_ptr<ndarray_base>(new ndarray_float(pkpy::numpy::zeros<float64>(shape)));
    });
    m.def("full", [](const std::vector<int>& shape, int_ value) {
        return std::unique_ptr<ndarray_base>(new ndarray_int(pkpy::numpy::full<int_>(shape, value)));
    });
    m.def("full", [](const std::vector<int>& shape, float64 value) {
        return std::unique_ptr<ndarray_base>(new ndarray_float(pkpy::numpy::full<float64>(shape, value)));
    });
    m.def("identity", [](int n) {
        return std::unique_ptr<ndarray_base>(new ndarray_float(pkpy::numpy::identity<float64>(n)));
    });
    m.def("arange", [](int_ stop) {
        return std::unique_ptr<ndarray_base>(new ndarray_int(pkpy::numpy::arange<int_>(0, stop)));
    });
    m.def("arange", [](int_ start, int_ stop) {
        return std::unique_ptr<ndarray_base>(new ndarray_int(pkpy::numpy::arange<int_>(start, stop)));
    });
    m.def("arange", [](int_ start, int_ stop, int_ step) {
        return std::unique_ptr<ndarray_base>(new ndarray_int(pkpy::numpy::arange<int_>(start, stop, step)));
    });
    m.def("arange", [](float64 stop) {
        return std::unique_ptr<ndarray_base>(new ndarray_float(pkpy::numpy::arange<float64>(0, stop)));
    });
    m.def("arange", [](float64 start, float64 stop) {
        return std::unique_ptr<ndarray_base>(new ndarray_float(pkpy::numpy::arange<float64>(start, stop)));
    });
    m.def("arange", [](float64 start, float64 stop, float64 step) {
        return std::unique_ptr<ndarray_base>(new ndarray_float(pkpy::numpy::arange<float64>(start, stop, step)));
    });
    m.def(
        "linspace",
        [](float64 start, float64 stop, int num, bool endpoint) {
            return std::unique_ptr<ndarray_base>(new ndarray_float(pkpy::numpy::linspace(start, stop, num, endpoint)));
        },
        py::arg("start"),
        py::arg("stop"),
        py::arg("num") = 50,
        py::arg("endpoint") = true);

    // Trigonometric Functions
    m.def("sin", [](const ndarray_base& arr) {
        if (auto p = dynamic_cast<const ndarray<int8>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::sin(p->data)));
        } else if (auto p = dynamic_cast<const ndarray<int16>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::sin(p->data)));
        } else if (auto p = dynamic_cast<const ndarray<int32>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::sin(p->data)));
        } else if (auto p = dynamic_cast<const ndarray<int_>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::sin(p->data)));
        } else if (auto p = dynamic_cast<const ndarray<float32>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::sin(p->data)));
        } else if (auto p = dynamic_cast<const ndarray<float64>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::sin(p->data)));
        }
        throw std::invalid_argument("Invalid dtype");
    });
    m.def("cos", [](const ndarray_base& arr) {
        if(auto p = dynamic_cast<const ndarray<int8>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::cos(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<int16>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::cos(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<int32>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::cos(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<int_>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::cos(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<float32>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::cos(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<float64>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::cos(p->data)));
        }
        throw std::invalid_argument("Invalid dtype");
    });
    m.def("tan", [](const ndarray_base& arr) {
        if(auto p = dynamic_cast<const ndarray<int8>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::tan(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<int16>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::tan(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<int32>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::tan(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<int_>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::tan(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<float32>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::tan(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<float64>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::tan(p->data)));
        }
        throw std::invalid_argument("Invalid dtype");
    });
    m.def("arcsin", [](const ndarray_base& arr) {
        if(auto p = dynamic_cast<const ndarray<int8>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::arcsin(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<int16>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::arcsin(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<int32>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::arcsin(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<int_>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::arcsin(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<float32>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::arcsin(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<float64>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::arcsin(p->data)));
        }
        throw std::invalid_argument("Invalid dtype");
    });
    m.def("arccos", [](const ndarray_base& arr) {
        if(auto p = dynamic_cast<const ndarray<int8>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::arccos(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<int16>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::arccos(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<int32>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::arccos(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<int_>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::arccos(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<float32>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::arccos(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<float64>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::arccos(p->data)));
        }
        throw std::invalid_argument("Invalid dtype");
    });
    m.def("arctan", [](const ndarray_base& arr) {
        if(auto p = dynamic_cast<const ndarray<int8>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::arctan(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<int16>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::arctan(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<int32>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::arctan(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<int_>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::arctan(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<float32>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::arctan(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<float64>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::arctan(p->data)));
        }
        throw std::invalid_argument("Invalid dtype");
    });

    // Exponential and Logarithmic Functions
    m.def("exp", [](const ndarray_base& arr) {
        if(auto p = dynamic_cast<const ndarray<int8>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::exp(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<int16>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::exp(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<int32>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::exp(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<int_>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::exp(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<float32>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::exp(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<float64>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::exp(p->data)));
        }
        throw std::invalid_argument("Invalid dtype");
    });
    m.def("log", [](const ndarray_base& arr) {
        if(auto p = dynamic_cast<const ndarray<int8>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::log(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<int16>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::log(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<int32>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::log(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<int_>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::log(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<float32>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::log(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<float64>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::log(p->data)));
        }
        throw std::invalid_argument("Invalid dtype");
    });
    m.def("log2", [](const ndarray_base& arr) {
        if(auto p = dynamic_cast<const ndarray<int8>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::log2(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<int16>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::log2(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<int32>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::log2(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<int_>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::log2(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<float32>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::log2(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<float64>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::log2(p->data)));
        }
        throw std::invalid_argument("Invalid dtype");
    });
    m.def("log10", [](const ndarray_base& arr) {
        if(auto p = dynamic_cast<const ndarray<int8>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::log10(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<int16>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::log10(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<int32>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::log10(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<int_>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::log10(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<float32>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::log10(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<float64>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::log10(p->data)));
        }
        throw std::invalid_argument("Invalid dtype");
    });

    // Miscellaneous Functions
    m.def("round", [](const ndarray_base& arr) {
        if(auto p = dynamic_cast<const ndarray<int8>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<int8>(pkpy::numpy::round(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<int16>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<int16>(pkpy::numpy::round(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<int32>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<int32>(pkpy::numpy::round(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<int_>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<int_>(pkpy::numpy::round(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<float32>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float32>(pkpy::numpy::round(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<float64>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::round(p->data)));
        }
        throw std::invalid_argument("Invalid dtype");
    });
    m.def("floor", [](const ndarray_base& arr) {
        if(auto p = dynamic_cast<const ndarray<int8>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<int8>(pkpy::numpy::floor(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<int16>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<int16>(pkpy::numpy::floor(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<int32>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<int32>(pkpy::numpy::floor(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<int_>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<int_>(pkpy::numpy::floor(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<float32>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float32>(pkpy::numpy::floor(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<float64>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::floor(p->data)));
        }
        throw std::invalid_argument("Invalid dtype");
    });
    m.def("ceil", [](const ndarray_base& arr) {
        if(auto p = dynamic_cast<const ndarray<int8>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<int8>(pkpy::numpy::ceil(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<int16>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<int16>(pkpy::numpy::ceil(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<int32>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<int32>(pkpy::numpy::ceil(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<int_>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<int_>(pkpy::numpy::ceil(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<float32>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float32>(pkpy::numpy::ceil(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<float64>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::ceil(p->data)));
        }
        throw std::invalid_argument("Invalid dtype");
    });
    m.def("abs", [](const ndarray_base& arr) {
        if(auto p = dynamic_cast<const ndarray<int8>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<int8>(pkpy::numpy::abs(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<int16>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<int16>(pkpy::numpy::abs(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<int32>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<int32>(pkpy::numpy::abs(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<int_>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<int_>(pkpy::numpy::abs(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<float32>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float32>(pkpy::numpy::abs(p->data)));
        } else if(auto p = dynamic_cast<const ndarray<float64>*>(&arr)) {
            return std::unique_ptr<ndarray_base>(new ndarray<float64>(pkpy::numpy::abs(p->data)));
        }
        throw std::invalid_argument("Invalid dtype");
    });
    m.def(
        "concatenate",
        [](const ndarray_base& arr1, const ndarray_base& arr2, int axis) {
            if(auto p = dynamic_cast<const ndarray<int_>*>(&arr1)) {
                if(auto q = dynamic_cast<const ndarray<int_>*>(&arr2)) {
                    return std::unique_ptr<ndarray_base>(
                        new ndarray<int_>(pkpy::numpy::concatenate(p->data, q->data, axis)));
                } else if(auto q = dynamic_cast<const ndarray<float64>*>(&arr2)) {
                    return std::unique_ptr<ndarray_base>(
                        new ndarray<float64>(pkpy::numpy::concatenate(p->data, q->data, axis)));
                }
            } else if(auto p = dynamic_cast<const ndarray<float64>*>(&arr1)) {
                if(auto q = dynamic_cast<const ndarray<int_>*>(&arr2)) {
                    return std::unique_ptr<ndarray_base>(
                        new ndarray<float64>(pkpy::numpy::concatenate(p->data, q->data, axis)));
                } else if(auto q = dynamic_cast<const ndarray<float64>*>(&arr2)) {
                    return std::unique_ptr<ndarray_base>(
                        new ndarray<float64>(pkpy::numpy::concatenate(p->data, q->data, axis)));
                }
            }
            throw std::invalid_argument("Invalid dtype");
        },
        py::arg("arr1"),
        py::arg("arr2"),
        py::arg("axis") = 0);

    // Constants
    m.attr("pi") = pkpy::numpy::pi;
    m.attr("inf") = pkpy::numpy::inf;

    // Testing Functions
    m.def(
        "allclose",
        [](const ndarray_base& arr1, const ndarray_base& arr2, float64 rtol, float64 atol) {
            if(auto p = dynamic_cast<const ndarray<int_>*>(&arr1)) {
                if(auto q = dynamic_cast<const ndarray<int_>*>(&arr2)) {
                    return pkpy::numpy::allclose(p->data, q->data, rtol, atol);
                } else if(auto q = dynamic_cast<const ndarray<float64>*>(&arr2)) {
                    return pkpy::numpy::allclose(p->data, q->data, rtol, atol);
                }
            } else if(auto p = dynamic_cast<const ndarray<float64>*>(&arr1)) {
                if(auto q = dynamic_cast<const ndarray<int_>*>(&arr2)) {
                    return pkpy::numpy::allclose(p->data, q->data, rtol, atol);
                } else if(auto q = dynamic_cast<const ndarray<float64>*>(&arr2)) {
                    return pkpy::numpy::allclose(p->data, q->data, rtol, atol);
                }
            }
            throw std::invalid_argument("Invalid dtype");
        },
        py::arg("arr1"),
        py::arg("arr2"),
        py::arg("rtol") = 1e-5,
        py::arg("atol") = 1e-8);
}
