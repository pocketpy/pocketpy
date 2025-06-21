#pragma once

#include "pocketpy/pocketpy.h"
#include "pocketpy/common/vector.h"
#include "pocketpy/objects/object.h"

typedef struct py_TypeInfo {
    py_Name name;
    py_Type index;
    py_Type base;
    struct py_TypeInfo* base_ti;

    py_TValue self;
    py_GlobalRef module;

    bool is_python;  // is it a python class? (not derived from c object)
    bool is_sealed;  // can it be subclassed?

    py_Dtor dtor;  // destructor for this type, NULL if no dtor

    py_TValue annotations;

    void (*on_end_subclass)(struct py_TypeInfo*);  // backdoor for enum module
} py_TypeInfo;

py_TypeInfo* pk_typeinfo(py_Type type);
py_ItemRef pk_tpfindname(py_TypeInfo* ti, py_Name name);
#define pk_tpfindmagic pk_tpfindname