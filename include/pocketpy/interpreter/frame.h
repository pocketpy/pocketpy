#pragma once

#include "pocketpy/common/memorypool.h"
#include "pocketpy/objects/codeobject.h"
#include "pocketpy/objects/namedict.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/common/strname.h"
#include "pocketpy/pocketpy.h"

py_TValue* FastLocals__try_get_by_name(py_TValue* locals, const CodeObject* co, py_Name name);
NameDict* FastLocals__to_namedict(py_TValue* locals, const CodeObject* co);

typedef struct ValueStack {
    py_TValue* sp;
    py_TValue* end;
    // We allocate extra places to keep `_sp` valid to detect stack overflow
    py_TValue begin[PK_VM_STACK_SIZE + PK_MAX_CO_VARNAMES * 2];
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
    struct Frame* f_back;
    const Bytecode* ip;
    const CodeObject* co;
    py_GlobalRef module;
    py_StackRef p0;      // unwinding base
    py_StackRef locals;  // locals base
    bool has_function;   // is p0 a function?
    bool is_dynamic;     // is dynamic frame?
    UnwindTarget* uw_list;
} Frame;

Frame* Frame__new(const CodeObject* co,
                  py_GlobalRef module,
                  py_StackRef p0,
                  py_StackRef locals,
                  bool has_function);
void Frame__delete(Frame* self);

int Frame__ip(const Frame* self);
int Frame__lineno(const Frame* self);
int Frame__iblock(const Frame* self);
py_TValue* Frame__f_locals_try_get(Frame* self, py_Name name);
py_TValue* Frame__f_closure_try_get(Frame* self, py_Name name);

int Frame__prepare_jump_exception_handler(Frame* self, ValueStack*);

UnwindTarget* Frame__find_unwind_target(Frame* self, int iblock);
void Frame__set_unwind_target(Frame* self, py_TValue* sp);

void Frame__gc_mark(Frame* self);