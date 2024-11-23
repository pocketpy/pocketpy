#include "pocketpy/common/algorithm.h"
#include <string.h>
#include <stdlib.h>

static bool merge(char* a,
                  char* a_end,
                  char* b,
                  char* b_end,
                  char* r,
                  int elem_size,
                  int (*f_lt)(const void* a, const void* b, void* extra),
                  void* extra) {
    while(a < a_end && b < b_end) {
        int res = f_lt(a, b, extra);
        // check error
        if(res == -1) return false;
        if(res) {
            memcpy(r, a, elem_size);
            a += elem_size;
        } else {
            memcpy(r, b, elem_size);
            b += elem_size;
        }
        r += elem_size;
    }

    // one of the arrays is empty
    for(; a < a_end; a += elem_size, r += elem_size)
        memcpy(r, a, elem_size);
    for(; b < b_end; b += elem_size, r += elem_size)
        memcpy(r, b, elem_size);
    return true;
}

bool c11__stable_sort(void* ptr_,
                      int length,
                      int elem_size,
                      int (*f_lt)(const void* a, const void* b, void* extra),
                      void* extra) {
    // merge sort
    char *ptr = ptr_, *tmp = malloc(length * elem_size);
    for(int seg = 1; seg < length; seg *= 2) {
        for(char* a = ptr; a < ptr + (length - seg) * elem_size; a += 2 * seg * elem_size) {
            char *b = a + seg * elem_size, *a_end = b, *b_end = b + seg * elem_size;
            if(b_end > ptr + length * elem_size) b_end = ptr + length * elem_size;
            bool ok = merge(a, a_end, b, b_end, tmp, elem_size, f_lt, extra);
            if(!ok) {
                free(tmp);
                return false;
            }
            memcpy(a, tmp, b_end - a);
        }
    }
    free(tmp);
    return true;
}

/* Hash Functions */

// https://github.com/python/cpython/blob/v3.10.15/Objects/tupleobject.c#L406
#define SIZEOF_PY_UHASH_T 8
typedef uint64_t Py_uhash_t;

#if SIZEOF_PY_UHASH_T > 4
#define _PyHASH_XXPRIME_1 ((Py_uhash_t)11400714785074694791ULL)
#define _PyHASH_XXPRIME_2 ((Py_uhash_t)14029467366897019727ULL)
#define _PyHASH_XXPRIME_5 ((Py_uhash_t)2870177450012600261ULL)
#define _PyHASH_XXROTATE(x) ((x << 31) | (x >> 33))  /* Rotate left 31 bits */
#else
#define _PyHASH_XXPRIME_1 ((Py_uhash_t)2654435761UL)
#define _PyHASH_XXPRIME_2 ((Py_uhash_t)2246822519UL)
#define _PyHASH_XXPRIME_5 ((Py_uhash_t)374761393UL)
#define _PyHASH_XXROTATE(x) ((x << 13) | (x >> 19))  /* Rotate left 13 bits */
#endif

uint64_t cpy310_tuplehash(uint64_t* p, int len){
    Py_uhash_t acc = _PyHASH_XXPRIME_5;
    for (int i = 0; i < len; i++) {
        Py_uhash_t lane = p[i];
        if (lane == (Py_uhash_t)-1) {
            return -1;
        }
        acc += lane * _PyHASH_XXPRIME_2;
        acc = _PyHASH_XXROTATE(acc);
        acc *= _PyHASH_XXPRIME_1;
    }

    /* Add input length, mangled to keep the historical value of hash(()). */
    acc += len ^ (_PyHASH_XXPRIME_5 ^ 3527539UL);

    if (acc == (Py_uhash_t)-1) {
        return 1546275796;
    }
    return acc;
}
