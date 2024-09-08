#include "test.h"

namespace {

const char* source = R"(
class Point:
    def __init__(self, x, y):
        self.x = x
        self.y = y

    def __add__(self, other):
        return Point(self.x + other.x, self.y + other.y)

    def __sub__(self, other):
        return Point(self.x - other.x, self.y - other.y)

    def __mul__(self, other):
        return Point(self.x * other.x, self.y * other.y)

    def __truediv__(self, other):
        return Point(self.x / other.x, self.y / other.y)

    def __floordiv__(self, other):
        return Point(self.x // other.x, self.y // other.y)

    def __mod__(self, other):
        return Point(self.x % other.x, self.y % other.y)

    def __pow__(self, other):
        return Point(self.x ** other.x, self.y ** other.y)

    def __lshift__(self, other):
        return Point(self.x << other.x, self.y << other.y)

    def __rshift__(self, other):
        return Point(self.x >> other.x, self.y >> other.y)

    def __eq__(self, other):
        return self.x == other.x and self.y == other.y

    def __ne__(self, other) -> bool:
        return not self.__eq__(other)

    def __lt__(self, other) -> bool:
        return self.x < other.x and self.y < other.y

    def __le__(self, other) -> bool:
        return self.x <= other.x and self.y <= other.y

    def __gt__(self, other) -> bool:
        return self.x > other.x and self.y > other.y

    def __ge__(self, other) -> bool:
        return self.x >= other.x and self.y >= other.y

    def __repr__(self):
        return f'Point({self.x}, {self.y})'
)";

TEST_F(PYBIND11_TEST, object) {
    py::module m = py::module::import("__main__");
    py::exec(source);
    py::exec("p = Point(3, 4)");
    py::object p = py::eval("p");

    //  is
    EXPECT_FALSE(p.is_none());
    EXPECT_TRUE(p.is(p));

    //  attrs
    EXPECT_EQ(p.attr("x").cast<int>(), 3);
    EXPECT_EQ(p.attr("y").cast<int>(), 4);

    p.attr("x") = py::int_(5);
    p.attr("y") = py::int_(6);

    EXPECT_EQ(p.attr("x").cast<int>(), 5);
    EXPECT_EQ(p.attr("y").cast<int>(), 6);
    EXPECT_EXEC_EQ("p", "Point(5, 6)");

    //  operators
    EXPECT_EVAL_EQ("Point(10, 12)", p + p);
    EXPECT_EVAL_EQ("Point(0, 0)", p - p);
    EXPECT_EVAL_EQ("Point(25, 36)", p * p);
    EXPECT_EVAL_EQ("Point(1, 1)", p / p);
    // EXPECT_EVAL_EQ("Point(0, 0)", p // p);
    EXPECT_EVAL_EQ("Point(0, 0)", p % p);

    // iterators
    py::object l = py::eval("[1, 2, 3]");
    int index = 0;
    for(auto item: l) {
        EXPECT_EQ(item.cast<int>(), index + 1);
        index++;
    }
}

}  // namespace
