#include "pocketpy/objects/namedict.h"

#define SMALLMAP_T__SOURCE
#define K uint16_t
#define V py_TValue
#define NAME pk_NameDict
#include "pocketpy/xmacros/smallmap.h"
#undef SMALLMAP_T__SOURCE
