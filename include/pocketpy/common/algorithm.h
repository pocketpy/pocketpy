#pragma once

#include <stdbool.h>

#define c11__less(a, b) ((a) < (b))

#define c11__lower_bound(T, ptr, count, key, less, out_index)                                      \
    do {                                                                                           \
        T* __first = ptr;                                                                          \
        int __len = count;                                                                         \
        while(__len >= 8) {                                                                        \
            int __l2 = __len >> 1;                                                                 \
            T* __m = __first + __l2;                                                               \
            if(less((*__m), (key))) {                                                              \
                __first = ++__m;                                                                   \
                __len -= __l2 + 1;                                                                 \
            } else {                                                                               \
                __len = __l2;                                                                      \
            }                                                                                      \
        }                                                                                          \
        switch(__len) {                                                                            \
            case 7:                                                                                \
                if(less(*__first, (key))) __first++;                                               \
            case 6:                                                                                \
                if(less(*__first, (key))) __first++;                                               \
            case 5:                                                                                \
                if(less(*__first, (key))) __first++;                                               \
            case 4:                                                                                \
                if(less(*__first, (key))) __first++;                                               \
            case 3:                                                                                \
                if(less(*__first, (key))) __first++;                                               \
            case 2:                                                                                \
                if(less(*__first, (key))) __first++;                                               \
            case 1:                                                                                \
                if(less(*__first, (key))) __first++;                                               \
            case 0: break;                                                                         \
            default: c11__unreachable();                                                           \
        }                                                                                          \
        *(out_index) = __first - (T*)(ptr);                                                        \
    } while(0)

/**
 * @brief Sorts an array of elements of the same type, using the given comparison function.
 * @param ptr Pointer to the first element of the array.
 * @param count Number of elements in the array.
 * @param elem_size Size of each element in the array.
 * @param cmp Comparison function that takes two elements and returns an integer similar to
 * `strcmp`.
 */
bool c11__stable_sort(void* ptr,
                      int length,
                      int elem_size,
                      int (*f_lt)(const void* a, const void* b, void* extra),
                      void* extra);
