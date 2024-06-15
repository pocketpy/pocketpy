#include "pocketpy/common/algorithm.h"

void *c11__lower_bound(const void *key, const void *ptr, int count, int size,
                       bool (*less)(const void *, const void *)) {
    char* __first = (char*)ptr;
    int __len = count;

    while(__len != 0){
        int __l2 = (int)((unsigned int)__len >> 1);
        char* __m = __first + __l2 * size;
        if(less(__m, key)){
            __first = __m;
            __m += size;
            __len -= __l2 + 1;
        }else{
            __len = __l2;
        }
    }
    return __first;
}