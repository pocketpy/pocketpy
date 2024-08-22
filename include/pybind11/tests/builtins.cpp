#include "test.h"

namespace {

int copy_constructor_calls = 0;
int move_constructor_calls = 0;
int destructor_calls = 0;

struct Point {
    int x;
    int y;

    Point(int x, int y) : x(x), y(y) {}

    Point(const Point& other) : x(other.x), y(other.y) { ++copy_constructor_calls; }

    Point(Point&& other) : x(other.x), y(other.y) { ++move_constructor_calls; }

    ~Point() { ++destructor_calls; }

    bool operator== (const Point& p) const { return x == p.x && y == p.y; }
};

TEST_F(PYBIND11_TEST, exec_and_eval) {
    auto m = py::module::__main__();

    py::dict locals = {py::arg("x") = 1, py::arg("y") = 2};
    py::object obj = py::eval("x + y", py::none{}, locals);
    EXPECT_EQ(obj.cast<int>(), 3);

    py::exec("x = 1 + 2");
    EXPECT_EQ(py::eval("x").cast<int>(), 3);

    py::exec("y = 1 + 2", m.attr("__dict__"));
    EXPECT_EQ(py::eval("y", m.attr("__dict__")).cast<int>(), 3);

    EXPECT_EQ(locals["x"].cast<int>(), 1);
    EXPECT_EQ(locals["y"].cast<int>(), 2);
}

TEST_F(PYBIND11_TEST, locals_and_globals) {
    py::exec("x = 1");

    auto globals = py::globals();
    EXPECT_EQ(globals["x"].cast<int>(), 1);

    globals["y"] = 2;
    EXPECT_EQ(py::eval("y").cast<int>(), 2);
}

TEST_F(PYBIND11_TEST, cast) {
    auto m = py::module::__main__();

    py::class_<Point>(m, "Point")
        .def(py::init<int, int>())
        .def_readwrite("x", &Point::x)
        .def_readwrite("y", &Point::y)
        .def("__eq__", &Point::operator==);

    Point p(1, 2);

    // for py::cast's default policy

    // if argument is lvalue, policy is copy
    py::object o = py::cast(p);
    EXPECT_EQ(py::cast<Point&>(o), p);
    EXPECT_EQ(copy_constructor_calls, 1);
    EXPECT_EQ(move_constructor_calls, 0);

    // if argument is rvalue, policy is move
    py::object o2 = py::cast(std::move(p));
    EXPECT_EQ(py::cast<Point&>(o2), p);
    EXPECT_EQ(copy_constructor_calls, 1);
    EXPECT_EQ(move_constructor_calls, 1);

    // if argument is pointer, policy is reference(no taking ownership)
    py::object o3 = py::cast(&p);
    EXPECT_EQ(py::cast<Point&>(o3), p);
    EXPECT_EQ(copy_constructor_calls, 1);
    EXPECT_EQ(move_constructor_calls, 1);

    py::finalize(true);

    EXPECT_EQ(destructor_calls, 2);
}

TEST_F(PYBIND11_TEST, cpp_call_py) {
    auto m = py::module::__main__();

    py::exec(R"(
class Test:
    def __init__(self, x, y):
        self.x = x
        self.y = y

    def sum1(self, z):
        return self.x + self.y + z
    
    def sum2(self, z, *args):
        return self.x + self.y + z + sum(args)
    
    def sum3(self, z, n=0):
        return self.x + self.y + z + n

    def sum4(self, z, *args, **kwargs):
        return self.x + self.y + z + sum(args) + kwargs['a'] + kwargs['b']
)");

    auto obj = py::eval("Test(1, 2)");

    EXPECT_EQ(obj.attr("sum1")(3).cast<int>(), 6);
    EXPECT_EQ(obj.attr("sum2")(3, 4, 5).cast<int>(), 15);

    using namespace py::literals;

    EXPECT_EQ(obj.attr("sum3")(3, "n"_a = 4).cast<int>(), 10);
    EXPECT_EQ(obj.attr("sum4")(3, 4, 5, "a"_a = 6, "b"_a = 7).cast<int>(), 28);
}

}  // namespace

