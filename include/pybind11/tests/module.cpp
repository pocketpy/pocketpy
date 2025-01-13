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

struct import_callback {
    using cb_type = decltype(py_callbacks()->importfile);

    import_callback() {
        assert(_importfile == nullptr);
        _importfile = py_callbacks()->importfile;
        py_callbacks()->importfile = importfile;
    };

    ~import_callback() {
        assert(_importfile != nullptr);
        py_callbacks()->importfile = _importfile;
        _importfile = nullptr;
    };

    static char* importfile(const char* path) {
        if(value.empty()) return _importfile(path);
        // +1 for the null terminator
        char* cstr = new char[value.size() + 1];

        std::strcpy(cstr, value.c_str());
        return cstr;
    }

    static std::string value;

private:
    static cb_type _importfile;
};

import_callback::cb_type import_callback::_importfile = nullptr;
std::string import_callback::value = "";

TEST_F(PYBIND11_TEST, reload_module) {
    import_callback cb;

    import_callback::value = "value = 1\n";
    auto mod = py::module::import("reload_module");
    EXPECT_EQ(mod.attr("value").cast<int>(), 1);

    import_callback::value = "value = 2\n";
    mod.reload();
    EXPECT_EQ(mod.attr("value").cast<int>(), 2);

    import_callback::value = "raise ValueError()";
    // Reload in Python raises a ValueError
    py::exec(
        "import importlib\nimport reload_module\ntry:\n    importlib.reload(reload_module)\nexcept ValueError:\n    pass");

    // Reload in C++ raises a ValueError
    try {
        mod.reload();
    } catch(py::error_already_set& e) {
        if(e.match(tp_ValueError)) { return; }
        std::rethrow_exception(std::current_exception());
    }
}

}  // namespace
