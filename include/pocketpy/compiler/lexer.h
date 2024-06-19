#pragma once

#include "pocketpy/common/str.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const char* pk_TokenSymbols[];

typedef struct pk_TokenDeserializer {
    const char* curr;
    const char* source;
} pk_TokenDeserializer;

typedef enum TokenIndex{
    TK_EOF, TK_EOL, TK_SOF,
    TK_ID, TK_NUM, TK_STR, TK_FSTR, TK_LONG, TK_BYTES, TK_IMAG,
    TK_INDENT, TK_DEDENT,
    /***************/
    TK_IS_NOT, TK_NOT_IN, TK_YIELD_FROM,
    /***************/
    TK_ADD, TK_IADD, TK_SUB, TK_ISUB,
    TK_MUL, TK_IMUL, TK_DIV, TK_IDIV, TK_FLOORDIV, TK_IFLOORDIV, TK_MOD, TK_IMOD,
    TK_AND, TK_IAND, TK_OR, TK_IOR, TK_XOR, TK_IXOR,
    TK_LSHIFT, TK_ILSHIFT, TK_RSHIFT, TK_IRSHIFT,
    /***************/
    TK_LPAREN, TK_RPAREN, TK_LBRACKET, TK_RBRACKET, TK_LBRACE, TK_RBRACE,
    TK_DOT, TK_DOTDOT, TK_DOTDOTDOT, TK_COMMA, TK_COLON, TK_SEMICOLON,
    TK_POW, TK_ARROW, TK_HASH, TK_DECORATOR,
    TK_GT, TK_LT, TK_ASSIGN, TK_EQ, TK_NE, TK_GE, TK_LE, TK_INVERT,
    /***************/
    TK_FALSE, TK_NONE, TK_TRUE, TK_AND_KW, TK_AS, TK_ASSERT, TK_BREAK, TK_CLASS, TK_CONTINUE,
    TK_DEF, TK_DEL, TK_ELIF, TK_ELSE, TK_EXCEPT, TK_FINALLY, TK_FOR, TK_FROM, TK_GLOBAL,
    TK_IF, TK_IMPORT, TK_IN, TK_IS, TK_LAMBDA, TK_NOT_KW, TK_OR_KW, TK_PASS, TK_RAISE, TK_RETURN,
    TK_TRY, TK_WHILE, TK_WITH, TK_YIELD,
    /***************/
    TK__COUNT__
} TokenIndex;

void pk_TokenDeserializer__ctor(pk_TokenDeserializer* self, const char* source);
bool pk_TokenDeserializer__match_char(pk_TokenDeserializer* self, char c);
c11_string pk_TokenDeserializer__read_string(pk_TokenDeserializer* self, char c);
pkpy_Str pk_TokenDeserializer__read_string_from_hex(pk_TokenDeserializer* self, char c);
int pk_TokenDeserializer__read_count(pk_TokenDeserializer* self);
int64_t pk_TokenDeserializer__read_uint(pk_TokenDeserializer* self, char c);
double pk_TokenDeserializer__read_float(pk_TokenDeserializer* self, char c);

#ifdef __cplusplus
}
#endif