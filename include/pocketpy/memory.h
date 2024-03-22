#pragma once

#include "common.h"

namespace pkpy{

void* pool64_alloc(size_t) noexcept;
void pool64_dealloc(void*) noexcept;

void* pool128_alloc(size_t) noexcept;
void pool128_dealloc(void*) noexcept;

template<typename T>
void* pool64_alloc() noexcept{
    return pool64_alloc(sizeof(T));
}

template<typename T>
void* pool128_alloc() noexcept{
    return pool128_alloc(sizeof(T));
}

void pools_shrink_to_fit() noexcept;

std::string pool64_info() noexcept;
std::string pool128_info() noexcept;

};  // namespace pkpy
