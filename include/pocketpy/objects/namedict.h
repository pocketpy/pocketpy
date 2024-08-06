#pragma once

#include "pocketpy/common/vector.h"
#include "pocketpy/common/str.h"
#include "pocketpy/objects/base.h"
#include <stdint.h>

#define SMALLMAP_T__HEADER
#define K uint16_t
#define V py_TValue
#define NAME NameDict
#include "pocketpy/xmacros/smallmap.h"
#undef SMALLMAP_T__HEADER

