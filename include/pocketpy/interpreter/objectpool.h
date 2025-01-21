#pragma once

#include "pocketpy/common/vector.h"

#define kPoolArenaSize (120 * 1024)
#define kMultiPoolCount 5
#define kPoolMaxBlockSize (32*kMultiPoolCount)

typedef struct PoolArena {
    int block_size;
    int block_count;
    int unused_count;
    int* unused;
    char data[kPoolArenaSize];
} PoolArena;

typedef struct Pool {
    c11_vector /* PoolArena* */ arenas;
    c11_vector /* PoolArena* */ not_free_arenas;
    int block_size;
} Pool;

typedef struct MultiPool {
    Pool pools[kMultiPoolCount];
} MultiPool;

void* MultiPool__alloc(MultiPool* self, int size);
int MultiPool__sweep_dealloc(MultiPool* self);
void MultiPool__ctor(MultiPool* self);
void MultiPool__dtor(MultiPool* self);
