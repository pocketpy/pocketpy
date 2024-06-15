#include "pocketpy/objects/base.hpp"
#include "pocketpy/objects/pyvar.h"
#include "pocketpy/interpreter/vm.hpp"

extern "C" {

bool pkpy_Var__eq__(void* vm_, pkpy_Var a, pkpy_Var b) {
    auto vm = (pkpy::VM*)(vm_);
    return vm->py_eq(*(pkpy::PyVar*)(&a), *(pkpy::PyVar*)(&b));
}

int64_t pkpy_Var__hash__(void* vm_, pkpy_Var a) {
    auto vm = (pkpy::VM*)(vm_);
    return vm->py_hash(*(pkpy::PyVar*)(&a));
}

}
