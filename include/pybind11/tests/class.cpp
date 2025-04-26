#include "test.h"

namespace {

struct Point {
    int x;
    int y;

private:
    int z;

public:
    inline static int constructor_calls = 0;
    inline static int copy_constructor_calls = 0;
    inline static int move_constructor_calls = 0;
    inline static int destructor_calls = 0;

    Point() : x(0), y(0), z(0) { constructor_calls++; }

    Point(int x, int y, int z) : x(x), y(y), z(z) { constructor_calls++; }

    Point(const Point& p) : x(p.x), y(p.y), z(p.z) {
        copy_constructor_calls++;
        constructor_calls++;
    }

    Point(Point&& p) noexcept : x(p.x), y(p.y), z(p.z) {
        move_constructor_calls++;
        constructor_calls++;
    }

    Point& operator= (const Point& p) {
        x = p.x;
        y = p.y;
        z = p.z;
        return *this;
    }

    ~Point() { destructor_calls++; }

    int& get_z() { return z; }

    void set_z(int z) { this->z = z; }

    std::string stringfy() const {
        return "(" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")";
    }

    void set_pointer(Point* p) {
        this->x = p->x;
        this->y = p->y;
        this->z = p->z;
    }
};

struct Line {
    Point start;
    Point end;
};

TEST_F(PYBIND11_TEST, class) {
    py::module m = py::module::import("__main__");
    py::class_<Point>(m, "Point")
        .def(py::init<>())
        .def(py::init<int, int, int>())
        .def_readwrite("x", &Point::x)
        .def_readwrite("y", &Point::y)
        .def_property("z", &Point::get_z, &Point::set_z)
        .def("stringfy", &Point::stringfy)
        .def("set_pointer", &Point::set_pointer);

    py::exec(R"(
p = Point()
assert p.stringfy() == '(0, 0, 0)'
p = Point(1, 2, 3)
assert p.x == 1
assert p.y == 2
assert p.z == 3
assert p.stringfy() == '(1, 2, 3)'
p.x = 10
p.y = 20
p.z = 30
assert p.stringfy() == '(10, 20, 30)'
p.set_pointer(Point(4,5,6))
assert p.stringfy() == '(4, 5, 6)'
)");

    py::class_<Line> line(m, "Line");

    line  // not bind constructor
        .def_readwrite("start", &Line::start)
        .def_readwrite("end", &Line::end);

    // bind constructor
    line.def(py::init<>());

    py::exec(R"(
l = Line()
l.start = Point(1, 2, 3)
l.end = Point(4, 5, 6)
p = l.start
assert l.start.stringfy() == '(1, 2, 3)'
assert l.end.stringfy() == '(4, 5, 6)'
)");

    py::finalize(true);

    EXPECT_EQ(Point::constructor_calls, Point::destructor_calls);
    EXPECT_EQ(Point::copy_constructor_calls, 0);
    EXPECT_EQ(Point::move_constructor_calls, 0);
}

TEST_F(PYBIND11_TEST, inheritance) {
    static int constructor_calls = 0;

    struct Point {
        int x;
        int y;

        Point() : x(0), y(0) { constructor_calls++; }

        Point(int x, int y) : x(x), y(y) { constructor_calls++; }
    };

    struct Point3D : Point {
        int z;

        Point3D() : Point(), z(0) { constructor_calls++; }

        Point3D(int x, int y, int z) : Point(x, y), z(z) { constructor_calls++; }
    };

    py::module m = py::module::import("__main__");

    py::class_<Point>(m, "Point")
        .def(py::init<>())
        .def(py::init<int, int>())
        .def_readwrite("x", &Point::x)
        .def_readwrite("y", &Point::y);

    py::class_<Point3D, Point>(m, "Point3D")
        .def(py::init<>())
        .def(py::init<int, int, int>())
        .def_readwrite("z", &Point3D::z);

    py::exec(R"(
p = Point3D()
assert type(p) == Point3D
assert p.x == 0
assert p.y == 0
assert p.z == 0

p = Point3D(1, 2, 3)
assert p.x == 1
assert p.y == 2
assert p.z == 3

p.x = 10
p.y = 20
p.z = 30
assert p.x == 10
assert p.y == 20
assert p.z == 30
)");

    py::finalize(true);

    EXPECT_EQ(constructor_calls, 4);
}

TEST_F(PYBIND11_TEST, dynamic_attr) {
    py::module m = py::module::import("__main__");

    struct Point {
        int x;
        int y;

        Point(int x, int y) : x(x), y(y) {}
    };

    py::class_<Point>(m, "Point", py::dynamic_attr())
        .def(py::init<int, int>())
        .def_readwrite("x", &Point::x)
        .def_readwrite("y", &Point::y);

    py::object p = py::eval("Point(1, 2)");
    EXPECT_EQ(p.attr("x").cast<int>(), 1);
    EXPECT_EQ(p.attr("y").cast<int>(), 2);

    p.attr("z") = py::int_(3);
    EXPECT_EQ(p.attr("z").cast<int>(), 3);
}

TEST_F(PYBIND11_TEST, enum) {
    enum class Color { RED, Yellow, GREEN, BLUE };

    py::module m = py::module::import("__main__");

    py::enum_<Color> color(m, "Color");

    color.value("RED", Color::RED)
        .value("Yellow", Color::Yellow)
        .value("GREEN", Color::GREEN)
        .value("BLUE", Color::BLUE);

    EXPECT_EVAL_EQ("Color.RED", Color::RED);
    EXPECT_EVAL_EQ("Color.Yellow", Color::Yellow);
    EXPECT_EVAL_EQ("Color.GREEN", Color::GREEN);
    EXPECT_EVAL_EQ("Color.BLUE", Color::BLUE);

    EXPECT_EVAL_EQ("Color(0)", Color::RED);
    EXPECT_EVAL_EQ("Color(1)", Color::Yellow);
    EXPECT_EVAL_EQ("Color(2)", Color::GREEN);
    EXPECT_EVAL_EQ("Color(3)", Color::BLUE);

    EXPECT_EXEC_EQ("Color(0)", "Color.RED");
    EXPECT_EXEC_EQ("Color(1)", "Color.Yellow");
    EXPECT_EXEC_EQ("Color(2)", "Color.GREEN");
    EXPECT_EXEC_EQ("Color(3)", "Color.BLUE");

    EXPECT_EVAL_EQ("Color.RED.value", static_cast<int>(Color::RED));
    EXPECT_EVAL_EQ("Color.Yellow.value", static_cast<int>(Color::Yellow));
    EXPECT_EVAL_EQ("Color.GREEN.value", static_cast<int>(Color::GREEN));
    EXPECT_EVAL_EQ("Color.BLUE.value", static_cast<int>(Color::BLUE));

    color.export_values();
    EXPECT_EVAL_EQ("RED", Color::RED);
    EXPECT_EVAL_EQ("Yellow", Color::Yellow);
    EXPECT_EVAL_EQ("GREEN", Color::GREEN);
    EXPECT_EVAL_EQ("BLUE", Color::BLUE);
}

}  // namespace

