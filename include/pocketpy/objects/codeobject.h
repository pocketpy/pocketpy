#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BC_NOARG        0
#define BC_KEEPLINE     -1

typedef enum BindType {
    BindType_FUNCTION,
    BindType_STATICMETHOD,
    BindType_CLASSMETHOD,
} BindType;

typedef enum FuncType {
    FuncType_UNSET,
    FuncType_NORMAL,
    FuncType_SIMPLE,
    FuncType_EMPTY,
    FuncType_GENERATOR,
} FuncType;

typedef enum NameScope {
    NAME_LOCAL,
    NAME_GLOBAL,
    NAME_GLOBAL_UNKNOWN
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

#ifdef __cplusplus
}
#endif