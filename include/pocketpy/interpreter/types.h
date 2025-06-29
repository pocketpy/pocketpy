#pragma once

#include "pocketpy/common/vector.h"
#include "pocketpy/objects/base.h"

typedef struct {
    uint64_t hash;
    py_TValue key;
    py_TValue val;
} DictEntry;

typedef struct {
    int length;
    uint32_t capacity;
    uint32_t null_index_value;
    bool index_is_short;
    void* indices;
    c11_vector /*T=DictEntry*/ entries;
} Dict;

typedef c11_vector List;

void c11_chunked_array2d__mark(void* ud, c11_vector* p_stack);
void function__gc_mark(void* ud, c11_vector* p_stack);
