#include "pocketpy/objects/base.h"

PyVar pkpy_NULL = {.type=0, .is_ptr=false};
PyVar pkpy_OP_CALL = {.type=tp_op_call, .is_ptr=false};
PyVar pkpy_OP_YIELD = {.type=tp_op_yield, .is_ptr=false};
