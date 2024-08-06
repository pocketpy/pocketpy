#pragma once

#define kPoolExprBlockSize      128
#define kPoolFrameBlockSize     80
#define kPoolObjectBlockSize    80

#define kPoolObjectArenaSize    (256*1024)
#define kPoolObjectMaxBlocks    (kPoolObjectArenaSize / kPoolObjectBlockSize)

void MemoryPools__initialize();
void MemoryPools__finalize();

void* PoolExpr_alloc();
void PoolExpr_dealloc(void*);
void* PoolFrame_alloc();
void PoolFrame_dealloc(void*);

void* PoolObject_alloc();
void PoolObject_dealloc(void* p);
void PoolObject_shrink_to_fit();

void Pools_debug_info(char* buffer, int size);
