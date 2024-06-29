#pragma once

#include "pocketpy/common/vector.h"
#include "pocketpy/common/str.h"
#include "pocketpy/objects/base.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SMALLMAP_T__HEADER
#define K uint16_t
#define V py_TValue
#define NAME pk_NameDict
#include "pocketpy/xmacros/smallmap.h"
#undef SMALLMAP_T__HEADER

#ifdef __cplusplus
}
#endif
