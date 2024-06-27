#pragma once

#include "pocketpy/objects/error.hpp"
#include "pocketpy/objects/sourcedata.h"
#include "pocketpy/compiler/lexer.h"

#include <variant>

namespace pkpy {


struct Lexer {
    PK_ALWAYS_PASS_BY_POINTER(Lexer)

    VM* vm;
    pkpy_SourceData_ src;
    const char* token_start;
    const char* curr_char;
    int current_line = 1;
    vector<Token> nexts;
    small_vector_2<int, 8> indents;
    int brackets_level = 0;
    bool used = false;

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
    [[nodiscard]] Error* _error(bool lexer_err, const char* type, const char* msg, va_list* args, i64 userdata=0) noexcept;
    [[nodiscard]] Error* SyntaxError(const char* fmt, ...) noexcept;
    [[nodiscard]] Error* IndentationError(const char* msg) noexcept { return _error(true, "IndentationError", msg, NULL); }
    [[nodiscard]] Error* NeedMoreLines() noexcept { return _error(true, "NeedMoreLines", "", NULL, 0); }

    [[nodiscard]] Error* run() noexcept;
    [[nodiscard]] Error* from_precompiled() noexcept;
    [[nodiscard]] Error* precompile(Str* out) noexcept;

    Lexer(VM* vm, std::string_view source, const Str& filename, CompileMode mode) noexcept{
        src = pkpy_SourceData__rcnew({source.data(), (int)source.size()}, &filename, mode);
        this->token_start = py_Str__data(&src->source);
        this->curr_char = py_Str__data(&src->source);
    }

    ~Lexer(){
        PK_DECREF(src);
    }
};

enum class IntParsingResult {
    Success,
    Failure,
    Overflow,
};

IntParsingResult parse_uint(std::string_view text, i64* out, int base) noexcept;

}  // namespace pkpy
