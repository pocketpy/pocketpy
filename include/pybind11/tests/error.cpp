#include "test.h"

TEST_F(PYBIND11_TEST, exception_python_to_cpp) {
    auto test = [](const char* code, py_Type type) {
        try {
            py::exec(code);
        } catch(py::python_error& e) {
            EXPECT_TRUE(e.match(type));
        }
    };

    test("raise ValueError()", tp_ValueError);
    test("raise KeyError()", tp_KeyError);
    test("raise IndexError()", tp_IndexError);
    test("raise StopIteration()", tp_StopIteration);
    test("raise Exception()", tp_Exception);
}

TEST_F(PYBIND11_TEST, exception_cpp_to_python) {
    auto m = py::module::__main__();

#define TEST_EXCEPTION(cppe, pye)                                                                                      \
    m.def("test_" #cppe, []() {                                                                                        \
        throw cppe(#cppe);                                                                                             \
    });                                                                                                                \
    py::exec("try:\n    test_" #cppe "()\nexcept Exception as e:\n    assert isinstance(e, " #pye                      \
             ")\n    assert str(e) == '" #cppe "'")

    using namespace std;
    TEST_EXCEPTION(runtime_error, RuntimeError);
    TEST_EXCEPTION(domain_error, ValueError);
    TEST_EXCEPTION(invalid_argument, ValueError);
    TEST_EXCEPTION(length_error, ValueError);
    TEST_EXCEPTION(out_of_range, IndexError);
    TEST_EXCEPTION(range_error, ValueError);

    using namespace py;

    m.def("test_stop_iteration", []() {
        throw py::stop_iteration();
    });
    py::exec("try:\n    test_stop_iteration()\nexcept StopIteration as e:\n    pass");

    m.def("test_stop_iteration_value", []() {
        throw py::stop_iteration(py::int_(42));
    });
    py::exec(
        "try:\n    test_stop_iteration_value()\nexcept StopIteration as e:\n    assert e.value == 42");

    m.def("test_error_already_set", []() {
        KeyError(none().ptr());
        throw py::error_already_set();
    });
    py::exec("try:\n    test_error_already_set()\nexcept KeyError as e:\n    pass");

    m.def("test_error_already_set_matches", []() {
        try {
            KeyError(none().ptr());
            throw py::error_already_set();
        } catch(py::error_already_set& e) {
            if(e.match(tp_KeyError)) { return; }
            std::rethrow_exception(std::current_exception());
        }

        try {
            StopIteration();
            throw py::error_already_set();
        } catch(py::error_already_set& e) {
            if(e.match(type(tp_StopIteration))) { return; }
            std::rethrow_exception(std::current_exception());
        }
    });
    py::exec("test_error_already_set_matches()");
    TEST_EXCEPTION(index_error, IndexError);
    // FIXME: TEST_EXCEPTION(key_error, KeyError);
    TEST_EXCEPTION(value_error, ValueError);
    TEST_EXCEPTION(type_error, TypeError);
    TEST_EXCEPTION(import_error, ImportError);
    TEST_EXCEPTION(attribute_error, AttributeError);
    TEST_EXCEPTION(runtime_error, RuntimeError);
}
