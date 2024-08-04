#pragma once

#include "pocketpy/objects/codeobject.h"
#include "pocketpy/pocketpy.h"
#include "pocketpy/interpreter/gc.h"
#include "pocketpy/interpreter/frame.h"
#include "pocketpy/interpreter/vfs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct pk_TypeInfo {
    py_Name name;
    py_Type base;

    py_TValue self;
    py_TValue module;  // the module where the type is defined

    bool is_python;  // is it a python class? (not derived from c object)
    bool is_sealed;  // can it be subclassed?

    void (*dtor)(void*);

    c11_vector /*T=py_Name*/ annotated_fields;

    void (*on_end_subclass)(struct pk_TypeInfo*);  // backdoor for enum module

    /* Magic Slots */
    py_TValue magic[64];
} pk_TypeInfo;

typedef struct pk_VM {
    Frame* top_frame;

    pk_NameDict modules;
    c11_vector /*T=pk_TypeInfo*/ types;

    py_TValue builtins;  // builtins module
    py_TValue main;      // __main__ module

    void (*ceval_on_step)(Frame*, Bytecode);
    unsigned char* (*import_file)(const char*, int*);
    void (*print)(const char*);

    py_TValue last_retval;
    py_TValue curr_exception;
    bool is_stopiteration;

    py_TValue reg[8];  // users' registers

    Vfs __vfs;
    py_TValue* __curr_class;
    FuncDecl_ __dynamic_func_decl;
    py_TValue __vectorcall_buffer[PK_MAX_CO_VARNAMES];

    pk_ManagedHeap heap;
    ValueStack stack;  // put `stack` at the end for better cache locality
} pk_VM;

void pk_VM__ctor(pk_VM* self);
void pk_VM__dtor(pk_VM* self);

void pk_VM__push_frame(pk_VM* self, Frame* frame);
void pk_VM__pop_frame(pk_VM* self);

bool pk__parse_int_slice(py_Ref slice, int length, int* start, int* stop, int* step);
bool pk__normalize_index(int* index, int length);

void pk_list__mark(void* ud, void (*marker)(py_TValue*));
void pk_dict__mark(void* ud, void (*marker)(py_TValue*));

typedef enum pk_FrameResult {
    RES_RETURN,
    RES_CALL,
    RES_YIELD,
    RES_ERROR,
} pk_FrameResult;

pk_FrameResult pk_VM__run_top_frame(pk_VM* self);

py_Type pk_newtype(const char* name,
                   py_Type base,
                   const py_GlobalRef module,
                   void (*dtor)(void*),
                   bool is_python,
                   bool is_sealed);

pk_FrameResult pk_VM__vectorcall(pk_VM* self, uint16_t argc, uint16_t kwargc, bool opcall);

const char* pk_opname(Opcode op);

py_TValue* pk_arrayview(py_Ref self, int* length);
int pk_arrayequal(py_TValue* lhs, int lhs_length, py_TValue* rhs, int rhs_length);
bool pk_arrayiter(py_Ref val);
bool pk_arraycontains(py_Ref self, py_Ref val);

bool pk_pushmethod(py_StackRef self, py_Name name);
bool pk_callmagic(py_Name name, int argc, py_Ref argv);

/// Assumes [a, b] are on the stack, performs a binary op.
/// The result is stored in `self->last_retval`.
/// The stack remains unchanged.
bool pk_stack_binaryop(pk_VM* self, py_Name op, py_Name rop);

void pk_print_stack(pk_VM* self, Frame* frame, Bytecode byte);

// type registration
void pk_object__register();
void pk_number__register();
py_Type pk_str__register();
py_Type pk_str_iterator__register();
py_Type pk_bytes__register();
py_Type pk_dict__register();
py_Type pk_dict_items__register();
py_Type pk_list__register();
py_Type pk_tuple__register();
py_Type pk_array_iterator__register();
py_Type pk_slice__register();
py_Type pk_function__register();
py_Type pk_nativefunc__register();
py_Type pk_range__register();
py_Type pk_range_iterator__register();
py_Type pk_BaseException__register();
py_Type pk_Exception__register();

py_TValue pk_builtins__register();

void pk__add_module_pkpy();

#ifdef __cplusplus
}
#endif