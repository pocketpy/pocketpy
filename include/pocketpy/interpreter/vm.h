#pragma once

#include "pocketpy/pocketpy.h"
#include "pocketpy/interpreter/gc.h"
#include "pocketpy/interpreter/frame.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct pk_TypeInfo {
    py_Name name;
    py_Type base;

    py_TValue self;
    py_TValue module;  // the module where the type is defined
    bool subclass_enabled;

    void (*dtor)(void*);
    void (*gc_mark)(void*);

    c11_vector /*T=StrName*/ annotated_fields;

    py_CFunction on_end_subclass;  // backdoor for enum module

    /* Magic Slots */
    py_TValue magic[64];
} pk_TypeInfo;

void pk_TypeInfo__ctor(pk_TypeInfo* self,
                       py_Name name,
                       py_Type index,
                       py_Type base,
                       const py_TValue* module,
                       bool subclass_enabled);
void pk_TypeInfo__dtor(pk_TypeInfo* self);

typedef struct pk_VM {
    Frame* top_frame;

    pk_NameDict modules;
    c11_vector /*T=pk_TypeInfo*/ types;

    py_TValue StopIteration;  // a special Exception class
    py_TValue builtins;       // builtins module
    py_TValue main;           // __main__ module

    void (*_ceval_on_step)(Frame*, Bytecode);
    unsigned char* (*_import_file)(const char*);
    void (*_stdout)(const char*, ...);
    void (*_stderr)(const char*, ...);

    // singleton objects
    py_TValue True, False, None, NotImplemented, Ellipsis;

    py_TValue last_retval;
    py_TValue reg[8];  // users' registers

    py_TValue __curr_class;
    py_TValue __cached_object_new;
    FuncDecl_ __dynamic_func_decl;
    py_TValue __vectorcall_buffer[PK_MAX_CO_VARNAMES];

    pk_ManagedHeap heap;
    ValueStack stack;  // put `stack` at the end for better cache locality
} pk_VM;

void pk_VM__ctor(pk_VM* self);
void pk_VM__dtor(pk_VM* self);

void pk_VM__push_frame(pk_VM* self, Frame* frame);
void pk_VM__pop_frame(pk_VM* self);

void pk_VM__init_builtins(pk_VM* self);

typedef enum pk_FrameResult {
    RES_RETURN,
    RES_CALL,
    RES_YIELD,
    RES_ERROR,
} pk_FrameResult;

pk_FrameResult pk_VM__run_top_frame(pk_VM* self);

py_Type pk_VM__new_type(pk_VM* self,
                        const char* name,
                        py_Type base,
                        const py_TValue* module,
                        bool subclass_enabled);

// type registration
py_Type pk_list__register();

#ifdef __cplusplus
}
#endif