#include "pocketpy/common/sstream.h"
#include "pocketpy/common/vector.h"
#include "pocketpy/compiler/lexer.h"
#include "pocketpy/objects/sourcedata.h"
#include <ctype.h>

#define is_raw_string_used(t) ((t) == TK_ID)

typedef struct Lexer {
    SourceData_ src;
    const char* token_start;
    const char* curr_char;
    int current_line;
    int brackets_level;

    c11_vector /*T=Token*/ nexts;
    c11_vector /*T=int*/ indents;
} Lexer;

const static TokenValue EmptyTokenValue;

static Error* lex_one_token(Lexer* self, bool* eof, bool is_fstring);

static void Lexer__ctor(Lexer* self, SourceData_ src) {
    PK_INCREF(src);
    self->src = src;
    self->curr_char = self->token_start = src->source->data;
    self->current_line = 1;
    self->brackets_level = 0;
    c11_vector__ctor(&self->nexts, sizeof(Token));
    c11_vector__ctor(&self->indents, sizeof(int));
}

static void Lexer__dtor(Lexer* self) {
    PK_DECREF(self->src);
    c11_vector__dtor(&self->nexts);
    c11_vector__dtor(&self->indents);
}

static char eatchar(Lexer* self) {
    char c = *self->curr_char;
    assert(c != '\n');  // eatchar() cannot consume a newline
    self->curr_char++;
    return c;
}

static char eatchar_include_newline(Lexer* self) {
    char c = *self->curr_char;
    self->curr_char++;
    if(c == '\n') {
        self->current_line++;
        c11_vector__push(const char*, &self->src->line_starts, self->curr_char);
    }
    return c;
}

static int eat_spaces(Lexer* self) {
    int count = 0;
    while(true) {
        switch(*self->curr_char) {
            case ' ': count += 1; break;
            case '\t': count += 4; break;
            default: return count;
        }
        eatchar(self);
    }
}

static bool matchchar(Lexer* self, char c) {
    if(*self->curr_char != c) return false;
    eatchar_include_newline(self);
    return true;
}

static bool match_n_chars(Lexer* self, int n, char c0) {
    const char* c = self->curr_char;
    for(int i = 0; i < n; i++) {
        if(*c == '\0') return false;
        if(*c != c0) return false;
        c++;
    }
    for(int i = 0; i < n; i++)
        eatchar_include_newline(self);
    return true;
}

static void skip_line_comment(Lexer* self) {
    while(*self->curr_char) {
        if(*self->curr_char == '\n') return;
        eatchar(self);
    }
}

static void add_token_with_value(Lexer* self, TokenIndex type, TokenValue value) {
    switch(type) {
        case TK_LBRACE:
        case TK_LBRACKET:
        case TK_LPAREN: self->brackets_level++; break;
        case TK_RPAREN:
        case TK_RBRACKET:
        case TK_RBRACE: self->brackets_level--; break;
        default: break;
    }
    Token token = {type,
                   self->token_start,
                   (int)(self->curr_char - self->token_start),
                   self->current_line - ((type == TK_EOL) ? 1 : 0),
                   self->brackets_level,
                   value};
    // handle "not in", "is not", "yield from"
    if(self->nexts.length > 0) {
        Token* back = &c11_vector__back(Token, &self->nexts);
        if(back->type == TK_NOT_KW && type == TK_IN) {
            back->type = TK_NOT_IN;
            return;
        }
        if(back->type == TK_IS && type == TK_NOT_KW) {
            back->type = TK_IS_NOT;
            return;
        }
        if(back->type == TK_YIELD && type == TK_FROM) {
            back->type = TK_YIELD_FROM;
            return;
        }
        c11_vector__push(Token, &self->nexts, token);
    }
}

static void add_token(Lexer* self, TokenIndex type) {
    add_token_with_value(self, type, EmptyTokenValue);
}

static void add_token_2(Lexer* self, char c, TokenIndex one, TokenIndex two) {
    if(matchchar(self, c))
        add_token(self, two);
    else
        add_token(self, one);
}

static bool eat_indentation(Lexer* self) {
    if(self->brackets_level > 0) return true;
    int spaces = eat_spaces(self);
    if(*self->curr_char == '#') skip_line_comment(self);
    if(*self->curr_char == '\0' || *self->curr_char == '\n') { return true; }
    // https://docs.python.org/3/reference/lexical_analysis.html#indentation
    int indents_back = c11_vector__back(int, &self->indents);
    if(spaces > indents_back) {
        c11_vector__push(int, &self->indents, spaces);
        Token t = {TK_INDENT,
                   self->token_start,
                   0,
                   self->current_line,
                   self->brackets_level,
                   EmptyTokenValue};
        c11_vector__push(Token, &self->nexts, t);
    } else if(spaces < indents_back) {
        do {
            c11_vector__pop(&self->indents);
            Token t = {TK_DEDENT,
                       self->token_start,
                       0,
                       self->current_line,
                       self->brackets_level,
                       EmptyTokenValue};
            c11_vector__push(Token, &self->nexts, t);
            indents_back = c11_vector__back(int, &self->indents);
        } while(spaces < indents_back);
        if(spaces != indents_back) { return false; }
    }
    return true;
}

static bool is_possible_number_char(char c) {
    switch(c) {
            // clang-format off
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
        case '.': case 'x': case 'o': case 'j':
        return true;
        default: return false;
            // clang-format on
    }
}

/******************************/
static Error* LexerError(Lexer* self, const char* fmt, ...) {
    Error* err = PK_MALLOC(sizeof(Error));
    err->src = self->src;
    PK_INCREF(self->src);
    err->lineno = self->current_line;
    if(*self->curr_char == '\n') { err->lineno--; }
    va_list args;
    va_start(args, fmt);
    vsnprintf(err->msg, sizeof(err->msg), fmt, args);
    va_end(args);
    return err;
}

static Error* eat_name(Lexer* self) {
    self->curr_char--;
    while(true) {
        unsigned char c = *self->curr_char;
        int u8bytes = c11__u8_header(c, true);
        if(u8bytes == 0) return LexerError(self, "invalid char: %c", c);
        if(u8bytes == 1) {
            if(isalnum(c) || c == '_') {
                self->curr_char++;
                continue;
            } else {
                break;
            }
        }
        int value = c11__u8_value(u8bytes, self->curr_char);
        if(c11__is_unicode_Lo_char(value)) {
            self->curr_char += u8bytes;
        } else {
            break;
        }
    }

    int length = (int)(self->curr_char - self->token_start);
    if(length == 0) return LexerError(self, "@id contains invalid char");
    c11_sv name = {self->token_start, length};

    const char** KW_BEGIN = TokenSymbols + TK_FALSE;
    int KW_COUNT = TK__COUNT__ - TK_FALSE;
#define less(a, b) (c11_sv__cmp2(b, a) > 0)
    int out;
    c11__lower_bound(const char*, KW_BEGIN, KW_COUNT, name, less, &out);
#undef less

    if(out != KW_COUNT && c11__sveq2(name, KW_BEGIN[out])) {
        add_token(self, (TokenIndex)(out + TK_FALSE));
    } else {
        add_token(self, TK_ID);
    }
    return NULL;
}

enum StringType { NORMAL_STRING, RAW_STRING, F_STRING, NORMAL_BYTES };

static Error* _eat_string(Lexer* self, c11_sbuf* buff, char quote, enum StringType type) {
    bool is_raw = type == RAW_STRING;
    bool is_fstring = type == F_STRING;

    if(is_fstring) { add_token(self, TK_FSTR_BEGIN); }

    // previous char is quote
    bool quote3 = match_n_chars(self, 2, quote);
    while(true) {
        char c = eatchar_include_newline(self);
        if(c == quote) {
            if(quote3 && !match_n_chars(self, 2, quote)) {
                c11_sbuf__write_char(buff, c);
                continue;
            }
            // end of string
            break;
        }
        if(c == '\0') { return LexerError(self, "EOL while scanning string literal"); }
        if(c == '\n') {
            if(!quote3)
                return LexerError(self, "EOL while scanning string literal");
            else {
                c11_sbuf__write_char(buff, c);
                continue;
            }
        }
        if(!is_raw && c == '\\') {
            switch(eatchar_include_newline(self)) {
                // For the list of available escape sequences see
                // https://docs.python.org/3/reference/lexical_analysis.html#escape-sequences
                case '"': c11_sbuf__write_char(buff, '"'); break;
                case '\'': c11_sbuf__write_char(buff, '\''); break;
                case '\\': c11_sbuf__write_char(buff, '\\'); break;
                case 'n': c11_sbuf__write_char(buff, '\n'); break;
                case 'r': c11_sbuf__write_char(buff, '\r'); break;
                case 't': c11_sbuf__write_char(buff, '\t'); break;
                case 'a': c11_sbuf__write_char(buff, '\a'); break;
                case 'b': c11_sbuf__write_char(buff, '\b'); break;
                case 'f': c11_sbuf__write_char(buff, '\f'); break;
                case 'v': c11_sbuf__write_char(buff, '\v'); break;
                // Special case for the often used \0 while we don't have full support for octal literals.
                case '0': c11_sbuf__write_char(buff, '\0'); break;
                case 'x': {
                    char hex[3] = {eatchar(self), eatchar(self), '\0'};
                    int code;
                    if(sscanf(hex, "%x", &code) != 1 || code > 0xFF) {
                        return LexerError(self, "invalid hex escape");
                    }
                    if(type == NORMAL_BYTES) {
                        // Bytes literals: write raw byte
                        c11_sbuf__write_char(buff, (char)code);
                    } else {
                        // Regular strings: encode as UTF-8
                        if(code <= 0x7F) {
                            c11_sbuf__write_char(buff, (char)code);
                        } else {
                            // Encode as 2-byte UTF-8 for code points 0x80-0xFF
                            c11_sbuf__write_char(buff, 0xC0 | (code >> 6));    // Leading byte
                            c11_sbuf__write_char(buff, 0x80 | (code & 0x3F));  // Continuation byte
                        }
                    }
                } break;
                default: return LexerError(self, "invalid escape char");
            }
        } else {
            if(is_fstring) {
                if(c == '{') {
                    if(matchchar(self, '{')) {
                        // '{{' -> '{'
                        c11_sbuf__write_char(buff, '{');
                    } else {
                        // submit previous string
                        c11_string* res = c11_sbuf__submit(buff);
                        if(res->size > 0) {
                            TokenValue value = {TokenValue_STR, ._str = res};
                            add_token_with_value(self, TK_FSTR_CPNT, value);
                        } else {
                            c11_string__delete(res);
                        }
                        c11_sbuf__ctor(buff);  // re-init buffer

                        // submit {expr} tokens
                        bool eof = false;
                        int token_count = self->nexts.length;
                        while(!eof) {
                            Error* err = lex_one_token(self, &eof, true);
                            if(err) return err;
                        }
                        if(self->nexts.length == token_count) {
                            // f'{}' is not allowed
                            return LexerError(self, "f-string: empty expression not allowed");
                        }
                    }
                } else if(c == '}') {
                    if(matchchar(self, '}')) {
                        // '}}' -> '}'
                        c11_sbuf__write_char(buff, '}');
                    } else {
                        return LexerError(self, "f-string: single '}' is not allowed");
                    }
                } else {
                    c11_sbuf__write_char(buff, c);
                }
            } else {
                c11_sbuf__write_char(buff, c);
            }
        }
    }

    c11_string* res = c11_sbuf__submit(buff);
    TokenValue value = {TokenValue_STR, ._str = res};

    if(is_fstring) {
        if(res->size > 0) {
            add_token_with_value(self, TK_FSTR_CPNT, value);
        } else {
            c11_string__delete(res);
        }
        add_token(self, TK_FSTR_END);
        return NULL;
    }

    if(type == NORMAL_BYTES) {
        add_token_with_value(self, TK_BYTES, value);
    } else {
        add_token_with_value(self, TK_STR, value);
    }
    return NULL;
}

static Error* eat_string(Lexer* self, char quote, enum StringType type) {
    c11_sbuf buff;
    c11_sbuf__ctor(&buff);
    Error* err = _eat_string(self, &buff, quote, type);
    c11_sbuf__dtor(&buff);
    return err;
}

static Error* eat_number(Lexer* self) {
    const char* i = self->token_start;
    while(is_possible_number_char(*i))
        i++;

    bool is_scientific_notation = false;
    if(*(i - 1) == 'e' && (*i == '+' || *i == '-')) {
        i++;
        while(isdigit(*i) || *i == 'j')
            i++;
        is_scientific_notation = true;
    }

    c11_sv text = {self->token_start, i - self->token_start};
    self->curr_char = i;

    if(text.data[0] != '.' && !is_scientific_notation) {
        // try integer
        TokenValue value = {.index = TokenValue_I64};
        switch(c11__parse_uint(text, &value._i64, -1)) {
            case IntParsing_SUCCESS: add_token_with_value(self, TK_NUM, value); return NULL;
            case IntParsing_OVERFLOW: return LexerError(self, "int literal is too large");
            case IntParsing_FAILURE: break;  // do nothing
        }
    }

    // try float
    double float_out;
    char* p_end;
    float_out = strtod(text.data, &p_end);

    if(p_end == text.data + text.size) {
        TokenValue value = {.index = TokenValue_F64, ._f64 = float_out};
        add_token_with_value(self, TK_NUM, value);
        return NULL;
    }

    if(i[-1] == 'j' && p_end == text.data + text.size - 1) {
        TokenValue value = {.index = TokenValue_F64, ._f64 = float_out};
        add_token_with_value(self, TK_IMAG, value);
        return NULL;
    }

    return LexerError(self, "invalid number literal");
}

static Error* eat_fstring_spec(Lexer* self, bool* eof) {
    while(true) {
        char c = eatchar_include_newline(self);
        if(c == '\n' || c == '\0') {
            return LexerError(self, "EOL while scanning f-string format spec");
        }
        if(c == '}') {
            add_token(self, TK_FSTR_SPEC);
            *eof = true;
            break;
        }
    }
    return NULL;
}

static Error* lex_one_token(Lexer* self, bool* eof, bool is_fstring) {
    *eof = false;
    while(*self->curr_char) {
        self->token_start = self->curr_char;
        char c = eatchar_include_newline(self);
        switch(c) {
            case '\'':
            case '"': {
                Error* err = eat_string(self, c, NORMAL_STRING);
                if(err) return err;
                return NULL;
            }
            case '#': skip_line_comment(self); break;
            case '~': add_token(self, TK_INVERT); return NULL;
            case '{': add_token(self, TK_LBRACE); return NULL;
            case '}': {
                if(is_fstring) {
                    *eof = true;
                    return NULL;
                }
                add_token(self, TK_RBRACE);
                return NULL;
            }
            case ',': add_token(self, TK_COMMA); return NULL;
            case ':': {
                if(is_fstring) {
                    // BUG: f"{stack[2:]}"
                    return eat_fstring_spec(self, eof);
                }
                add_token(self, TK_COLON);
                return NULL;
            }
            case ';': add_token(self, TK_SEMICOLON); return NULL;
            case '(': add_token(self, TK_LPAREN); return NULL;
            case ')': add_token(self, TK_RPAREN); return NULL;
            case '[': add_token(self, TK_LBRACKET); return NULL;
            case ']': add_token(self, TK_RBRACKET); return NULL;
            case '@': add_token(self, TK_DECORATOR); return NULL;
            case '\\': {
                // line continuation character
                char c = eatchar_include_newline(self);
                if(c != '\n') {
                    return LexerError(self, "expected newline after line continuation character");
                }
                eat_spaces(self);
                return NULL;
            }
            case '%': add_token_2(self, '=', TK_MOD, TK_IMOD); return NULL;
            case '&': add_token_2(self, '=', TK_AND, TK_IAND); return NULL;
            case '|': add_token_2(self, '=', TK_OR, TK_IOR); return NULL;
            case '^': add_token_2(self, '=', TK_XOR, TK_IXOR); return NULL;
            case '.': {
                if(matchchar(self, '.')) {
                    if(matchchar(self, '.')) {
                        add_token(self, TK_DOTDOTDOT);
                    } else {
                        add_token(self, TK_DOTDOT);
                    }
                } else {
                    char next_char = *self->curr_char;
                    if(next_char >= '0' && next_char <= '9') {
                        Error* err = eat_number(self);
                        if(err) return err;
                    } else {
                        add_token(self, TK_DOT);
                    }
                }
                return NULL;
            }
            case '=': add_token_2(self, '=', TK_ASSIGN, TK_EQ); return NULL;
            case '+': add_token_2(self, '=', TK_ADD, TK_IADD); return NULL;
            case '>': {
                if(matchchar(self, '='))
                    add_token(self, TK_GE);
                else if(matchchar(self, '>'))
                    add_token_2(self, '=', TK_RSHIFT, TK_IRSHIFT);
                else
                    add_token(self, TK_GT);
                return NULL;
            }
            case '<': {
                if(matchchar(self, '='))
                    add_token(self, TK_LE);
                else if(matchchar(self, '<'))
                    add_token_2(self, '=', TK_LSHIFT, TK_ILSHIFT);
                else
                    add_token(self, TK_LT);
                return NULL;
            }
            case '-': {
                if(matchchar(self, '='))
                    add_token(self, TK_ISUB);
                else if(matchchar(self, '>'))
                    add_token(self, TK_ARROW);
                else
                    add_token(self, TK_SUB);
                return NULL;
            }
            case '!':
                if(is_fstring) {
                    if(matchchar(self, 'r')) { return eat_fstring_spec(self, eof); }
                }
                if(matchchar(self, '=')) {
                    add_token(self, TK_NE);
                    return NULL;
                } else {
                    return LexerError(self, "expected '=' after '!'");
                }
            case '*':
                if(matchchar(self, '*')) {
                    add_token(self, TK_POW);  // '**'
                } else {
                    add_token_2(self, '=', TK_MUL, TK_IMUL);
                }
                return NULL;
            case '/':
                if(matchchar(self, '/')) {
                    add_token_2(self, '=', TK_FLOORDIV, TK_IFLOORDIV);
                } else {
                    add_token_2(self, '=', TK_DIV, TK_IDIV);
                }
                return NULL;
            case ' ':
            case '\t': eat_spaces(self); break;
            case '\n': {
                add_token(self, TK_EOL);
                if(!eat_indentation(self)) {
                    return LexerError(self, "unindent does not match any outer indentation level");
                }
                return NULL;
            }
            default: {
                if(c == 'f') {
                    if(matchchar(self, '\'')) return eat_string(self, '\'', F_STRING);
                    if(matchchar(self, '"')) return eat_string(self, '"', F_STRING);
                } else if(c == 'r') {
                    if(matchchar(self, '\'')) return eat_string(self, '\'', RAW_STRING);
                    if(matchchar(self, '"')) return eat_string(self, '"', RAW_STRING);
                } else if(c == 'b') {
                    if(matchchar(self, '\'')) return eat_string(self, '\'', NORMAL_BYTES);
                    if(matchchar(self, '"')) return eat_string(self, '"', NORMAL_BYTES);
                }
                if(c >= '0' && c <= '9') return eat_number(self);
                return eat_name(self);
            }
        }
    }

    if(is_fstring) return LexerError(self, "unterminated f-string expression");

    self->token_start = self->curr_char;
    while(self->indents.length > 1) {
        c11_vector__pop(&self->indents);
        add_token(self, TK_DEDENT);
        return NULL;
    }
    add_token(self, TK_EOF);
    *eof = true;
    return NULL;
}

Error* Lexer__process(SourceData_ src, Token** out_tokens, int* out_length) {
    Lexer lexer;
    Lexer__ctor(&lexer, src);

    // push initial tokens
    Token sof =
        {TK_SOF, lexer.token_start, 0, lexer.current_line, lexer.brackets_level, EmptyTokenValue};
    c11_vector__push(Token, &lexer.nexts, sof);
    c11_vector__push(int, &lexer.indents, 0);

    bool eof = false;
    while(!eof) {
        void* err = lex_one_token(&lexer, &eof, false);
        if(err) {
            Lexer__dtor(&lexer);
            return err;
        }
    }
    // set out_tokens
    *out_tokens = c11_vector__submit(&lexer.nexts, out_length);

    Lexer__dtor(&lexer);
    return NULL;
}

const char* TokenSymbols[] = {
    "@eof",
    "@eol",
    "@sof",
    "@id",
    "@num",
    "@str",
    "@fstr-begin",  // TK_FSTR_BEGIN
    "@fstr-cpnt",   // TK_FSTR_CPNT
    "@fstr-spec",   // TK_FSTR_SPEC
    "@fstr-end",    // TK_FSTR_END
    "@bytes",
    "@imag",
    "@indent",
    "@dedent",
    // These 3 are compound keywords which are generated on the fly
    "is not",
    "not in",
    "yield from",
    /*****************************************/
    "+",
    "+=",
    "-",
    "-=",  // (INPLACE_OP - 1) can get '=' removed
    "*",
    "*=",
    "/",
    "/=",
    "//",
    "//=",
    "%",
    "%=",
    "&",
    "&=",
    "|",
    "|=",
    "^",
    "^=",
    "<<",
    "<<=",
    ">>",
    ">>=",
    /*****************************************/
    "(",
    ")",
    "[",
    "]",
    "{",
    "}",
    ".",
    "..",
    "...",
    ",",
    ":",
    ";",
    "**",
    "->",
    "#",
    "@",
    ">",
    "<",
    "=",
    "==",
    "!=",
    ">=",
    "<=",
    "~",
    /** KW_BEGIN **/
    // NOTE: These keywords should be sorted in ascending order!!
    "False",
    "None",
    "True",
    "and",
    "as",
    "assert",
    "break",
    "class",
    "continue",
    "def",
    "del",
    "elif",
    "else",
    "except",
    "finally",
    "for",
    "from",
    "global",
    "if",
    "import",
    "in",
    "is",
    "lambda",
    "match",
    "not",
    "or",
    "pass",
    "raise",
    "return",
    "try",
    "while",
    "with",
    "yield",
};

#undef is_raw_string_used