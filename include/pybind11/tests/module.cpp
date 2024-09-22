#include "test.h"
#include <gtest/gtest.h>

PYBIND11_EMBEDDED_MODULE(example, m) {
    m.def("add", [](int a, int b) {
        return a + b;
    });

    auto math = m.def_submodule("math");
    math.def("sub", [](int a, int b) {
        return a - b;
    });
}

PYBIND11_MODULE(example3, m) {
    m.def("add", [](int a, int b) {
        return a + b;
    });

    auto math = m.def_submodule("math");
    math.def("sub", [](int a, int b) {
        return a - b;
    });
}

namespace {

TEST_F(PYBIND11_TEST, module) {
    py::exec("import example");
    EXPECT_EVAL_EQ("example.add(1, 2)", 3);

    py::exec("from example import math");
    EXPECT_EVAL_EQ("math.sub(1, 2)", -1);

    py::exec("from example.math import sub");
    EXPECT_EVAL_EQ("sub(1, 2)", -1);

    auto math = py::module::import("example.math");
    EXPECT_EQ(math.attr("sub")(4, 3).cast<int>(), 1);
}

TEST_F(PYBIND11_TEST, raw_module) {
    auto m = py::module::create("example2");
    m.def("add", [](int a, int b) {
        return a + b;
    });

    auto math = m.def_submodule("math");
    math.def("sub", [](int a, int b) {
        return a - b;
    });

    py::exec("import example2");
    EXPECT_EVAL_EQ("example2.add(1, 2)", 3);

    py::exec("from example2 import math");
    EXPECT_EVAL_EQ("math.sub(1, 2)", -1);

    py::exec("from example2.math import sub");
    EXPECT_EVAL_EQ("sub(1, 2)", -1);

    auto math2 = py::module::import("example2.math");
    EXPECT_EQ(math2.attr("sub")(4, 3).cast<int>(), 1);
}

TEST_F(PYBIND11_TEST, dynamic_module) {
    py_module_initialize();

    py::exec("import example3");
    EXPECT_EVAL_EQ("example3.add(1, 2)", 3);

    py::exec("from example3 import math");
    EXPECT_EVAL_EQ("math.sub(1, 2)", -1);

    py::exec("from example3.math import sub");
    EXPECT_EVAL_EQ("sub(1, 2)", -1);

    auto math = py::module::import("example3.math");
    EXPECT_EQ(math.attr("sub")(4, 3).cast<int>(), 1);
}

}  // namespace
