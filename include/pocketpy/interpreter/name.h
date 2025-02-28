#pragma once

#include "pocketpy/objects/base.h"
#include "pocketpy/common/smallmap.h"

typedef struct {
    char* data;     // null-terminated data
    int size;       // size of the data excluding the null-terminator
    py_TValue obj;  // cached `str` object (lazy initialized)
} RInternedEntry;

typedef struct {
    c11_smallmap_s2n interned;
    c11_vector /* T=RInternedEntry */ r_interned;
} InternedNames;

void InternedNames__ctor(InternedNames* self);
void InternedNames__dtor(InternedNames* self);
