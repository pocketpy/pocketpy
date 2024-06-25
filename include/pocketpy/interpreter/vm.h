#pragma once

#include "pocketpy/pocketpy.h"
#include "pocketpy/interpreter/gc.h"
#include "pocketpy/interpreter/frame.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct pk_TypeInfo{
    py_Name name;
    py_Type base;

    PyVar self;      // the type object itself
    PyVar module;    // the module where the type is defined
    bool subclass_enabled;

    void (*dtor)(void*);
    void (*gc_mark)(void*);

    c11_vector/*T=StrName*/ annotated_fields;

    /* Magic Caches */
    // unary operators
    py_CFunction m__repr__, m__str__, m__hash__, m__len__;
    py_CFunction m__iter__, m__next__, m__neg__, m__invert__;
    // binary operators
    py_CFunction m__eq__, m__lt__, m__le__, m__gt__, m__ge__, m__contains__;
    py_CFunction m__add__, m__sub__, m__mul__, m__truediv__, m__floordiv__;
    py_CFunction m__mod__, m__pow__, m__matmul__;
    py_CFunction m__lshift__, m__rshift__, m__and__, m__xor__, m__or__;
    // indexer
    py_CFunction m__getitem__, m__setitem__, m__delitem__;
    // attribute access (internal use only)
    py_CFunction m__getattr__, m__setattr__, m__delattr__;
    // backdoors
    py_CFunction on_end_subclass;   // for enum module
} pk_TypeInfo;

void pk_TypeInfo__ctor(pk_TypeInfo* self, py_Name name, py_Type base, PyObject* obj, const PyVar* module, bool subclass_enabled);
void pk_TypeInfo__dtor(pk_TypeInfo* self);

typedef struct pk_VM {
    Frame* top_frame;
    
    pk_NameDict modules;
    c11_vector/*T=pk_TypeInfo*/ types;

    PyVar StopIteration;    // a special Exception class
    PyVar builtins;         // builtins module
    PyVar main;             // __main__ module

    void (*_ceval_on_step)(Frame*, Bytecode);
    unsigned char* (*_import_file)(const char*);
    void (*_stdout)(const char*);
    void (*_stderr)(const char*);
    
    // singleton objects
    PyVar True, False, None, NotImplemented, Ellipsis;
    // last error
    py_Error* last_error;

    PyObject* __curr_class;
    PyObject* __cached_object_new;
    FuncDecl_ __dynamic_func_decl;
    PyVar __vectorcall_buffer[PK_MAX_CO_VARNAMES];

    void* userdata;     // for user-defined data (unused by pkpy itself)

    pk_ManagedHeap heap;
    ValueStack stack;   // put `stack` at the end for better cache locality
} pk_VM;

void pk_VM__ctor(pk_VM* self);
void pk_VM__dtor(pk_VM* self);

void pk_VM__push_frame(pk_VM* self, Frame* frame);
void pk_VM__pop_frame(pk_VM* self);

void pk_VM__init_builtins(pk_VM* self);

typedef enum pk_FrameResult{
    RES_RETURN,
    RES_CALL,
    RES_YIELD,
    RES_ERROR,
} pk_FrameResult;

pk_FrameResult pk_VM__run_top_frame(pk_VM* self);

py_Type pk_VM__new_type(pk_VM* self, const char* name, py_Type base, const PyVar* module, bool subclass_enabled);

#ifdef __cplusplus
}
#endif