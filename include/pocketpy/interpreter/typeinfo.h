#pragma once

#include "pocketpy/pocketpy.h"
#include "pocketpy/objects/object.h"

typedef struct py_TypeInfo {
    py_Name name;
    py_Type base;
    struct py_TypeInfo* base_ti;

    py_TValue self;
    py_TValue module;  // the module where the type is defined

    bool is_python;  // is it a python class? (not derived from c object)
    bool is_sealed;  // can it be subclassed?

    void (*dtor)(void*);

    py_TValue annotations;  // type annotations

    void (*on_end_subclass)(struct py_TypeInfo*);  // backdoor for enum module
    void (*gc_mark)(void* ud);

    /* Magic Slots */
    py_TValue magic[64];
} py_TypeInfo;

typedef struct TypeList {
    int length;
    py_TypeInfo* chunks[256];
} TypeList;

void TypeList__ctor(TypeList* self);
void TypeList__dtor(TypeList* self);
py_TypeInfo* TypeList__get(TypeList* self, py_Type index);
py_TypeInfo* TypeList__emplace(TypeList* self);
void TypeList__apply(TypeList* self, void (*f)(py_TypeInfo*, void*), void* ctx);
