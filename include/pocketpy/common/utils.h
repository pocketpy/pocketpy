#pragma once

#include "stdio.h"
#include "stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
#define PK_INLINE inline
#else
#define PK_INLINE static inline
#endif

#define PK_REGION(name) 1

#define PK_SLICE_LOOP(i, start, stop, step) for(int i = start; step > 0 ? i < stop : i > stop; i += step)

// global constants
#define PK_HEX_TABLE "0123456789abcdef"

extern const char* kPlatformStrings[];

#ifdef _MSC_VER
#define PK_UNREACHABLE() __assume(0);
#else
#define PK_UNREACHABLE() __builtin_unreachable();
#endif

#define PK_FATAL_ERROR(...) { fprintf(stderr, __VA_ARGS__); abort(); }

#define PK_MIN(a, b) ((a) < (b) ? (a) : (b))
#define PK_MAX(a, b) ((a) > (b) ? (a) : (b))

#define PK_ARRAY_COUNT(a)   (sizeof(a) / sizeof(a[0]))

// NARGS
#define PK_NARGS_SEQ(_1, _2, _3, _4, N, ...) N
#define PK_NARGS(...) PK_NARGS_SEQ(__VA_ARGS__, 4, 3, 2, 1, 0)
#define PK_NPTRS(...) PK_NARGS_SEQ(__VA_ARGS__, int****, int***, int**, int*, int)

#ifdef __cplusplus
}
#endif