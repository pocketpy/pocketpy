#include "pocketpy/common/vector.h"
#include "pocketpy/common/str.h"
#include <stdint.h>

#include "pocketpy/objects/pyvar.h"

#define SMALLMAP_T__SOURCE
#define K uint16_t
#define V pkpy_Var
#define TAG n2v
#include "pocketpy/xmacros/smallmap.h"
#undef SMALLMAP_T__SOURCE
