#pragma once

#include <cstddef>
#include <string>

namespace pkpy{

void* pool128_alloc(size_t) noexcept;
void pool128_dealloc(void*) noexcept;

template<typename T>
void* pool128_alloc() noexcept{
    return pool128_alloc(sizeof(T));
}

void pools_shrink_to_fit() noexcept;

std::string pool64_info() noexcept;
std::string pool128_info() noexcept;

};  // namespace pkpy
