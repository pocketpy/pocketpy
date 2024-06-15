#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

void *c11__lower_bound(const void *key, const void *ptr, int count, int size,
                       bool (*less)(const void *, const void *));

#ifdef __cplusplus
}

namespace pkpy{
template<typename T>
    T* lower_bound(T* begin, T* end, const T& value){
        return (T*)c11__lower_bound(&value, begin, end - begin, sizeof(T), [](const void* a, const void* b){
            return *(T*)a < *(T*)b;
        });
    }
}   // namespace pkpy
#endif