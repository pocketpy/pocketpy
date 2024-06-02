#pragma once

#include <cstdint>

namespace pkpy{

using i64 = int64_t;		// always 64-bit
using f64 = double;			// always 64-bit

static_assert(sizeof(i64) == 8);
static_assert(sizeof(f64) == 8);

// Explicitly allow copying if copy constructor is deleted
struct explicit_copy_t {
    explicit explicit_copy_t() = default;
};

// Dummy types
struct DummyInstance { };
struct DummyModule { };
struct NoReturn { };

// Forward declarations
struct PyObject;
struct Frame;
class VM;

};  // namespace pkpy