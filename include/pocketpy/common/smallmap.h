#pragma once

#include "pocketpy/common/vector.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SMALLMAP_T__HEADER
#define K uint16_t
#define V int
#define TAG n2i
#include "pocketpy/xmacros/smallmap.h"
#undef SMALLMAP_T__HEADER


#define SMALLMAP_T__HEADER
#define K const char*
#define V uint16_t
#define TAG s2n
#include "pocketpy/xmacros/smallmap.h"
#undef SMALLMAP_T__HEADER

#ifdef __cplusplus
}
#endif