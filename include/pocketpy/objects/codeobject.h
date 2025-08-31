#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "pocketpy/common/vector.h"
#include "pocketpy/common/smallmap.h"
#include "pocketpy/objects/base.h"
#include "pocketpy/objects/sourcedata.h"
#include "pocketpy/objects/namedict.h"
#include "pocketpy/pocketpy.h"

#define BC_NOARG 0
#define BC_KEEPLINE -1
#define BC_RETURN_VIRTUAL 5

typedef enum FuncType {
    FuncType_UNSET,
    FuncType_NORMAL,
    FuncType_SIMPLE,
    FuncType_GENERATOR,
} FuncType;

typedef enum NameScope {
    NAME_LOCAL,
    NAME_GLOBAL,
} NameScope;

typedef enum CodeBlockType {
    CodeBlockType_NO_BLOCK,
    CodeBlockType_WHILE_LOOP,
    CodeBlockType_TRY,
    /* context blocks (stack-based) */
    CodeBlockType_FOR_LOOP,
    CodeBlockType_WITH,
    /* context blocks (flag-based) */
    CodeBlockType_EXCEPT,
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
    int iblock;       // block index
} BytecodeEx;

typedef struct CodeObject {
    SourceData_ src;
    c11_string* name;

    c11_vector /*T=Bytecode*/ codes;
    c11_vector /*T=CodeObjectByteCodeEx*/ codes_ex;

    c11_vector /*T=py_TValue*/ consts;  // constants
    c11_vector /*T=py_Name*/ varnames;  // local variables
    c11_vector /*T=py_Name*/ names;
    int nlocals;

    c11_smallmap_n2d varnames_inv;
    c11_smallmap_n2d names_inv;

    c11_vector /*T=CodeBlock*/ blocks;
    c11_vector /*T=FuncDecl_*/ func_decls;

    int start_line;
    int end_line;
} CodeObject;

void CodeObject__ctor(CodeObject* self, SourceData_ src, c11_sv name);
void CodeObject__dtor(CodeObject* self);
int CodeObject__add_varname(CodeObject* self, py_Name name);
int CodeObject__add_name(CodeObject* self, py_Name name);
void CodeObject__gc_mark(const CodeObject* self, c11_vector* p_stack);

typedef struct FuncDeclKwArg {
    int index;        // index in co->varnames
    py_Name key;      // name of this argument
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
    c11_smallmap_n2d kw_to_index;
} FuncDecl;

typedef FuncDecl* FuncDecl_;

FuncDecl_ FuncDecl__rcnew(SourceData_ src, c11_sv name);
bool FuncDecl__is_duplicated_arg(const FuncDecl* self, py_Name name);
void FuncDecl__add_arg(FuncDecl* self, py_Name name);
void FuncDecl__add_kwarg(FuncDecl* self, py_Name name, const py_TValue* value);
void FuncDecl__add_starred_arg(FuncDecl* self, py_Name name);
void FuncDecl__add_starred_kwarg(FuncDecl* self, py_Name name);
void FuncDecl__gc_mark(const FuncDecl* self, c11_vector* p_stack);

// runtime function
typedef struct Function {
    FuncDecl_ decl;
    py_GlobalRef module;    // maybe NULL, weak ref
    py_Ref globals;         // maybe NULL, strong ref
    NameDict* closure;      // maybe NULL, strong ref
    PyObject* clazz;        // weak ref; for super()
    py_CFunction cfunc;     // wrapped C function; for decl-based binding
} Function;

void Function__ctor(Function* self, FuncDecl_ decl, py_GlobalRef module, py_Ref globals);
void Function__dtor(Function* self);
