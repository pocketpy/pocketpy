#pragma once

#include "obj.h"
#include "common.h"
#include "memory.h"
#include "str.h"
#include "iter.h"
#include "cffi.h"

namespace pkpy
{
    void add_module_collections(VM *vm);
} // namespace pkpy