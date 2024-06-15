#include "pocketpy/common/algorithm.h"

void *c11__lower_bound(const void *key, const void *ptr, int count, int size,
                       bool (*less)(const void *, const void *)) {
    char* __first = (char*)ptr;
    int __len = count;

    while(__len != 0){
        int __l2 = (int)((unsigned int)__len / 2);
        char* __m = __first + __l2 * size;
        if(less(__m, key)){
            __m += size;
            __first = __m;
            __len -= __l2 + 1;
        }else{
            __len = __l2;
        }
    }
    return __first;
}

static bool c11__less_int(const void* a, const void* b){
    return *(int*)a < *(int*)b;
}

static bool c11__less_double(const void* a, const void* b){
    return *(double*)a < *(double*)b;
}

int *c11__lower_bound_int(int key, const int *ptr, int count) {
    void* res = c11__lower_bound(&key, ptr, count, sizeof(int), c11__less_int);
    return (int*)res;
}

double *c11__lower_bound_double(double key, const double *ptr, int count) {
    void* res = c11__lower_bound(&key, ptr, count, sizeof(double), c11__less_double);
    return (double*)res;
}

