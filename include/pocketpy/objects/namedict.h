#pragma once

#include "pocketpy/objects/base.h"
#include "pocketpy/pocketpy.h"

typedef struct NameDict_KV {
    py_Name key;
    py_TValue value;
} NameDict_KV;

// https://github.com/pocketpy/pocketpy/blob/v1.x/include/pocketpy/namedict.h
typedef struct NameDict {
    int length;
    float load_factor;
    int capacity;
    int critical_size;
    uintptr_t mask;
    NameDict_KV* items;
} NameDict;

NameDict* NameDict__new(float load_factor);
void NameDict__delete(NameDict* self);
void NameDict__ctor(NameDict* self, float load_factor);
void NameDict__dtor(NameDict* self);
py_TValue* NameDict__try_get(NameDict* self, py_Name key);
bool NameDict__contains(NameDict* self, py_Name key);
void NameDict__set(NameDict* self, py_Name key, py_TValue* value);
bool NameDict__del(NameDict* self, py_Name key);
void NameDict__clear(NameDict* self);