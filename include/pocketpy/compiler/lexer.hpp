#pragma once

#include "pocketpy/objects/sourcedata.hpp"
#include "pocketpy/objects/error.hpp"

#include <variant>

namespace pkpy {

typedef uint8_t TokenIndex;

// clang-format off
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
    // NOTE: These keywords should be sorted in ascending order!!
    "False", "None", "True", "and", "as", "assert", "break", "class", "continue",
    "def", "del", "elif", "else", "except", "finally", "for", "from", "global",
    "if", "import", "in", "is", "lambda", "not", "or", "pass", "raise", "return",
    "try", "while", "with", "yield",
};
// clang-format on

using TokenValue = std::variant<std::monostate, i64, f64, Str>;
const int kTokenCount = sizeof(kTokens) / sizeof(kTokens[0]);

constexpr TokenIndex TK(const char token[]) {
    for(int k = 0; k < kTokenCount; k++) {
        const char* i = kTokens[k];
        const char* j = token;
        while(*i && *j && *i == *j) {
            i++;
            j++;
        }
        if(*i == *j) return k;
    }
    return 255;
}

constexpr inline bool is_raw_string_used(TokenIndex t) { return t == TK("@id") || t == TK("@long"); }

#define TK_STR(t) kTokens[t]

struct Token {
    TokenIndex type;
    const char* start;
    int length;
    int line;
    int brackets_level;
    TokenValue value;

    Str str() const { return Str(start, length); }

    std::string_view sv() const { return std::string_view(start, length); }
};

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

enum class StringType { NORMAL_STRING, RAW_STRING, F_STRING, NORMAL_BYTES };

struct Lexer {
    VM* vm;
    std::shared_ptr<SourceData> src;
    const char* token_start;
    const char* curr_char;
    int current_line = 1;
    vector<Token> nexts;
    small_vector_2<int, 8> indents;
    int brackets_level = 0;

    char peekchar() const noexcept { return *curr_char; }

    bool match_n_chars(int n, char c0) noexcept;
    bool match_string(const char* s) noexcept;
    int eat_spaces() noexcept;

    bool eat_indentation() noexcept;
    char eatchar() noexcept;
    char eatchar_include_newline() noexcept;
    void skip_line_comment() noexcept;
    bool matchchar(char c) noexcept;
    void add_token(TokenIndex type, TokenValue value = {}) noexcept;
    void add_token_2(char c, TokenIndex one, TokenIndex two) noexcept;

    [[nodiscard]] Error* eat_name() noexcept;
    [[nodiscard]] Error* eat_string_until(char quote, bool raw, Str* out) noexcept;
    [[nodiscard]] Error* eat_string(char quote, StringType type) noexcept;
    [[nodiscard]] Error* eat_number() noexcept;
    [[nodiscard]] Error* lex_one_token(bool* eof) noexcept;

    /***** Error Reporter *****/
    [[nodiscard]] Error* _error(bool lexer_err, const char* type, const char* msg, va_list args, i64 userdata=0) noexcept;
    [[nodiscard]] Error* SyntaxError(const char* fmt, ...) noexcept;
    [[nodiscard]] Error* IndentationError(const char* msg) noexcept { return _error(true, "IndentationError", msg, {}); }
    [[nodiscard]] Error* NeedMoreLines() noexcept { return _error(true, "NeedMoreLines", "", {}, 0); }

    Lexer(VM* vm, std::shared_ptr<SourceData> src) noexcept;
    
    [[nodiscard]] Error* run() noexcept;

    void from_precompiled();
    [[nodiscard]] Error* precompile(Str* out);
};

enum class IntParsingResult {
    Success,
    Failure,
    Overflow,
};

IntParsingResult parse_uint(std::string_view text, i64* out, int base) noexcept;

struct TokenDeserializer {
    const char* curr;
    const char* source;

    TokenDeserializer(const char* source) : curr(source), source(source) {}

    char read_char() { return *curr++; }

    bool match_char(char c) {
        if(*curr == c) {
            curr++;
            return true;
        }
        return false;
    }

    std::string_view read_string(char c);
    Str read_string_from_hex(char c);
    int read_count();
    i64 read_uint(char c);
    f64 read_float(char c);
};

}  // namespace pkpy
