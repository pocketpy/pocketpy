#pragma once

#include "common.h"

namespace pkpy{

void* pool64_alloc(size_t);
void pool64_dealloc(void*);

void* pool128_alloc(size_t);
void pool128_dealloc(void*);

template<typename T>
void* pool64_alloc(){
    return pool64_alloc(sizeof(T));
}

template<typename T>
void* pool128_alloc(){
    return pool128_alloc(sizeof(T));
}

void pools_shrink_to_fit();

};  // namespace pkpy
