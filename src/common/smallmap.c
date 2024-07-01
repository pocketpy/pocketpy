#include "pocketpy/common/smallmap.h"

#define SMALLMAP_T__SOURCE
#define K uint16_t
#define V int
#define NAME c11_smallmap_n2i
#include "pocketpy/xmacros/smallmap.h"
#undef SMALLMAP_T__SOURCE


#define SMALLMAP_T__SOURCE
#define K c11_sv
#define V uint16_t
#define NAME c11_smallmap_s2n
#define less(a, b)      (c11_sv__cmp((a), (b)) <  0)
#define equal(a, b)     (c11_sv__cmp((a), (b)) == 0)
#include "pocketpy/xmacros/smallmap.h"
#undef SMALLMAP_T__SOURCE
