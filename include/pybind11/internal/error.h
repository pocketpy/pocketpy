#pragma once

#include "object.h"

namespace pkbind {

/// represent a all exception raised by python.
class python_error : public std::exception {
public:
    python_error(char* what, object exception) : m_what(what), m_exception(std::move(exception)) {}

    const char* what() const noexcept override { return m_what; }

    // get the python  exception object
    object& exception() { return m_exception; }

    ~python_error() { py_free(m_what); }

    bool match(py_Type type) const { return py_isinstance(m_exception.ptr(), type); }

    bool match(class type) const;

private:
    char* m_what;
    object m_exception;
};

class error_already_set : public std::exception {
public:
    bool match(py_Type type) const { return py_matchexc(type); }

    bool match(type type) const;
};

template <auto Fn, typename... Args>
inline auto raise_call(Args&&... args) {
    auto pc = py_peek(0);
    auto result = Fn(std::forward<Args>(args)...);

    using type = decltype(result);
    if constexpr(std::is_same_v<type, bool>) {
        if(result != false) { return result; }
    } else if constexpr(std::is_same_v<type, int>) {
        if(result != -1) { return result; }
    } else {
        static_assert(dependent_false<type>, "invalid return type");
    }

    bool o = py_matchexc(tp_Exception);
    object e = object::from_ret();
    auto what = py_formatexc();
    py_clearexc(pc);
    throw python_error(what, std::move(e));
}

class stop_iteration {
public:
    stop_iteration() = default;

    stop_iteration(object value) : m_value(std::move(value)) {}

    object value() const { return m_value; }

private:
    object m_value;
};

class cast_error : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class index_error : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class key_error : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class value_error : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class type_error : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class import_error : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class attribute_error : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

inline object::operator bool () const { return raise_call<py_bool>(m_ptr); }

#define PKBIND_BINARY_OPERATOR(name, lop, rop)                                                     \
    inline object operator name (handle lhs, handle rhs) {                                         \
        raise_call<py_binaryop>(lhs.ptr(), rhs.ptr(), lop, rop);                                   \
        return object::from_ret();                                                                 \
    }

PKBIND_BINARY_OPERATOR(==, py_name("__eq__"), py_name("__eq__"))
PKBIND_BINARY_OPERATOR(!=, py_name("__ne__"), py_name("__ne__"))
PKBIND_BINARY_OPERATOR(<, py_name("__lt__"), py_name("__gt__"))
PKBIND_BINARY_OPERATOR(<=, py_name("__le__"), py_name("__ge__"))
PKBIND_BINARY_OPERATOR(>, py_name("__gt__"), py_name("__lt__"))
PKBIND_BINARY_OPERATOR(>=, py_name("__ge__"), py_name("__le__"))

PKBIND_BINARY_OPERATOR(+, py_name("__add__"), py_name("__radd__"))
PKBIND_BINARY_OPERATOR(-, py_name("__sub__"), py_name("__rsub__"))
PKBIND_BINARY_OPERATOR(*, py_name("__mul__"), py_name("__rmul__"))
PKBIND_BINARY_OPERATOR(/, py_name("__truediv__"), py_name("__rtruediv__"))
PKBIND_BINARY_OPERATOR(%, py_name("__mod__"), py_name("__rmod__"))

// FIXME: support __rand__ ...
PKBIND_BINARY_OPERATOR(&, py_name("__and__"), nullptr)
PKBIND_BINARY_OPERATOR(|, py_name("__or__"), nullptr)
PKBIND_BINARY_OPERATOR(^, py_name("__xor__"), nullptr)
PKBIND_BINARY_OPERATOR(<<, py_name("__lshift__"), nullptr)
PKBIND_BINARY_OPERATOR(>>, py_name("__rshift__"), nullptr)

#undef PKBIND_BINARY_OPERATOR

}  // namespace pkbind
