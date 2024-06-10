#pragma once

#ifdef __cplusplus
extern "C" {
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

#ifdef __cplusplus
}
#endif