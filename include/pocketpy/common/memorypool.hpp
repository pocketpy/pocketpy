#pragma once

#include "pocketpy/common/gil.hpp"

#include <cstddef>
#include <cassert>
#include <string>

namespace pkpy{

inline const int kPoolExprBlockSize = 128;
inline const int kPoolFrameBlockSize = 80;

void* PoolExpr_alloc() noexcept;
void PoolExpr_dealloc(void*) noexcept;
void* PoolFrame_alloc() noexcept;
void PoolFrame_dealloc(void*) noexcept;

void* PoolObject_alloc(size_t size) noexcept;
void PoolObject_dealloc(void* p) noexcept;
void PoolObject_shrink_to_fit() noexcept;

};  // namespace pkpy
