#pragma once

#include "pocketpy/objects/codeobject.h"
#include "pocketpy/objects/namedict.h"
#include "pocketpy/pocketpy.h"

void FastLocals__to_dict(py_TValue* locals, const CodeObject* co) PY_RETURN;
NameDict* FastLocals__to_namedict(py_TValue* locals, const CodeObject* co);

typedef struct ValueStack {
    py_TValue* sp;
    py_TValue* end;
    // We allocate extra places to keep `_sp` valid to detect stack overflow
    py_TValue begin[PK_VM_STACK_SIZE + PK_MAX_CO_VARNAMES];
} ValueStack;

typedef struct FrameExcInfo {
    int iblock;     // try block index
    int offset;     // stack offset from p0
    py_TValue exc;  // handled exception
} FrameExcInfo;

typedef struct py_Frame {
    struct py_Frame* f_back;
    const CodeObject* co;
    py_StackRef p0;  // unwinding base
    py_GlobalRef module;
    py_Ref globals;  // a module object or a dict object
    py_Ref locals;
    bool is_locals_special;
    int ip;
    c11_vector /*T=FrameExcInfo*/ exc_stack;
} py_Frame;

typedef struct SourceLocation {
    SourceData_ src;
    int lineno;
} SourceLocation;

py_Frame* Frame__new(const CodeObject* co,
                  py_StackRef p0,
                  py_GlobalRef module,
                  py_Ref globals,
                  py_Ref locals,
                  bool is_locals_special);
void Frame__delete(py_Frame* self);

int Frame__lineno(const py_Frame* self);
int Frame__iblock(const py_Frame* self);

int Frame__getglobal(py_Frame* self, py_Name name) PY_RAISE PY_RETURN;
bool Frame__setglobal(py_Frame* self, py_Name name, py_TValue* val) PY_RAISE;
int Frame__delglobal(py_Frame* self, py_Name name) PY_RAISE;

py_Ref Frame__getclosure(py_Frame* self, py_Name name);
py_StackRef Frame__getlocal_noproxy(py_Frame* self, py_Name name);

int Frame__goto_exception_handler(py_Frame* self, ValueStack*, py_Ref);
void Frame__begin_try(py_Frame* self, py_TValue* sp);
FrameExcInfo* Frame__top_exc_info(py_Frame* self);

void Frame__gc_mark(py_Frame* self, c11_vector* p_stack);
SourceLocation Frame__source_location(py_Frame* self);