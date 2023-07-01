#pragma once

#include "common.h"

#if PK_MODULE_BASE64

#include "cffi.h"

namespace pkpy {

void add_module_base64(VM* vm);

} // namespace pkpy


#else

ADD_MODULE_PLACEHOLDER(base64)

#endif