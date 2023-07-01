#pragma once

#include "common.h"

#if PK_MODULE_EASING

#include "cffi.h"

namespace pkpy{

void add_module_easing(VM* vm);

} // namespace pkpy

#else

ADD_MODULE_PLACEHOLDER(easing)

#endif