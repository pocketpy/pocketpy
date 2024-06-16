#include "pocketpy/objects/base.h"

struct pkpy_G pkpy_g;

PyVar pkpy_NULL = {.type=0, .is_ptr=false, .extra=0, ._i64=0};
PyVar pkpy_OP_CALL = {.type=27, .is_ptr=false, .extra=0, ._i64=0};
PyVar pkpy_OP_YIELD = {.type=28, .is_ptr=false, .extra=0, ._i64=0};

