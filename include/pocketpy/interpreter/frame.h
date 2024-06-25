#pragma once

#include "pocketpy/common/memorypool.h"
#include "pocketpy/objects/codeobject.h"
#include "pocketpy/objects/namedict.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/common/config.h"
#include "pocketpy/common/strname.h"

#ifdef __cplusplus
extern "C" {
#endif

PyVar* FastLocals__try_get_by_name(PyVar* locals, const CodeObject* co, py_Name name);
pk_NameDict* FastLocals__to_namedict(PyVar* locals, const CodeObject* co);

typedef struct ValueStack {
    // We allocate extra PK_VM_STACK_SIZE/128 places to keep `_sp` valid when `is_overflow() == true`.
    PyVar* sp;
    PyVar* end;
    PyVar begin[PK_VM_STACK_SIZE + PK_VM_STACK_SIZE / 128];
} ValueStack;

void ValueStack__ctor(ValueStack* self);
void ValueStack__clear(ValueStack* self);

typedef struct UnwindTarget {
    struct UnwindTarget* next;
    int iblock;
    int offset;
} UnwindTarget;

UnwindTarget* UnwindTarget__new(UnwindTarget* next, int iblock, int offset);
void UnwindTarget__delete(UnwindTarget* self);

typedef struct Frame {
    struct Frame* f_back;   // TODO: set this
    const Bytecode* ip;
    const CodeObject* co;
    PyObject* module;
    PyObject* function;     // a function object or NULL (global scope)
    PyVar* p0;              // unwinding base
    PyVar* locals;          // locals base
    const CodeObject* locals_co;
    UnwindTarget* uw_list;
} Frame;


Frame* Frame__new(const CodeObject* co, const PyVar* module, const PyVar* function, PyVar* p0, PyVar* locals, const CodeObject* locals_co);
void Frame__delete(Frame* self);

PK_INLINE int Frame__ip(const Frame* self){
    return self->ip - (Bytecode*)self->co->codes.data;
}

PK_INLINE int Frame__lineno(const Frame* self){
    int ip = Frame__ip(self);
    return c11__getitem(BytecodeEx, &self->co->codes_ex, ip).lineno;
}

PK_INLINE int Frame__iblock(const Frame* self){
    int ip = Frame__ip(self);
    return c11__getitem(BytecodeEx, &self->co->codes_ex, ip).iblock;
}

PK_INLINE pk_NameDict* Frame__f_globals(Frame* self){
    return PyObject__dict(self->module);
}

PK_INLINE PyVar* Frame__f_globals_try_get(Frame* self, py_Name name){
    return pk_NameDict__try_get(Frame__f_globals(self), name);
}

PyVar* Frame__f_closure_try_get(Frame* self, py_Name name);

int Frame__prepare_jump_exception_handler(Frame* self, ValueStack*);
void Frame__prepare_jump_break(Frame* self, ValueStack*, int);
int Frame__prepare_loop_break(Frame* self, ValueStack*);
int Frame__exit_block(Frame* self, ValueStack*, int);

void Frame__gc_mark(Frame* self);
UnwindTarget* Frame__find_unwind_target(Frame* self, int iblock);
void Frame__set_unwind_target(Frame* self, PyVar* sp);

#ifdef __cplusplus
}
#endif


// some patch here
#ifdef __cplusplus
#include "pocketpy/objects/codeobject.hpp"

extern "C"{
    inline PyVar* Frame__f_closure_try_get(Frame* self, StrName name){
        if(self->function == NULL) return NULL;
        pkpy::Function* fn = PyObject__as(pkpy::Function, self->function);
        if(fn->_closure == nullptr) return nullptr;
        return pk_NameDict__try_get(fn->_closure, name);
    }
}
#endif