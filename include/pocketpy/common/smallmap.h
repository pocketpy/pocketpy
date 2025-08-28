#pragma once

#include "pocketpy/common/vector.h"
#include "pocketpy/common/str.h"
#include <stdint.h>

#define SMALLMAP_T__HEADER
#define K py_Name
#define V int
#define NAME c11_smallmap_n2d
#include "pocketpy/xmacros/smallmap.h"
#undef SMALLMAP_T__HEADER

#define SMALLMAP_T__HEADER
#define K int
#define V int
#define NAME c11_smallmap_d2d
#include "pocketpy/xmacros/smallmap.h"
#undef SMALLMAP_T__HEADER

#define SMALLMAP_T__HEADER
#define K c11_sv
#define V int
#define NAME c11_smallmap_v2d
#define less(a, b) (c11_sv__cmp((a), (b)) < 0)
#define equal(a, b) c11__sveq((a), (b))
#include "pocketpy/xmacros/smallmap.h"
#undef SMALLMAP_T__HEADER

#define SMALLMAP_T__HEADER
#define K void*
#define V py_i64
#define NAME c11_smallmap_p2i
#include "pocketpy/xmacros/smallmap.h"
#undef SMALLMAP_T__HEADER
