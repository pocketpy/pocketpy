#pragma once

#include "pocketpy/common/vector.h"
#include "pocketpy/common/str.h"

#define kPoolArenaSize (120 * 1024)
#define kMultiPoolCount 5
#define kPoolMaxBlockSize (32 * kMultiPoolCount)

typedef uint16_t PoolBlockIndex;

typedef struct UsedBlockListNode {
    PoolBlockIndex next;
    PoolBlockIndex data;
} UsedBlockListNode;

typedef struct UsedBlockList {
    UsedBlockListNode* nodes;
    int head_idx;
} UsedBlockList;

void UsedBlockList__ctor(UsedBlockList* self, int capacity);
void UsedBlockList__dtor(UsedBlockList* self);

typedef struct PoolArena {
    int block_size;
    int block_count;
    int unused_length;
    PoolBlockIndex* unused;
    UsedBlockList used_blocks;

    union {
        char data[kPoolArenaSize];
        int64_t _align64;
    };
} PoolArena;

typedef struct Pool {
    c11_vector /* PoolArena* */ arenas;
    c11_vector /* PoolArena* */ no_free_arenas;
    int block_size;
} Pool;

typedef struct MultiPool {
    Pool pools[kMultiPoolCount];
} MultiPool;

void* MultiPool__alloc(MultiPool* self, int size);
int MultiPool__sweep_dealloc(MultiPool* self);
void MultiPool__ctor(MultiPool* self);
void MultiPool__dtor(MultiPool* self);
c11_string* MultiPool__summary(MultiPool* self);