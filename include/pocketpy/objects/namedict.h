#pragma once

#include "pocketpy/common/vector.h"
#include "pocketpy/common/str.h"
#include <stdint.h>

#include "pocketpy/objects/pyvar.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SMALLMAP_T__HEADER
#define K uint16_t
#define V pkpy_Var
#define TAG n2v
#include "pocketpy/xmacros/smallmap.h"
#undef SMALLMAP_T__HEADER

typedef c11_smallmap_n2v pkpy_NameDict;

#ifdef __cplusplus
}
#endif
