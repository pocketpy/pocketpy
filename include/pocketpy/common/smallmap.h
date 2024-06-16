#pragma once

#include "pocketpy/common/vector.h"
#include "pocketpy/common/str.h"
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
#define K c11_string
#define V uint16_t
#define TAG s2n
#define less(a, b)      (c11_string__cmp((a), (b)) <  0)
#define equal(a, b)     (c11_string__cmp((a), (b)) == 0)
#include "pocketpy/xmacros/smallmap.h"
#undef SMALLMAP_T__HEADER

#ifdef __cplusplus
}
#endif