#include "test.h"

TEST_F(PYBIND11_TEST, int) {
    py::object obj = py::int_(123);
    py::object obj2 = py::eval("123");

    EXPECT_EQ(obj, obj2);

    EXPECT_EQ(obj.cast<int>(), 123);
    EXPECT_EQ(obj.cast<long>(), 123);
    EXPECT_EQ(obj.cast<long long>(), 123);
}

TEST_F(PYBIND11_TEST, float) {
    py::object obj = py::float_(123.0);
    py::object obj2 = py::eval("123.0");

    EXPECT_EQ(obj, obj2);

    EXPECT_EQ(obj.cast<float>(), 123.0);
    EXPECT_EQ(obj.cast<double>(), 123.0);
}

TEST_F(PYBIND11_TEST, str) {
    py::object obj = py::str("123");
    py::object obj2 = py::eval("'123'");

    EXPECT_EQ(obj, obj2);

    EXPECT_STREQ(obj.cast<const char*>(), "123");
    EXPECT_EQ(obj.cast<std::string>(), "123");
    EXPECT_EQ(obj.cast<std::string_view>(), "123");

    auto s = py::str("Hello, {}");
    EXPECT_EQ(s.format("world").cast<std::string>(), "Hello, world");
}

TEST_F(PYBIND11_TEST, tuple) {
    py::tuple tuple = py::tuple{
        py::int_(1),
        py::str("123"),
        py::int_(3),
    };
    EXPECT_EQ(tuple, py::eval("(1, '123', 3)"));
    EXPECT_EQ(tuple.size(), 3);
    EXPECT_FALSE(tuple.empty());

    tuple[0] = py::int_(3);
    tuple[2] = py::int_(1);
    EXPECT_EQ(tuple, py::eval("(3, '123', 1)"));

    // iterators.
    int index = 0;
    for(auto item: tuple) {
        if(index == 0) {
            EXPECT_EQ(item, py::int_(3));
        } else if(index == 1) {
            EXPECT_EQ(item, py::str("123"));
        } else if(index == 2) {
            EXPECT_EQ(item, py::int_(1));
        }
        index++;
    }
}

TEST_F(PYBIND11_TEST, list) {
    // constructors
    py::list list = py::list();
    EXPECT_EQ(list, py::eval("[]"));
    EXPECT_EQ(list.size(), 0);
    EXPECT_TRUE(list.empty());

    list = py::list{
        py::int_(1),
        py::int_(2),
        py::int_(3),
    };
    EXPECT_EQ(list, py::eval("[1, 2, 3]"));
    EXPECT_EQ(list.size(), 3);
    EXPECT_FALSE(list.empty());

    // accessor
    list[0] = py::int_(3);
    list[2] = py::int_(1);
    EXPECT_EQ(list, py::eval("[3, 2, 1]"));

    // iterators
    int index = 0;
    for(auto item: list) {
        if(index == 0) {
            EXPECT_EQ(item, py::int_(3));
        } else if(index == 1) {
            EXPECT_EQ(item, py::int_(2));
        } else if(index == 2) {
            EXPECT_EQ(item, py::int_(1));
        }
        index++;
    }

    // others
    list.append(py::int_(4));
    EXPECT_EQ(list, py::eval("[3, 2, 1, 4]"));

    list.insert(0, py::int_(7));
    EXPECT_EQ(list, py::eval("[7, 3, 2, 1, 4]"));
}

TEST_F(PYBIND11_TEST, dict) {
    // constructors
    py::dict dict = py::dict();
    EXPECT_EQ(dict, py::eval("{}"));
    EXPECT_EQ(dict.size(), 0);
    EXPECT_TRUE(dict.empty());

    // accessor
    dict["a"] = py::int_(1);
    dict["b"] = py::int_(2);
    dict["c"] = py::int_(3);
    EXPECT_EQ(dict, py::eval("{'a': 1, 'b': 2, 'c': 3}"));
    EXPECT_EQ(dict,
              py::dict({
                  {"a", py::int_(1)},
                  {"b", py::int_(2)},
                  {"c", py::int_(3)},
    }));

    // iterators
    int index = 0;
    for(auto item: dict) {
        if(index == 0) {
            EXPECT_EQ(item.first.cast<std::string>(), "a");
            EXPECT_EQ(item.second, py::int_(1));
        } else if(index == 1) {
            EXPECT_EQ(item.first.cast<std::string>(), "b");
            EXPECT_EQ(item.second, py::int_(2));
        } else if(index == 2) {
            EXPECT_EQ(item.first.cast<std::string>(), "c");
            EXPECT_EQ(item.second, py::int_(3));
        }
        index++;
    }
}

TEST_F(PYBIND11_TEST, capsule) {
    static int times = 0;

    struct NotTrivial {
        ~NotTrivial() { times++; }
    };

    py::handle x = py::capsule(new NotTrivial(), [](void* ptr) {
        delete static_cast<NotTrivial*>(ptr);
    });

    auto m = py::module::__main__();

    m.def("foo", [](int x) {
        return py::capsule(new int(x));
    });

    m.def("bar", [](py::capsule x, int y) {
        EXPECT_EQ(x.cast<int>(), y);
        delete (int*)x.data();
    });

    py::exec("bar(foo(123), 123)");
    py::finalize(true);
    EXPECT_EQ(times, 1);
}
