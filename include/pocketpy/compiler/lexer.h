#pragma once

#include "pocketpy/common/str.h"
#include "pocketpy/objects/sourcedata.h"
#include "pocketpy/objects/error.h"
#include <stdint.h>

extern const char* TokenSymbols[];

typedef enum TokenIndex {
    TK_EOF,
    TK_EOL,
    TK_SOF,
    TK_ID,
    TK_NUM,
    TK_STR,
    TK_FSTR_BEGIN,
    TK_FSTR_CPNT,
    TK_FSTR_SPEC,
    TK_FSTR_END,
    TK_BYTES,
    TK_IMAG,
    TK_INDENT,
    TK_DEDENT,
    /***************/
    TK_IS_NOT,
    TK_NOT_IN,
    TK_YIELD_FROM,
    /***************/
    TK_ADD,
    TK_IADD,
    TK_SUB,
    TK_ISUB,
    TK_MUL,
    TK_IMUL,
    TK_DIV,
    TK_IDIV,
    TK_FLOORDIV,
    TK_IFLOORDIV,
    TK_MOD,
    TK_IMOD,
    TK_AND,
    TK_IAND,
    TK_OR,
    TK_IOR,
    TK_XOR,
    TK_IXOR,
    TK_LSHIFT,
    TK_ILSHIFT,
    TK_RSHIFT,
    TK_IRSHIFT,
    /***************/
    TK_LPAREN,
    TK_RPAREN,
    TK_LBRACKET,
    TK_RBRACKET,
    TK_LBRACE,
    TK_RBRACE,
    TK_DOT,
    TK_DOTDOT,
    TK_DOTDOTDOT,
    TK_COMMA,
    TK_COLON,
    TK_SEMICOLON,
    TK_POW,
    TK_ARROW,
    TK_HASH,
    TK_DECORATOR,
    TK_GT,
    TK_LT,
    TK_ASSIGN,
    TK_EQ,
    TK_NE,
    TK_GE,
    TK_LE,
    TK_INVERT,
    /***************/
    TK_FALSE,
    TK_NONE,
    TK_TRUE,
    TK_AND_KW,
    TK_AS,
    TK_ASSERT,
    TK_BREAK,
    TK_CLASS,
    TK_CONTINUE,
    TK_DEF,
    TK_DEL,
    TK_ELIF,
    TK_ELSE,
    TK_EXCEPT,
    TK_FINALLY,
    TK_FOR,
    TK_FROM,
    TK_GLOBAL,
    TK_IF,
    TK_IMPORT,
    TK_IN,
    TK_IS,
    TK_LAMBDA,
    TK_MATCH,
    TK_NOT_KW,
    TK_OR_KW,
    TK_PASS,
    TK_RAISE,
    TK_RETURN,
    TK_TRY,
    TK_WHILE,
    TK_WITH,
    TK_YIELD,
    /***************/
    TK__COUNT__
} TokenIndex;

enum TokenValueIndex {
    TokenValue_EMPTY = 0,
    TokenValue_I64 = 1,
    TokenValue_F64 = 2,
    TokenValue_STR = 3,
};

typedef struct TokenValue {
    enum TokenValueIndex index;  // 0: empty

    union {
        int64_t _i64;      // 1
        double _f64;       // 2
        c11_string* _str;  // 3
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
    PREC_LOWEST = 0,
    PREC_LAMBDA,       // lambda
    PREC_TERNARY,      // ?:
    PREC_LOGICAL_OR,   // or
    PREC_LOGICAL_AND,  // and
    PREC_LOGICAL_NOT,  // not
    /* https://docs.python.org/3/reference/expressions.html#comparisons
     * Unlike C, all comparison operations in Python have the same priority,
     * which is lower than that of any arithmetic, shifting or bitwise operation.
     * Also unlike C, expressions like a < b < c have the interpretation that is conventional in
     * mathematics.
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

Error* Lexer__process(SourceData_ src, Token** out_tokens, int* out_length);

#define Token__sv(self)                                                                            \
    (c11_sv) { (self)->start, (self)->length }
