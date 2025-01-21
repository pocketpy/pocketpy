#pragma once

#define kPoolExprBlockSize      128
#define kPoolFrameBlockSize     80

void MemoryPools__initialize();
void MemoryPools__finalize();

void* PoolExpr_alloc();
void PoolExpr_dealloc(void*);
void* PoolFrame_alloc();
void PoolFrame_dealloc(void*);
