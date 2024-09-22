#pragma once

#include "internal/class.h"
#include "internal/function.h"
#include "internal/types.h"

namespace pkbind {

namespace literals {
inline arg operator""_a (const char* c, size_t) { return arg(c); }
}  // namespace literals

inline bool initialized = false;

/// initialize the vm.
inline void initialize(int object_pool_size = 1024) {
    if(!initialized) { py_initialize(); }

    // initialize object pool.
    object_pool::initialize(object_pool_size);

    action::initialize();
    initialized = true;
}

/// finalize the vm.
inline void finalize(bool test = false) {
    if(!initialized) { return; }
    object_pool::finalize();
    if(test) {
        type::m_type_map.clear();
        capsule::tp_capsule.reset();
        cpp_function::tp_function_record.reset();
        py_resetvm();
    } else {
        py_finalize();
    }
}

/// a RAII class to initialize and finalize python interpreter
class scoped_interpreter {
public:
    scoped_interpreter(int object_pool_size = 1024) { initialize(object_pool_size); }

    ~scoped_interpreter() { finalize(); }
};

}  // namespace pkbind

namespace pybind11 = pkbind;