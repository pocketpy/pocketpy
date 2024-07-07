#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "pocketpy/common/vector.h"
#include "pocketpy/common/smallmap.h"
#include "pocketpy/objects/base.h"
#include "pocketpy/objects/sourcedata.h"
#include "pocketpy/objects/namedict.h"
#include "pocketpy/pocketpy.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BC_NOARG 0
#define BC_KEEPLINE -1

typedef enum FuncType {
    FuncType_UNSET,
    FuncType_NORMAL,
    FuncType_SIMPLE,
    FuncType_GENERATOR,
} FuncType;

typedef enum NameScope {
    NAME_LOCAL,
    NAME_GLOBAL,
    NAME_GLOBAL_UNKNOWN,
} NameScope;

typedef enum CodeBlockType {
    CodeBlockType_NO_BLOCK,
    CodeBlockType_FOR_LOOP,
    CodeBlockType_WHILE_LOOP,
    CodeBlockType_CONTEXT_MANAGER,
    CodeBlockType_TRY_EXCEPT,
} CodeBlockType;

typedef enum Opcode {

#define OPCODE(name) OP_##name,
#include "pocketpy/xmacros/opcodes.h"
#undef OPCODE
} Opcode;

typedef struct Bytecode {
    uint8_t op;
    uint16_t arg;
} Bytecode;

void Bytecode__set_signed_arg(Bytecode* self, int arg);
bool Bytecode__is_forward_jump(const Bytecode* self);

typedef struct CodeBlock {
    CodeBlockType type;
    int parent;  // parent index in blocks
    int start;   // start index of this block in codes, inclusive
    int end;     // end index of this block in codes, exclusive
    int end2;    // ...
} CodeBlock;

typedef struct BytecodeEx {
    int lineno;       // line number for each bytecode
    bool is_virtual;  // whether this bytecode is virtual (not in source code)
    int iblock;       // block index
} BytecodeEx;

typedef struct CodeObject {
    pk_SourceData_ src;
    c11_string* name;

    c11_vector /*T=Bytecode*/ codes;
    c11_vector /*T=CodeObjectByteCodeEx*/ codes_ex;

    c11_vector /*T=py_TValue*/ consts;  // constants
    c11_vector /*T=py_Name*/ varnames;  // local variables
    int nlocals;                        // cached varnames.size()

    c11_smallmap_n2i varnames_inv;
    c11_smallmap_n2i labels;

    c11_vector /*T=CodeBlock*/ blocks;
    c11_vector /*T=FuncDecl_*/ func_decls;

    int start_line;
    int end_line;
} CodeObject;

void CodeObject__ctor(CodeObject* self, pk_SourceData_ src, c11_sv name);
void CodeObject__dtor(CodeObject* self);
void CodeObject__gc_mark(const CodeObject* self);

typedef struct FuncDeclKwArg {
    int index;        // index in co->varnames
    uint16_t key;     // name of this argument
    py_TValue value;  // default value
} FuncDeclKwArg;

typedef struct FuncDecl {
    RefCounted rc;
    CodeObject code;  // strong ref

    c11_vector /*T=int*/ args;      // indices in co->varnames
    c11_vector /*T=KwArg*/ kwargs;  // indices in co->varnames

    int starred_arg;    // index in co->varnames, -1 if no *arg
    int starred_kwarg;  // index in co->varnames, -1 if no **kwarg
    bool nested;        // whether this function is nested

    const char* docstring;  // docstring of this function (weak ref)

    FuncType type;
    c11_smallmap_n2i kw_to_index;
} FuncDecl;

typedef FuncDecl* FuncDecl_;

FuncDecl_ FuncDecl__rcnew(pk_SourceData_ src, c11_sv name);
void FuncDecl__add_kwarg(FuncDecl* self, int index, uint16_t key, const py_TValue* value);

// runtime function
typedef struct Function {
    FuncDecl_ decl;
    PyObject* module;      // weak ref
    PyObject* clazz;       // weak ref
    pk_NameDict* closure;  // strong ref
    py_CFunction cfunc;    // wrapped C function
} Function;

void Function__ctor(Function* self, FuncDecl_ decl, PyObject* module);
void Function__dtor(Function* self);

#ifdef __cplusplus
}
#endif