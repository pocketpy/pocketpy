#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// ref counting
typedef struct RefCounted {
    int count;
    void (*dtor)(void*);
} RefCounted;

#define PK_INCREF(obj) (obj)->rc.count++
#define PK_DECREF(obj) do { \
    if(--(obj)->rc.count == 0) { \
        (obj)->rc.dtor(obj); \
        free(obj); \
    } \
} while(0)

#ifdef __cplusplus
}
#endif