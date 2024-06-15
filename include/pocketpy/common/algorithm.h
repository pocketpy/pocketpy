#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define c11__less(a, b) ((a) < (b))

#define c11__lower_bound(T, ptr, count, key, less, out_index)                                      \
    do {                                                                                           \
        T* __first = ptr;                                                                          \
        int __len = count;                                                                         \
        while(__len != 0) {                                                                        \
            int __l2 = __len >> 1;                                                                 \
            T* __m = __first + __l2;                                                               \
            if(less((*__m), (key))) {                                                              \
                __first = ++__m;                                                                   \
                __len -= __l2 + 1;                                                                 \
            } else {                                                                               \
                __len = __l2;                                                                      \
            }                                                                                      \
        }                                                                                          \
        *(out_index) = __first - (T*)(ptr);                                                        \
    } while(0)

#ifdef __cplusplus
}

#endif