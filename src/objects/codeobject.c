#include "pocketpy/objects/codeobject.h"
#include "pocketpy/common/utils.h"

void Bytecode__set_signed_arg(Bytecode* self, int arg) {
    if(arg < INT16_MIN || arg > INT16_MAX) {
        PK_FATAL_ERROR("set_signed_arg: %d is out of range", arg);
    }
    self->arg = (int16_t)arg;
}

bool Bytecode__is_forward_jump(const Bytecode* self) {
    return self->op >= OP_JUMP_FORWARD && self->op <= OP_LOOP_BREAK;
}
