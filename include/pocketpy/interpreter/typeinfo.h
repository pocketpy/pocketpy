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
    bool is_final;  // can it be subclassed?

    bool (*getattribute)(py_Ref self, py_Name name) PY_RAISE PY_RETURN;
    bool (*setattribute)(py_Ref self, py_Name name, py_Ref val) PY_RAISE PY_RETURN;
    bool (*delattribute)(py_Ref self, py_Name name) PY_RAISE;
    bool (*getunboundmethod)(py_Ref self, py_Name name) PY_RETURN;

    py_TValue annotations;
    py_Dtor dtor;  // destructor for this type, NULL if no dtor
    void (*on_end_subclass)(struct py_TypeInfo*);  // backdoor for enum module
} py_TypeInfo;

py_TypeInfo* pk_typeinfo(py_Type type);
py_ItemRef pk_tpfindname(py_TypeInfo* ti, py_Name name);
#define pk_tpfindmagic pk_tpfindname

py_Type pk_newtype(const char* name,
                   py_Type base,
                   const py_GlobalRef module,
                   void (*dtor)(void*),
                   bool is_python,
                   bool is_final);


py_Type pk_newtypewithmode(py_Name name,
                   py_Type base,
                   const py_GlobalRef module,
                   void (*dtor)(void*),
                   bool is_python,
                   bool is_final, enum py_CompileMode mode);