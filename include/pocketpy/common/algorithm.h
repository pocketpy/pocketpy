#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define c11__less(a, b) ((a) < (b))

#define c11__lower_bound(T, ptr, count, key, less, out)                        \
  do {                                                                         \
    const T *__first = ptr;                                                    \
    int __len = count;                                                         \
    while (__len != 0) {                                                       \
      int __l2 = (int)((unsigned int)__len / 2);                               \
      const T *__m = __first + __l2;                                           \
      if (less((*__m), (key))) {                                               \
        __first = ++__m;                                                       \
        __len -= __l2 + 1;                                                     \
      } else {                                                                 \
        __len = __l2;                                                          \
      }                                                                        \
    }                                                                          \
    *(out) = __first;                                                          \
  } while (0)

#ifdef __cplusplus
}

#endif