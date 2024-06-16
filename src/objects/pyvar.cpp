#include "pocketpy/objects/base.h"
#include "pocketpy/interpreter/vm.hpp"

extern "C" {

bool pkpy_Var__eq__(void* vm_, PyVar a, PyVar b) {
    auto vm = (pkpy::VM*)(vm_);
    return vm->py_eq(*(PyVar*)(&a), *(PyVar*)(&b));
}

int64_t pkpy_Var__hash__(void* vm_, PyVar a) {
    auto vm = (pkpy::VM*)(vm_);
    return vm->py_hash(*(PyVar*)(&a));
}

}
