#include <gtest/gtest.h>
#include <pybind11/pkbind.h>

namespace py = pkbind;

class PYBIND11_TEST : public ::testing::Test {
protected:
    void SetUp() override { py::initialize(); }

    void TearDown() override { py::finalize(true); }
};

#define EXPECT_CAST_EQ(expr, expected) EXPECT_EQ(py::cast(expr), py::cast(expected))
#define EXPECT_EVAL_EQ(expr, expected) EXPECT_EQ(py::eval(expr).cast<decltype(expected)>(), expected)
#define EXPECT_EXEC_EQ(expr, expected) EXPECT_EQ(py::eval(expr), py::eval(expected))

