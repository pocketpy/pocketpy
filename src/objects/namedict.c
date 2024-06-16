#include "pocketpy/objects/namedict.h"

#define SMALLMAP_T__SOURCE
#define K uint16_t
#define V PyVar
#define NAME pkpy_NameDict
#include "pocketpy/xmacros/smallmap.h"
#undef SMALLMAP_T__SOURCE
