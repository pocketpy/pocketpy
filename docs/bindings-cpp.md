---
icon: cpu
title: C++ Bindings
order: 16
---

# C++ bindings

pocketpy bundles a **C++17 binding layer with a pybind11-style API** in
`include/pybind11/`. Use these headers with the pocketpy library.
It implements a subset of pybind11 and does not use CPython's extension ABI.

## A complete embedded module

Save this as `main.cpp`. Link it to the `pocketpy` CMake target as in
[quick start](quick-start.md), and set
`target_compile_features(my_app PRIVATE cxx_std_17)`.

```cpp
#include <pybind11/embed.h>
#include <iostream>

namespace py = pybind11;

struct Point {
    double x;
    double y;

    Point(double x, double y) : x(x), y(y) {}
    double squared_length() const { return x * x + y * y; }
};

PYBIND11_EMBEDDED_MODULE(example, m) {
    m.def("scale", [](double value, double factor) {
        return value * factor;
    }, py::arg("value"), py::arg("factor") = 2.0);

    py::class_<Point>(m, "Point")
        .def(py::init<double, double>())
        .def_readwrite("x", &Point::x)
        .def_readwrite("y", &Point::y)
        .def("squared_length", &Point::squared_length);
}

int main() {
    py::scoped_interpreter guard{};
    try {
        py::exec(R"(
from example import Point, scale
point = Point(3.0, 4.0)
assert point.squared_length() == 25.0
assert scale(3.0) == 6.0
assert scale(3.0, factor=4.0) == 12.0
)");
    } catch(const py::python_error& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
    return 0;
}
```

The module macro registers initialization code that runs when the interpreter
starts. Create the guard before any Python object and destroy all Python
objects before the guard. Use one interpreter lifetime per process.

The explicit lifecycle functions are `py::initialize()` and
`py::finalize()`. If using the binding layer, initialize through this layer
so that embedded-module registrations are run.

## Functions and overloads

Use function pointers or lambdas with `m.def()`. Register argument defaults
with `py::arg`; C++ default arguments alone do not provide Python defaults.

For an overloaded C++ function, select the overload explicitly:

```cpp
int add(int a, int b) { return a + b; }
double add(double a, double b) { return a + b; }

void bind_add(py::module_& m) {
    m.def("add", py::overload_cast<int, int>(&add));
    m.def("add", py::overload_cast<double, double>(&add));
}
```

The example assumes values whose integer sum fits in C++ `int`.
For variable arguments, accept `py::args` or `py::kwargs`.
The repository's
[function tests](https://github.com/pocketpy/pocketpy/blob/main/include/pybind11/tests/function.cpp)
show supported combinations.

## Classes and properties

| Binding | Purpose |
| --- | --- |
| `.def(py::init<Args...>())` | Expose a constructor. |
| `.def("name", &T::method)` | Expose an instance method. |
| `.def_readwrite("name", &T::field)` | Read/write field. |
| `.def_readonly("name", &T::field)` | Read-only field. |
| `.def_property("name", getter, setter)` | Computed property. |
| `py::class_<Derived, Base>` | Register single inheritance. |
| `py::dynamic_attr()` | Allow additional Python attributes on an instance. |

Use `<pybind11/operators.h>` for bindings such as
`.def(py::self + py::self)`. The operator must also be supported by
[pocketpy's object model](features/basic.md#special-methods).

For pointer and reference results, choose the return-value policy to match
ownership. `reference` leaves ownership with C++; `take_ownership` transfers
it to the Python wrapper; `reference_internal` associates the result with its
parent. Never return a reference to a local C++ variable.

## Work with Python values

This fragment runs while a `py::scoped_interpreter` is alive:

```cpp
py::object result = py::eval("sum([1, 2, 3])");
int total = result.cast<int>();               // 6
py::object math = py::module_::import("math");
double root = math.attr("sqrt")(9.0).cast<double>();  // 3.0
py::list values;
values.append(py::int_(1));
values.append(py::int_(2));
```

Use `obj.attr("name")` for attributes, `obj[key]` for items,
`obj(...)` for calls, `obj.cast<T>()` to convert to C++, and
`py::cast(value)` to construct a Python value from C++.
`py::object` manages a rooted handle; a plain `py::handle` does not own one.

Include `<pybind11/stl.h>` for supported STL container conversions, including
vectors and maps. These conversions copy container contents; they are not
shared views into the C++ container.

## Compatibility boundaries

The bundled headers and
[tests](https://github.com/pocketpy/pocketpy/tree/main/include/pybind11/tests)
are the reference for supported features. In particular:

- Multiple inheritance, CPython's buffer protocol, and GIL APIs are unavailable.
- The bundled conversion layer does not provide the full upstream smart-pointer,
  NumPy, chrono, or callable conversion ecosystem.
- Python syntax and behavior follow the [pocketpy compatibility guide](features/differences.md),
  including positional argument rules and in-place special-method limitations.
- The binding layer maintains process-wide object-pool and type-registration
  state. Do not assume its handles can be shared across VMs or used concurrently.

Calls such as `py::exec()` translate a Python failure to `py::python_error`,
whose `what()` contains the traceback. Other binding operations can use
`py::error_already_set` to signal a pending VM exception. Follow the
[exception tests](https://github.com/pocketpy/pocketpy/blob/main/include/pybind11/tests/error.cpp)
when adding custom exception handling.
