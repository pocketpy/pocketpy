#pragma once

#include "pocketpy/common/gil.hpp"

#include <cstddef>
#include <cassert>
#include <string>

namespace pkpy{

void* pool128_alloc(size_t) noexcept;
void pool128_dealloc(void*) noexcept;
void pools_shrink_to_fit() noexcept;

inline const int kPoolExprBlockSize = 128;
inline const int kPoolFrameBlockSize = 80;

void* PoolExpr_alloc() noexcept;
void PoolExpr_dealloc(void*) noexcept;
void* PoolFrame_alloc() noexcept;
void PoolFrame_dealloc(void*) noexcept;

};  // namespace pkpy
