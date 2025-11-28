#pragma once

#include "pocketpy/common/vector.h"
#include "pocketpy/common/str.h"

#define kPoolArenaSize (120 * 1024)
#define kMultiPoolCount 5
// #define kPoolMaxBlockSize (32 * kMultiPoolCount)

typedef struct PoolArena {
    int block_size;
    int block_count;
    int unused_length;

    union {
        char data[kPoolArenaSize];
        int64_t _align64;
    };

    int unused[];
} PoolArena;

typedef struct Pool {
    c11_vector /* PoolArena* */ arenas;
    int available_index;
    int block_size;
} Pool;

typedef struct MultiPool {
    Pool pools[kMultiPoolCount];
} MultiPool;

void* MultiPool__alloc(MultiPool* self, int size);
int MultiPool__sweep_dealloc(MultiPool* self, int* out_types);
void MultiPool__ctor(MultiPool* self);
void MultiPool__dtor(MultiPool* self);
size_t MultiPool__total_allocated_bytes(MultiPool* self);
c11_string* MultiPool__summary(MultiPool* self);