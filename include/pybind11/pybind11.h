#pragma once
#include "internal/class.h"

namespace pybind11 {

namespace literals {
inline arg operator""_a (const char* c, size_t) { return arg(c); }
}  // namespace literals

struct scoped_interpreter {
    scoped_interpreter() { interpreter::initialize(); }

    ~scoped_interpreter() { interpreter::finalize(); }
};

}  // namespace pybind11

// namespace pybind11

#undef PYBIND11_TYPE_IMPLEMENT
#undef PYBIND11_REGISTER_INIT
