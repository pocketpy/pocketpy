#include "pocketpy/common/vector.h"
#include <stdint.h>

#define SMALLMAP_T__SOURCE
#define K uint16_t
#define V int
#define TAG n2i
#include "pocketpy/xmacros/smallmap.h"
#undef SMALLMAP_T__SOURCE


#define SMALLMAP_T__SOURCE
#define K const char*
#define V uint16_t
#define TAG s2n
#include "pocketpy/xmacros/smallmap.h"
#undef SMALLMAP_T__SOURCE
