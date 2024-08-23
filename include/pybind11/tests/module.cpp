#include "test.h"

PYBIND11_EMBEDDED_MODULE(example, m) {
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

    auto math = py::module_::import("example.math");
    EXPECT_EQ(math.attr("sub")(4, 3).cast<int>(), 1);
}

}  // namespace

