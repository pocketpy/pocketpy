#pragma once

#include "internal/class.h"

namespace pkbind {

namespace literals {
inline arg operator""_a (const char* c, size_t) { return arg(c); }
}  // namespace literals

inline bool initialized = false;

/// initialize the vm.
inline void initialize(int object_pool_size = 1024) {
    if(!initialized) { py_initialize(); }

    // initialize all registers.
    reg<0>.value = py_getreg(0);
    reg<1>.value = py_getreg(1);
    reg<2>.value = py_getreg(2);
    reg<3>.value = py_getreg(3);
    reg<4>.value = py_getreg(4);
    reg<5>.value = py_getreg(5);
    reg<6>.value = py_getreg(6);

    // initialize ret.
    retv.value = py_retval();

    // initialize object pool.
    object_pool::initialize(object_pool_size);

    m_type_map = new std::unordered_map<std::type_index, py_Type>();

    // register types.
    capsule::register_();
    cpp_function::register_();

    action::initialize();
    initialized = true;
}

/// finalize the vm.
inline void finalize(bool test = false) {
    if(!initialized) { return; }
    delete m_type_map;
    m_type_map = nullptr;
    object_pool::finalize();
    if(test) {
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