# module

```cpp
#include <pybind11/pybind11.h>
namespace py = pybind11;

PYBIND11_MODULE(example, m) {
    m.def("add", [](int a, int b) {
        return a + b;
    });

    auto math = m.def_submodule("math");
}
```

# function

```cpp
int add(int a, int b) { return a + b; }

int add(int a, int b, int c) { return a + b + c; }

void register_function(py::module_& m)
{
    m.def("add", py::overload_cast<int, int>(&add));

    // support function overload
    m.def("add", py::overload_cast<int, int, int>(&add));

    // bind with default arguments
    m.def("sub", [](int a, int b) { 
        return a - b; 
    }, py::arg("a") = 1, py::arg("b") = 2);

    // bind *args
    m.def("add", [](py::args args) {
        int sum = 0;
        for (auto& arg : args) {
            sum += arg.cast<int>();
        }
        return sum;
    });

    // bind **kwargs
    m.def("add", [](py::kwargs kwargs) {
        int sum = 0;
        for (auto item : kwargs) {
            sum += item.second.cast<int>();
        }
        return sum;
    });
}
```

# class

```cpp
struct Point
{
    const int x;
    int y;

public:
    Point() : x(0), y(0) {}

    Point(int x, int y) : x(x), y(y) {}

    Point(const Point& p) : x(p.x), y(p.y) {}

    std::string stringfy() const { 
        return "(" + std::to_string(x) + ", " + std::to_string(y) + ")"; 
    }
};

struct Point3D : Point
{
private:
    int z;

public:
    Point3D(int x, int y, int z) : Point(x, y), z(z) {}

    int get_z() const { return z; }

    void set_z(int z) { this->z = z; }
};

void bind_class(py::module_& m)
{
    py::class_<Point>(m, "Point")
        .def(py::init<>())
        .def(py::init<int, int>())
        .def(py::init<const Point&>())
        .def_readonly("x", &Point::x)
        .def_readwrite("y", &Point::y)
        .def("__str__", &Point::stringfy);

    // only support single inheritance
    py::class_<Point3D, Point>(m, "Point3D", py::dynamic_attr())
        .def(py::init<int, int, int>())
        .def_property("z", &Point3D::get_z, &Point3D::set_z);

    // dynamic_attr will enable the dict of bound class
}
```

# operators

```cpp
#include <pybind11/operators.h>

namespace py = pybind11;

struct Int {
    int value;

    Int(int value) : value(value) {}

    Int operator+(const Int& other) const {
        return Int(value + other.value);
    }

    Int operator-(const Int& other) const {
        return Int(value - other.value);
    }

    bool operator==(const Int& other) const {
        return value == other.value;
    }

    bool operator!=(const Int& other) const {
        return value != other.value;
    }
};

void bind_operators(py::module_& m)
{
    py::class_<Int>(m, "Int")
        .def(py::init<int>())
        .def(py::self + py::self)
        .def(py::self - py::self)
        .def(py::self == py::self)
        .def(py::self != py::self);
}
```

# py::object

`py::object` is just simple wrapper around `PyVar`. It supports some convenient methods to interact with Python objects.


here are some common methods:

```cpp
obj.attr("x"); // access attribute
obj[1]; // access item

obj.is_none(); // same as obj is None in Python
obj.is(obj2); // same as obj is obj2 in Python

// operators
obj + obj2; // same as obj + obj2 in Python
// ...
obj == obj2; // same as obj == obj2 in Python
// ...

obj(...); // same as obj.__call__(...)

py::cast(obj); // cast to Python object
obj.cast<T>; // cast to C++ type

py::type::of(obj); // get type of obj
py::type::of<T>(); // get type of T, if T is registered
```

you can also create some builtin objects with their according wrappers:

```cpp
py::bool_ b = {true};
py::int_ i = {1};
py::float_ f = {1.0};
py::str s = {"hello"};
py::list l = {1, 2, 3};
py::tuple t = {1, 2, 3};
// ...
```

More examples please see the test [folder](https://github.com/pocketpy/gsoc-2024-dev/tree/main/pybind11/tests) in the GSoC repository. All tested features are supported.