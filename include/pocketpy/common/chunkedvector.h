#pragma once

#include "pocketpy/common/vector.h"

typedef struct c11_chunkedvector_chunk {
    int length;
    int capacity;
    void* data;
} c11_chunkedvector_chunk;

typedef struct c11_chunkedvector {
    c11_vector /*T=c11_chunkedvector_chunk*/ chunks;
    int length;
    int capacity;
    int elem_size;
    int initial_chunks;
} c11_chunkedvector;

// chunks[0]: size=1, total_capacity=1
// chunks[1]: size=2, total_capacity=3
// chunks[2]: size=4, total_capacity=7
// chunks[3]: size=8, total_capacity=15
// chunks[4]: size=16, total_capacity=31
// chunks[5]: size=32, total_capacity=63
// ...
// chunks[n]: size=2^n, total_capacity=2^(n+1)-1

void c11_chunkedvector__ctor(c11_chunkedvector* self, int elem_size, int initial_chunks);
void c11_chunkedvector__dtor(c11_chunkedvector* self);
void* c11_chunkedvector__emplace(c11_chunkedvector* self);
void* c11_chunkedvector__at(c11_chunkedvector* self, int index);