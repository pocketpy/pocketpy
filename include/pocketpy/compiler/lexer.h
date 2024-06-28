#pragma once

#include "pocketpy/common/str.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const char* pk_TokenSymbols[];

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

typedef struct TokenValue {
    int index;
    union {
        int64_t _i64;   // 0
        double _f64;    // 1
        py_Str _str;    // 2
    };
} TokenValue;

typedef struct Token {
    TokenIndex type;
    const char* start;
    int length;
    int line;
    int brackets_level;
    TokenValue value;
} Token;

// https://docs.python.org/3/reference/expressions.html#operator-precedence
enum Precedence {
    PREC_LOWEST,
    PREC_LAMBDA,       // lambda
    PREC_TERNARY,      // ?:
    PREC_LOGICAL_OR,   // or
    PREC_LOGICAL_AND,  // and
    PREC_LOGICAL_NOT,  // not
    /* https://docs.python.org/3/reference/expressions.html#comparisons
     * Unlike C, all comparison operations in Python have the same priority,
     * which is lower than that of any arithmetic, shifting or bitwise operation.
     * Also unlike C, expressions like a < b < c have the interpretation that is conventional in mathematics.
     */
    PREC_COMPARISION,    // < > <= >= != ==, in / is / is not / not in
    PREC_BITWISE_OR,     // |
    PREC_BITWISE_XOR,    // ^
    PREC_BITWISE_AND,    // &
    PREC_BITWISE_SHIFT,  // << >>
    PREC_TERM,           // + -
    PREC_FACTOR,         // * / % // @
    PREC_UNARY,          // - not ~
    PREC_EXPONENT,       // **
    PREC_PRIMARY,        // f() x[] a.b 1:2
    PREC_HIGHEST,
};

#define is_raw_string_used(t) ((t) == TK_ID || (t) == TK_LONG)

typedef struct pk_TokenDeserializer {
    const char* curr;
    const char* source;
} pk_TokenDeserializer;

void pk_TokenDeserializer__ctor(pk_TokenDeserializer* self, const char* source);
bool pk_TokenDeserializer__match_char(pk_TokenDeserializer* self, char c);
c11_string pk_TokenDeserializer__read_string(pk_TokenDeserializer* self, char c);
py_Str pk_TokenDeserializer__read_string_from_hex(pk_TokenDeserializer* self, char c);
int pk_TokenDeserializer__read_count(pk_TokenDeserializer* self);
int64_t pk_TokenDeserializer__read_uint(pk_TokenDeserializer* self, char c);
double pk_TokenDeserializer__read_float(pk_TokenDeserializer* self, char c);

typedef enum IntParsingResult{
    IntParsing_SUCCESS,
    IntParsing_FAILURE,
    IntParsing_OVERFLOW,
} IntParsingResult;

IntParsingResult parse_uint(c11_string text, int64_t* out, int base);

#ifdef __cplusplus
}
#endif