#pragma once

#include "pocketpy/common/vector.h"
#include "pocketpy/common/str.h"
#include "pocketpy/objects/base.h"
#include <stdint.h>

#define SMALLMAP_T__HEADER
#define K uint16_t
#define V py_TValue
#define NAME NameDict
#include "pocketpy/xmacros/smallmap.h"
#undef SMALLMAP_T__HEADER

/* A simple binary tree for storing modules. */
typedef struct ModuleDict {
    const char* path;
    py_TValue module;
    struct ModuleDict* left;
    struct ModuleDict* right;
} ModuleDict;

void ModuleDict__ctor(ModuleDict* self, const char* path, py_TValue module);
void ModuleDict__dtor(ModuleDict* self);
void ModuleDict__set(ModuleDict* self, const char* key, py_TValue val);
py_TValue* ModuleDict__try_get(ModuleDict* self, const char* path);
bool ModuleDict__contains(ModuleDict* self, const char* path);
void ModuleDict__apply_mark(ModuleDict* self, void (*marker)(PyObject*));
