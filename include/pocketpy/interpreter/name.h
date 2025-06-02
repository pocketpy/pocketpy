#pragma once

#include "pocketpy/objects/base.h"
#include "pocketpy/common/smallmap.h"

typedef struct {
    int size;       // size of the data excluding the null-terminator
    py_TValue obj;  // cached `str` object (lazy initialized)
    char data[];    // null-terminated data
} InternedEntry;

typedef struct {
    c11_smallmap_s2n interned;
} InternedNames;

void InternedNames__ctor(InternedNames* self);
void InternedNames__dtor(InternedNames* self);

#define MAGIC_METHOD(x) extern py_Name x;
#include "pocketpy/xmacros/magics.h"
#undef MAGIC_METHOD