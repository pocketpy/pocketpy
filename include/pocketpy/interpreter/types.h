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
