#include "pocketpy/objects/base.hpp"
#include "pocketpy/objects/pyvar.h"
#include "pocketpy/interpreter/vm.hpp"

extern "C" {

bool pkpy_Var__eq__(void *vm_, pkpy_Var a, pkpy_Var b) {
    auto vm = static_cast<pkpy::VM *>(vm_);
    return vm->py_eq(*reinterpret_cast<pkpy::PyVar*>(&a), *reinterpret_cast<pkpy::PyVar*>(&b));
}

int64_t pkpy_Var__hash__(void *vm_, pkpy_Var a) {
    auto vm = static_cast<pkpy::VM *>(vm_);
    return vm->py_hash(*reinterpret_cast<pkpy::PyVar*>(&a));
}

}
