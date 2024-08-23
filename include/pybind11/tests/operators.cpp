#include "test.h"
#include <pybind11/operators.h>

namespace {

struct Int {
    int x;

    Int(int x) : x(x) {}

#define OPERATOR_IMPL(op)                                                                                              \
    template <typename LHS, typename RHS>                                                                              \
    friend int operator op (const LHS& lhs, const RHS& rhs) {                                                          \
        int l, r;                                                                                                      \
        if constexpr(std::is_same_v<LHS, Int>)                                                                         \
            l = lhs.x;                                                                                                 \
        else                                                                                                           \
            l = lhs;                                                                                                   \
        if constexpr(std::is_same_v<RHS, Int>)                                                                         \
            r = rhs.x;                                                                                                 \
        else                                                                                                           \
            r = rhs;                                                                                                   \
        return l op r;                                                                                                 \
    }

    OPERATOR_IMPL(+)
    OPERATOR_IMPL(-)
    OPERATOR_IMPL(*)
    OPERATOR_IMPL(/)
    OPERATOR_IMPL(%)
    OPERATOR_IMPL(&)
    OPERATOR_IMPL(|)
    OPERATOR_IMPL(^)
    OPERATOR_IMPL(<<)
    OPERATOR_IMPL(>>)

#undef OPERATOR_IMPL

    bool operator== (const Int& other) const { return x == other.x; }

    bool operator!= (const Int& other) const { return x != other.x; }

    bool operator< (const Int& other) const { return x < other.x; }

    bool operator<= (const Int& other) const { return x <= other.x; }

    bool operator> (const Int& other) const { return x > other.x; }

    bool operator>= (const Int& other) const { return x >= other.x; }

    bool operator!() const { return !x; }
};

}  // namespace

TEST_F(PYBIND11_TEST, arithmetic_operators) {
    py::module_ m = py::module_::import("__main__");
    py::class_<Int>(m, "Int")
        .def(py::init<int>())
        .def(py::self + py::self)
        .def(py::self + int())
        .def(int() + py::self)
        .def(py::self - py::self)
        .def(py::self - int())
        .def(int() - py::self)
        .def(py::self * py::self)
        .def(py::self * int())
        .def(int() * py::self)
        .def(py::self / py::self)
        .def(py::self / int())
        .def(int() / py::self)
        .def(py::self % py::self)
        .def(py::self % int())
        .def(int() % py::self)
        .def(py::self & py::self)
        .def(py::self & int())
        .def(int() & py::self)
        .def(py::self | py::self)
        .def(py::self | int())
        .def(int() | py::self)
        .def(py::self ^ py::self)
        .def(py::self ^ int())
        .def(int() ^ py::self)
        .def(py::self << py::self)
        .def(py::self << int())
        .def(int() << py::self)
        .def(py::self >> py::self)
        .def(py::self >> int())
        .def(int() >> py::self);

    auto a = py::cast(Int(1));
    auto ai = py::cast(1);
    auto b = py::cast(Int(2));
    auto bi = py::cast(2);

    EXPECT_CAST_EQ(a + b, 3);
    EXPECT_CAST_EQ(a + bi, 3);
    EXPECT_CAST_EQ(ai + b, 3);
    EXPECT_CAST_EQ(a - b, -1);
    EXPECT_CAST_EQ(a - bi, -1);
    EXPECT_CAST_EQ(ai - b, -1);
    EXPECT_CAST_EQ(a * b, 2);
    EXPECT_CAST_EQ(a * bi, 2);
    EXPECT_CAST_EQ(ai * b, 2);
    EXPECT_CAST_EQ(a / b, 0);
    EXPECT_CAST_EQ(a / bi, 0);
    // EXPECT_CAST_EQ(ai / b, 0);
    EXPECT_CAST_EQ(a % b, 1);
    EXPECT_CAST_EQ(a % bi, 1);
    // EXPECT_CAST_EQ(ai % b, 1);
    EXPECT_CAST_EQ(a & b, 0);
    EXPECT_CAST_EQ(a & bi, 0);
    // EXPECT_CAST_EQ(ai & b, 0);
    EXPECT_CAST_EQ(a | b, 3);
    EXPECT_CAST_EQ(a | bi, 3);
    // EXPECT_CAST_EQ(ai | b, 3);
    EXPECT_CAST_EQ(a ^ b, 3);
    EXPECT_CAST_EQ(a ^ bi, 3);
    // EXPECT_CAST_EQ(ai ^ b, 3);
    EXPECT_CAST_EQ(a << b, 4);
    EXPECT_CAST_EQ(a << bi, 4);
    // EXPECT_CAST_EQ(ai << b, 4);
    EXPECT_CAST_EQ(a >> b, 0);
    EXPECT_CAST_EQ(a >> bi, 0);
    // EXPECT_CAST_EQ(ai >> b, 0);
}

TEST_F(PYBIND11_TEST, logic_operators) {
    py::module_ m = py::module_::import("__main__");
    py::class_<Int>(m, "Int")
        .def(py::init<int>())
        .def_readwrite("x", &Int::x)
        .def(py::self == py::self)
        .def(py::self != py::self)
        .def(py::self < py::self)
        .def(py::self <= py::self)
        .def(py::self > py::self)
        .def(py::self >= py::self);

    auto a = py::cast(Int(1));
    auto b = py::cast(Int(2));

    EXPECT_FALSE(a == b);
    EXPECT_TRUE(a != b);
    EXPECT_TRUE(a < b);
    EXPECT_TRUE(a <= b);
    EXPECT_FALSE(a > b);
    EXPECT_FALSE(a >= b);
}

