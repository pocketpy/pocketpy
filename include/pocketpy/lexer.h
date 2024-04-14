#pragma once

#include "common.h"
#include "error.h"
#include "str.h"
#include "obj.h"

namespace pkpy{

typedef uint8_t TokenIndex;

constexpr const char* kTokens[] = {
    "is not", "not in", "yield from",
    "@eof", "@eol", "@sof",
    "@id", "@num", "@str", "@fstr", "@long", "@bytes", "@imag",
    "@indent", "@dedent",
    /*****************************************/
    "+", "+=", "-", "-=",   // (INPLACE_OP - 1) can get '=' removed
    "*", "*=", "/", "/=", "//", "//=", "%", "%=",
    "&", "&=", "|", "|=", "^", "^=", 
    "<<", "<<=", ">>", ">>=",
    /*****************************************/
    ".", ",", ":", ";", "#", "(", ")", "[", "]", "{", "}",
    "**", "=", ">", "<", "..", "...", "->", "@", "==", "!=", ">=", "<=",
    "++", "--", "~",
    /** KW_BEGIN **/
    "class", "import", "as", "def", "lambda", "pass", "del", "from", "with", "yield",
    "None", "in", "is", "and", "or", "not", "True", "False", "global", "try", "except", "finally",
    "while", "for", "if", "elif", "else", "break", "continue", "return", "assert", "raise"
};

using TokenValue = std::variant<std::monostate, i64, f64, Str>;
const TokenIndex kTokenCount = sizeof(kTokens) / sizeof(kTokens[0]);

constexpr TokenIndex TK(const char token[]) {
    for(int k=0; k<kTokenCount; k++){
        const char* i = kTokens[k];
        const char* j = token;
        while(*i && *j && *i == *j) { i++; j++;}
        if(*i == *j) return k;
    }
    return 255;
}

inline constexpr bool is_raw_string_used(TokenIndex t){
    return t == TK("@id") || t == TK("@long");
}

#define TK_STR(t) kTokens[t]
const std::map<std::string_view, TokenIndex> kTokenKwMap = [](){
    std::map<std::string_view, TokenIndex> map;
    for(int k=TK("class"); k<kTokenCount; k++) map[kTokens[k]] = k;
    return map;
}();

struct Token{
  TokenIndex type;
  const char* start;
  int length;
  int line;
  int brackets_level;
  TokenValue value;

  Str str() const { return Str(start, length);}
  std::string_view sv() const { return std::string_view(start, length);}
};

// https://docs.python.org/3/reference/expressions.html#operator-precedence
enum Precedence {
  PREC_LOWEST,
  PREC_LAMBDA,        // lambda
  PREC_TERNARY,       // ?:
  PREC_LOGICAL_OR,    // or
  PREC_LOGICAL_AND,   // and
  PREC_LOGICAL_NOT,   // not
  /* https://docs.python.org/3/reference/expressions.html#comparisons
   * Unlike C, all comparison operations in Python have the same priority,
   * which is lower than that of any arithmetic, shifting or bitwise operation.
   * Also unlike C, expressions like a < b < c have the interpretation that is conventional in mathematics.
   */
  PREC_COMPARISION,   // < > <= >= != ==, in / is / is not / not in
  PREC_BITWISE_OR,    // |
  PREC_BITWISE_XOR,   // ^
  PREC_BITWISE_AND,   // &
  PREC_BITWISE_SHIFT, // << >>
  PREC_TERM,          // + -
  PREC_FACTOR,        // * / % // @
  PREC_UNARY,         // - not ~
  PREC_EXPONENT,      // **
  PREC_PRIMARY,       // f() x[] a.b 1:2
  PREC_HIGHEST,
};

enum StringType { NORMAL_STRING, RAW_STRING, F_STRING, NORMAL_BYTES };

struct Lexer {
    VM* vm;
    std::shared_ptr<SourceData> src;
    const char* token_start;
    const char* curr_char;
    int current_line = 1;
    std::vector<Token> nexts;
    stack_no_copy<int, small_vector_no_copy_and_move<int, 8>> indents;
    int brackets_level = 0;

    char peekchar() const{ return *curr_char; }
    bool match_n_chars(int n, char c0);
    bool match_string(const char* s);
    int eat_spaces();

    bool eat_indentation();
    char eatchar();
    char eatchar_include_newline();
    int eat_name();
    void skip_line_comment();
    bool matchchar(char c);
    void add_token(TokenIndex type, TokenValue value={});
    void add_token_2(char c, TokenIndex one, TokenIndex two);
    Str eat_string_until(char quote, bool raw);
    void eat_string(char quote, StringType type);

    void eat_number();
    bool lex_one_token();

    /***** Error Reporter *****/
    void throw_err(StrName type, Str msg);
    void throw_err(StrName type, Str msg, int lineno, const char* cursor);
    void SyntaxError(Str msg){ throw_err("SyntaxError", msg); }
    void SyntaxError(){ throw_err("SyntaxError", "invalid syntax"); }
    void IndentationError(Str msg){ throw_err("IndentationError", msg); }
    Lexer(VM* vm, std::shared_ptr<SourceData> src);
    std::vector<Token> run();
};


enum class IntParsingResult{
    Success,
    Failure,
    Overflow,
};

IntParsingResult parse_uint(std::string_view text, i64* out, int base);

} // namespace pkpy
