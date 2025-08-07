#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define PK_REGION(name) 1

#define PK_SLICE_LOOP(i, start, stop, step)                                                        \
    for(int i = start; step > 0 ? i < stop : i > stop; i += step)

// global constants
#define PK_HEX_TABLE "0123456789abcdef"

#ifdef _MSC_VER
#define c11__unreachable() __assume(0)
#else
#define c11__unreachable() __builtin_unreachable()
#endif

#define c11__abort(...)                                                                            \
    do {                                                                                           \
        fprintf(stderr, __VA_ARGS__);                                                              \
        putchar('\n');                                                                             \
        abort();                                                                                   \
    } while(0)

#define c11__rtassert(cond)                                                                        \
    if(!(cond)) c11__abort("assertion failed: %s", #cond)

#define c11__min(a, b) ((a) < (b) ? (a) : (b))
#define c11__max(a, b) ((a) > (b) ? (a) : (b))

#define c11__count_array(a) (sizeof(a) / sizeof(a[0]))

// ref counting
typedef struct RefCounted {
    int count;
    void (*dtor)(void*);
} RefCounted;

#define PK_INCREF(obj) (obj)->rc.count++
#define PK_DECREF(obj)                                                                             \
    do {                                                                                           \
        if(--(obj)->rc.count == 0) {                                                               \
            (obj)->rc.dtor(obj);                                                                   \
            PK_FREE(obj);                                                                          \
        }                                                                                          \
    } while(0)
