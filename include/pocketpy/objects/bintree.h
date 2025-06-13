#pragma once

#include "pocketpy/objects/base.h"
#include "pocketpy/common/vector.h"
#include "pocketpy/pocketpy.h"

typedef struct BinTreeConfig {
    int (*f_cmp)(void* lhs, void* rhs);
    bool need_free_key;
} BinTreeConfig;

typedef struct BinTree {
    void* key;
    py_TValue value;
    const BinTreeConfig* config;
    struct BinTree* left;
    struct BinTree* right;
} BinTree;

void BinTree__ctor(BinTree* self, void* key, py_Ref value, const BinTreeConfig* config);
void BinTree__dtor(BinTree* self);
void BinTree__set(BinTree* self, void* key, py_Ref value);
py_Ref BinTree__try_get(BinTree* self, void* key);
bool BinTree__contains(BinTree* self, void* key);
void BinTree__apply_mark(BinTree* self, c11_vector* p_stack);
