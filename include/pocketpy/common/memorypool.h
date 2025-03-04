#pragma once

typedef struct FixedMemoryPool {
    int BlockSize;
    int BlockCount;

    char* data;
    char* data_end;
    int exceeded_bytes;

    char** _free_list;
    int _free_list_length;
} FixedMemoryPool;

void FixedMemoryPool__ctor(FixedMemoryPool* self, int BlockSize, int BlockCount);
void FixedMemoryPool__dtor(FixedMemoryPool* self);
void* FixedMemoryPool__alloc(FixedMemoryPool* self);
void FixedMemoryPool__dealloc(FixedMemoryPool* self, void* p);