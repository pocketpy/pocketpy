#include "pocketpy/objects/base.h"
#include "pocketpy/interpreter/vm.hpp"

extern "C" {

bool py_eq(const PyVar* a, const PyVar* b){
    auto vm = (pkpy::VM*)pkpy_g.vm;
    return vm->py_eq(*a, *b);
}

bool py_le(const PyVar* a, const PyVar* b){
    auto vm = (pkpy::VM*)pkpy_g.vm;
    return vm->py_le(*a, *b);
}

int64_t py_hash(const PyVar* a){
    auto vm = (pkpy::VM*)pkpy_g.vm;
    return vm->py_hash(*a);
}

}
