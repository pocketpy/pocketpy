#pragma once

#include "pocketpy/objects/base.h"
#include "pocketpy/common/vector.h"

typedef struct tuple_iterator {
    py_TValue* p;
    int length;
    int index;
} tuple_iterator;

typedef struct list_iterator {
    c11_vector* vec;
    int index;
} list_iterator;
