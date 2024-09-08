#include "test.h"
#include "pybind11/stl.h"

namespace {

int constructor_calls = 0;
int destructor_calls = 0;

struct Point {
    int x;
    int y;

    Point() : x(0), y(0) { constructor_calls++; }

    Point(int x, int y) : x(x), y(y) { constructor_calls++; }

    Point(const Point& p) : x(p.x), y(p.y) { constructor_calls++; }

    Point(Point&& p) noexcept : x(p.x), y(p.y) { constructor_calls++; }

    Point& operator= (const Point& p) {
        x = p.x;
        y = p.y;
        return *this;
    }

    ~Point() { destructor_calls++; }

    bool operator== (const Point& p) const { return x == p.x && y == p.y; }
};

}  // namespace

TEST_F(PYBIND11_TEST, vector_bool) {
    std::vector<bool> v = {true, false, true};
    py::object obj = py::cast(v);
    EXPECT_EVAL_EQ("[True, False, True]", obj);

    std::vector<bool> v2 = obj.cast<std::vector<bool>>();
    EXPECT_EQ(v, v2);
}

TEST_F(PYBIND11_TEST, list_like) {
    py::class_<Point>(py::module::__main__(), "Point")
        .def(py::init<int, int>())
        .def_readwrite("x", &Point::x)
        .def_readwrite("y", &Point::y)
        .def("__eq__", &Point::operator==);

    // array
    {
        std::array<Point, 2> a = {Point(1, 2), Point(3, 4)};
        py::object obj = py::eval("[Point(1, 2), Point(3, 4)]");
        EXPECT_EVAL_EQ("[Point(1, 2), Point(3, 4)]", obj);

        std::array<Point, 2> a2 = obj.cast<std::array<Point, 2>>();
        EXPECT_EQ(a, a2);
    }

    // vector
    {
        std::vector<Point> v = {Point(1, 2), Point(3, 4)};
        py::object obj = py::cast(v);
        EXPECT_EVAL_EQ("[Point(1, 2), Point(3, 4)]", obj);

        std::vector<Point> v2 = obj.cast<std::vector<Point>>();
        EXPECT_EQ(v, v2);
    }

    // list
    {
        std::list<Point> l = {Point(1, 2), Point(3, 4)};
        py::object obj = py::cast(l);
        EXPECT_EVAL_EQ("[Point(1, 2), Point(3, 4)]", obj);

        std::list<Point> l2 = obj.cast<std::list<Point>>();
        EXPECT_EQ(l, l2);
    }

    // deque
    {
        std::deque<Point> d = {Point(1, 2), Point(3, 4)};
        py::object obj = py::cast(d);
        EXPECT_EVAL_EQ("[Point(1, 2), Point(3, 4)]", obj);

        std::deque<Point> d2 = obj.cast<std::deque<Point>>();
        EXPECT_EQ(d, d2);
    }
}

TEST_F(PYBIND11_TEST, dict_like) {
    py::class_<Point>(py::module::__main__(), "Point")
        .def(py::init<int, int>())
        .def_readwrite("x", &Point::x)
        .def_readwrite("y", &Point::y)
        .def("__eq__", &Point::operator==);

    // map
    {
        std::map<std::string, Point> m = {
            {"a", Point(1, 2)},
            {"b", Point(3, 4)}
        };

        py::object obj = py::cast(m);
        EXPECT_EVAL_EQ("{'a': Point(1, 2), 'b': Point(3, 4)}", obj);

        std::map<std::string, Point> m2 = obj.cast<std::map<std::string, Point>>();
        EXPECT_EQ(m, m2);
    }

    // unordered_map
    {
        std::unordered_map<std::string, Point> m = {
            {"a", Point(1, 2)},
            {"b", Point(3, 4)}
        };

        py::object obj = py::cast(m);
        EXPECT_EVAL_EQ("{'a': Point(1, 2), 'b': Point(3, 4)}", obj);

        std::unordered_map<std::string, Point> m2 =
            obj.cast<std::unordered_map<std::string, Point>>();
        EXPECT_EQ(m, m2);
    }
}
