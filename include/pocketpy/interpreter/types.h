#pragma once

#include "pocketpy/common/vector.h"
#include "pocketpy/objects/base.h"

#define PK_DICT_MAX_COLLISION 4

typedef struct {
    uint64_t hash;
    py_TValue key;
    py_TValue val;
} DictEntry;

typedef struct {
    int _[PK_DICT_MAX_COLLISION];
} DictIndex;

typedef struct {
    int length;
    uint32_t capacity;
    DictIndex* indices;
    c11_vector /*T=DictEntry*/ entries;
} Dict;

typedef c11_vector List;

void c11_chunked_array2d__mark(void* ud);
void function__gc_mark(void* ud);