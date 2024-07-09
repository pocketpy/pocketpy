#include "pocketpy/common/smallmap.h"
#include "pocketpy/common/config.h"
#include "pocketpy/common/sstream.h"
#include "pocketpy/common/vector.h"
#include "pocketpy/compiler/lexer.h"
#include "pocketpy/objects/sourcedata.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>

#define is_raw_string_used(t) ((t) == TK_ID || (t) == TK_LONG)

typedef struct pk_Lexer{
    pk_SourceData_ src;
    const char* token_start;
    const char* curr_char;
    int current_line;
    int brackets_level;

    c11_vector/*T=Token*/ nexts;
    c11_vector/*T=int*/ indents;
} pk_Lexer;

typedef struct TokenDeserializer {
    const char* curr;
    const char* source;
} TokenDeserializer;

void TokenDeserializer__ctor(TokenDeserializer* self, const char* source);
bool TokenDeserializer__match_char(TokenDeserializer* self, char c);
c11_sv TokenDeserializer__read_string(TokenDeserializer* self, char c);
c11_string* TokenDeserializer__read_string_from_hex(TokenDeserializer* self, char c);
int TokenDeserializer__read_count(TokenDeserializer* self);
int64_t TokenDeserializer__read_uint(TokenDeserializer* self, char c);
double TokenDeserializer__read_float(TokenDeserializer* self, char c);


const static TokenValue EmptyTokenValue;

static void pk_Lexer__ctor(pk_Lexer* self, pk_SourceData_ src){
    PK_INCREF(src);
    self->src = src;
    self->curr_char = self->token_start = src->source->data;
    self->current_line = 1;
    self->brackets_level = 0;
    c11_vector__ctor(&self->nexts, sizeof(Token));
    c11_vector__ctor(&self->indents, sizeof(int));
}

static void pk_Lexer__dtor(pk_Lexer* self){
    PK_DECREF(self->src);
    c11_vector__dtor(&self->nexts);
    c11_vector__dtor(&self->indents);
}

static char eatchar(pk_Lexer* self){
    char c = *self->curr_char;
    assert(c != '\n');  // eatchar() cannot consume a newline
    self->curr_char++;
    return c;
}

static char eatchar_include_newline(pk_Lexer* self){
    char c = *self->curr_char;
    self->curr_char++;
    if(c == '\n') {
        self->current_line++;
        c11_vector__push(const char*, &self->src->line_starts, self->curr_char);
    }
    return c;
}

static int eat_spaces(pk_Lexer* self){
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

static bool matchchar(pk_Lexer* self, char c){
    if(*self->curr_char != c) return false;
    eatchar_include_newline(self);
    return true;
}

static bool match_n_chars(pk_Lexer* self, int n, char c0){
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

static void skip_line_comment(pk_Lexer* self){
    while(*self->curr_char) {
        if(*self->curr_char == '\n') return;
        eatchar(self);
    }
}

static void add_token_with_value(pk_Lexer* self, TokenIndex type, TokenValue value){
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
    if(self->nexts.count > 0) {
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

static void add_token(pk_Lexer* self, TokenIndex type){
    add_token_with_value(self, type, EmptyTokenValue);
}

static void add_token_2(pk_Lexer* self, char c, TokenIndex one, TokenIndex two){
    if(matchchar(self, c))
        add_token(self, two);
    else
        add_token(self, one);
}

static bool eat_indentation(pk_Lexer* self){
    if(self->brackets_level > 0) return true;
    int spaces = eat_spaces(self);
    if(*self->curr_char == '#') skip_line_comment(self);
    if(*self->curr_char == '\0' || *self->curr_char == '\n'){
        return true;
    }
    // https://docs.python.org/3/reference/lexical_analysis.html#indentation
    int indents_back = c11_vector__back(int, &self->indents);
    if(spaces > indents_back) {
        c11_vector__push(int, &self->indents, spaces);
        Token t = {TK_INDENT, self->token_start, 0, self->current_line, self->brackets_level, EmptyTokenValue};
        c11_vector__push(Token, &self->nexts, t);
    } else if(spaces < indents_back) {
        do {
            c11_vector__pop(&self->indents);
            Token t = {TK_DEDENT, self->token_start, 0, self->current_line, self->brackets_level, EmptyTokenValue};
            c11_vector__push(Token, &self->nexts, t);
            indents_back = c11_vector__back(int, &self->indents);
        } while(spaces < indents_back);
        if(spaces != indents_back) { return false; }
    }
    return true;
}

static bool is_possible_number_char(char c){
    switch(c) {
        // clang-format off
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
        case '.': case 'L': case 'x': case 'o': case 'j':
        return true;
        default: return false;
        // clang-format on
    }
}

/******************************/
static Error* SyntaxError(const char* fmt, ...){
    // va_list args;
    // va_start(args, fmt);
    // Error* err = _error(true, "SyntaxError", fmt, &args);
    // va_end(args);
    // return err;
    return NULL;
}

static Error* eat_name(pk_Lexer* self){
    self->curr_char--;
    while(true) {
        unsigned char c = *self->curr_char;
        int u8bytes = c11__u8_header(c, true);
        if(u8bytes == 0) return SyntaxError("invalid char: %c", c);
        if(u8bytes == 1) {
            if(isalnum(c) || c == '_') {
                self->curr_char++;
                continue;
            } else {
                break;
            }
        }
        // handle multibyte char
        uint32_t value = 0;
        for(int k = 0; k < u8bytes; k++) {
            uint8_t b = self->curr_char[k];
            if(k == 0) {
                if(u8bytes == 2)
                    value = (b & 0b00011111) << 6;
                else if(u8bytes == 3)
                    value = (b & 0b00001111) << 12;
                else if(u8bytes == 4)
                    value = (b & 0b00000111) << 18;
            } else {
                value |= (b & 0b00111111) << (6 * (u8bytes - k - 1));
            }
        }
        if(c11__is_unicode_Lo_char(value)){
            self->curr_char += u8bytes;
        }else{
            break;
        }
    }

    int length = (int)(self->curr_char - self->token_start);
    if(length == 0) return SyntaxError("@id contains invalid char");
    c11_sv name = {self->token_start, length};

    const char** KW_BEGIN = pk_TokenSymbols + TK_FALSE;
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

static Error* eat_string_until(pk_Lexer* self, char quote, bool raw, c11_string** out) {
    // previous char is quote
    bool quote3 = match_n_chars(self, 2, quote);
    c11_sbuf buff;
    c11_sbuf__ctor(&buff);
    while(true) {
        char c = eatchar_include_newline(self);
        if(c == quote) {
            if(quote3 && !match_n_chars(self, 2, quote)) {
                c11_sbuf__write_char(&buff, c);
                continue;
            }
            break;
        }
        if(c == '\0') {
            return SyntaxError("EOL while scanning string literal");
        }
        if(c == '\n') {
            if(!quote3)
                return SyntaxError("EOL while scanning string literal");
            else {
                c11_sbuf__write_char(&buff, c);
                continue;
            }
        }
        if(!raw && c == '\\') {
            switch(eatchar_include_newline(self)) {
                case '"': c11_sbuf__write_char(&buff, '"'); break;
                case '\'': c11_sbuf__write_char(&buff, '\''); break;
                case '\\': c11_sbuf__write_char(&buff, '\\'); break;
                case 'n': c11_sbuf__write_char(&buff, '\n'); break;
                case 'r': c11_sbuf__write_char(&buff, '\r'); break;
                case 't': c11_sbuf__write_char(&buff, '\t'); break;
                case 'b': c11_sbuf__write_char(&buff, '\b'); break;
                case 'x': {
                    char hex[3] = {eatchar(self), eatchar(self), '\0'};
                    int code;
                    if(sscanf(hex, "%x", &code) != 1) {
                        return SyntaxError("invalid hex char");
                    }
                    c11_sbuf__write_char(&buff, (char)code);
                } break;
                default: return SyntaxError("invalid escape char");
            }
        } else {
            c11_sbuf__write_char(&buff, c);
        }
    }
    *out = c11_sbuf__submit(&buff);
    return NULL;
}

enum StringType {
    NORMAL_STRING,
    RAW_STRING,
    F_STRING,
    NORMAL_BYTES
};

static Error* eat_string(pk_Lexer* self, char quote, enum StringType type){
    c11_string* s;
    Error* err = eat_string_until(self, quote, type == RAW_STRING, &s);
    if(err) return err;
    TokenValue value = {TokenValue_STR, ._str = s};
    if(type == F_STRING) {
        add_token_with_value(self, TK_FSTR, value);
    }else if(type == NORMAL_BYTES) {
        add_token_with_value(self, TK_BYTES, value);
    }else{
        add_token_with_value(self, TK_STR, value);
    }
    return NULL;
}

static Error* eat_number(pk_Lexer* self){
    const char* i = self->token_start;
    while(is_possible_number_char(*i)) i++;

    bool is_scientific_notation = false;
    if(*(i - 1) == 'e' && (*i == '+' || *i == '-')) {
        i++;
        while(isdigit(*i) || *i == 'j') i++;
        is_scientific_notation = true;
    }

    c11_sv text = {self->token_start, i - self->token_start};
    self->curr_char = i;

    if(text.data[0] != '.' && !is_scientific_notation) {
        // try long
        if(i[-1] == 'L') {
            add_token(self, TK_LONG);
            return NULL;
        }
        // try integer
        TokenValue value = {.index = TokenValue_I64};
        switch(c11__parse_uint(text, &value._i64, -1)) {
            case IntParsing_SUCCESS:
                add_token_with_value(self, TK_NUM, value);
                return NULL;
            case IntParsing_OVERFLOW:
                return SyntaxError("int literal is too large");
            case IntParsing_FAILURE:
                break;  // do nothing
        }
    }

    // try float
    double float_out;
    char* p_end;
    float_out = strtod(text.data, &p_end);

    if(p_end == text.data + text.size){
        TokenValue value = {.index = TokenValue_F64, ._f64 = float_out};
        add_token_with_value(self, TK_NUM, value);
        return NULL;
    }

    if(i[-1] == 'j' && p_end == text.data + text.size - 1) {
        TokenValue value = {.index = TokenValue_F64, ._f64 = float_out};
        add_token_with_value(self, TK_IMAG, value);
        return NULL;
    }

    return SyntaxError("invalid number literal");
}

static Error* lex_one_token(pk_Lexer* self, bool* eof){
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
            case '}': add_token(self, TK_RBRACE); return NULL;
            case ',': add_token(self, TK_COMMA); return NULL;
            case ':': add_token(self, TK_COLON); return NULL;
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
                    return SyntaxError("expected newline after line continuation character");
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
                if(matchchar(self, '=')){
                    add_token(self, TK_NE);
                }else{
                    Error* err = SyntaxError("expected '=' after '!'");
                    if(err) return err;
                }
                break;
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
                if(!eat_indentation(self)){
                    return SyntaxError("unindent does not match any outer indentation level");
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

    self->token_start = self->curr_char;
    while(self->indents.count > 1) {
        c11_vector__pop(&self->indents);
        add_token(self, TK_DEDENT);
        return NULL;
    }
    add_token(self, TK_EOF);
    *eof = true;
    return NULL;
}

static Error* from_precompiled(pk_Lexer* self) {
    TokenDeserializer deserializer;
    TokenDeserializer__ctor(&deserializer, self->src->source->data);

    deserializer.curr += 5;  // skip "pkpy:"
    c11_sv version = TokenDeserializer__read_string(&deserializer, '\n');

    if(c11_sv__cmp2(version, PK_VERSION) != 0) {
        return SyntaxError("precompiled version mismatch");
    }
    if(TokenDeserializer__read_uint(&deserializer, '\n') != (int64_t)self->src->mode){
        return SyntaxError("precompiled mode mismatch");
    }

    int count = TokenDeserializer__read_count(&deserializer);
    c11_vector* precompiled_tokens = &self->src->_precompiled_tokens;
    for(int i = 0; i < count; i++) {
        c11_sv item = TokenDeserializer__read_string(&deserializer, '\n');
        c11_string* copied_item = c11_string__new2(item.data, item.size);
        c11_vector__push(c11_string*, precompiled_tokens, copied_item);
    }

    count = TokenDeserializer__read_count(&deserializer);
    for(int i = 0; i < count; i++) {
        Token t;
        t.type = (TokenIndex)TokenDeserializer__read_uint(&deserializer, ',');
        if(is_raw_string_used(t.type)) {
            int64_t index = TokenDeserializer__read_uint(&deserializer, ',');
            c11_string* p = c11__getitem(c11_string*, precompiled_tokens, index);
            t.start = p->data;
            t.length = p->size;
        } else {
            t.start = NULL;
            t.length = 0;
        }

        if(TokenDeserializer__match_char(&deserializer, ',')) {
            t.line = c11_vector__back(Token, &self->nexts).line;
        } else {
            t.line = (int)TokenDeserializer__read_uint(&deserializer, ',');
        }

        if(TokenDeserializer__match_char(&deserializer, ',')) {
            t.brackets_level = c11_vector__back(Token, &self->nexts).brackets_level;
        } else {
            t.brackets_level = (int)TokenDeserializer__read_uint(&deserializer, ',');
        }

        char type = (*deserializer.curr++);      // read_char
        switch(type) {
            case 'I': {
                int64_t res = TokenDeserializer__read_uint(&deserializer, '\n');
                t.value = (TokenValue){TokenValue_I64, ._i64 = res};
            } break;
            case 'F': {
                double res = TokenDeserializer__read_float(&deserializer, '\n');
                t.value = (TokenValue){TokenValue_F64, ._f64 = res};
            } break;
            case 'S': {
                c11_string* res = TokenDeserializer__read_string_from_hex(&deserializer, '\n');
                t.value = (TokenValue){TokenValue_STR, ._str = res};
            } break;
            default:
                t.value = EmptyTokenValue;
                break;
        }
        c11_vector__push(Token, &self->nexts, t);
    }
    return NULL;
}

Error* pk_Lexer__process(pk_SourceData_ src, pk_TokenArray* out_tokens){
    pk_Lexer lexer;
    pk_Lexer__ctor(&lexer, src);

    if(src->is_precompiled) {
        Error* err = from_precompiled(&lexer);
        // TODO: set out tokens
        pk_Lexer__dtor(&lexer);
        return err;
    }
    // push initial tokens
    Token sof = {TK_SOF, lexer.token_start, 0, lexer.current_line, lexer.brackets_level, EmptyTokenValue};
    c11_vector__push(Token, &lexer.nexts, sof);
    c11_vector__push(int, &lexer.indents, 0);

    bool eof = false;
    while(!eof) {
        void* err = lex_one_token(&lexer, &eof);
        if(err){
            pk_Lexer__dtor(&lexer);
            return err;
        }
    }
    // set out_tokens
    *out_tokens = c11_vector__submit(&lexer.nexts);

    pk_Lexer__dtor(&lexer);
    return NULL;
}

Error* pk_Lexer__process_and_dump(pk_SourceData_ src, c11_string** out) {
    assert(!src->is_precompiled);
    pk_TokenArray nexts;    // output tokens
    Error* err = pk_Lexer__process(src, &nexts);
    if(err) return err;

    c11_sbuf ss;
    c11_sbuf__ctor(&ss);

    // L1: version string
    c11_sbuf__write_cstr(&ss, "pkpy:" PK_VERSION "\n");
    // L2: mode
    c11_sbuf__write_int(&ss, (int)src->mode);
    c11_sbuf__write_char(&ss, '\n');

    c11_smallmap_s2n token_indices;
    c11_smallmap_s2n__ctor(&token_indices);

    c11__foreach(Token, &nexts, token) {
        if(is_raw_string_used(token->type)) {
            c11_sv token_sv = {token->start, token->length};
            if(!c11_smallmap_s2n__contains(&token_indices, token_sv)) {
                c11_smallmap_s2n__set(&token_indices, token_sv, 0);
            }
        }
    }
    // L3: raw string count
    c11_sbuf__write_char(&ss, '=');
    c11_sbuf__write_int(&ss, token_indices.count);
    c11_sbuf__write_char(&ss, '\n');

    uint16_t index = 0;
    for(int i=0; i<token_indices.count; i++){
        c11_smallmap_s2n_KV* kv = c11__at(c11_smallmap_s2n_KV, &token_indices, i);
        // L4: raw strings
        c11_sbuf__write_cstrn(&ss, kv->key.data, kv->key.size);
        kv->value = index++;
    }

    // L5: token count
    c11_sbuf__write_char(&ss, '=');
    c11_sbuf__write_int(&ss, nexts.count);
    c11_sbuf__write_char(&ss, '\n');

    for(int i = 0; i < nexts.count; i++) {
        const Token* token = c11__at(Token, &nexts, i);
        c11_sbuf__write_int(&ss, (int)token->type);
        c11_sbuf__write_char(&ss, ',');

        if(is_raw_string_used(token->type)) {
            uint16_t *p = c11_smallmap_s2n__try_get(
                &token_indices, (c11_sv){token->start, token->length});
            assert(p != NULL);
            c11_sbuf__write_int(&ss, (int)*p);
            c11_sbuf__write_char(&ss, ',');
        }
        if(i > 0 && c11__getitem(Token, &nexts, i-1).line == token->line){
            c11_sbuf__write_char(&ss, ',');
        }else{
            c11_sbuf__write_int(&ss, token->line);
            c11_sbuf__write_char(&ss, ',');
        }
            
        if(i > 0 && c11__getitem(Token, &nexts, i-1).brackets_level == token->brackets_level){
            c11_sbuf__write_char(&ss, ',');
        }else{
            c11_sbuf__write_int(&ss, token->brackets_level);
            c11_sbuf__write_char(&ss, ',');
        }
        // visit token value
        switch(token->value.index){
            case TokenValue_EMPTY: break;
            case TokenValue_I64:
                c11_sbuf__write_char(&ss, 'I');
                c11_sbuf__write_int(&ss, token->value._i64);
                break;
            case TokenValue_F64:
                c11_sbuf__write_char(&ss, 'F');
                c11_sbuf__write_f64(&ss, token->value._f64, -1);
                break;
            case TokenValue_STR: {
                c11_sbuf__write_char(&ss, 'S');
                c11_sv sv = c11_string__sv(token->value._str);
                for(int i=0; i<sv.size; i++){
                    c11_sbuf__write_hex(&ss, sv.data[i], false);
                }
                break;
            }
        }
        c11_sbuf__write_char(&ss, '\n');
    }
    *out = c11_sbuf__submit(&ss);
    c11_smallmap_s2n__dtor(&token_indices);
    return NULL;
}

void pk_TokenArray__dtor(pk_TokenArray *self){
    Token* data = self->data;
    for(int i=0; i<self->count; i++){
        if(data[i].value.index == TokenValue_STR){
            c11_string__delete(data[i].value._str);
        }
    }
    c11_array__dtor(self);
}

const char* pk_TokenSymbols[] = {
    "@eof", "@eol", "@sof",
    "@id", "@num", "@str", "@fstr", "@long", "@bytes", "@imag",
    "@indent", "@dedent",
    // These 3 are compound keywords which are generated on the fly
    "is not", "not in", "yield from",
    /*****************************************/
    "+", "+=", "-", "-=",   // (INPLACE_OP - 1) can get '=' removed
    "*", "*=", "/", "/=", "//", "//=", "%", "%=",
    "&", "&=", "|", "|=", "^", "^=", 
    "<<", "<<=", ">>", ">>=",
    /*****************************************/
    "(", ")", "[", "]", "{", "}",
    ".", "..", "...", ",", ":", ";",
    "**", "->", "#", "@",
    ">", "<", "=", "==", "!=", ">=", "<=", "~",
    /** KW_BEGIN **/
    // NOTE: These keywords should be sorted in ascending order!!
    "False", "None", "True", "and", "as", "assert", "break", "class", "continue",
    "def", "del", "elif", "else", "except", "finally", "for", "from", "global",
    "if", "import", "in", "is", "lambda", "not", "or", "pass", "raise", "return",
    "try", "while", "with", "yield",
};

void TokenDeserializer__ctor(TokenDeserializer* self, const char* source){
    self->curr = source;
    self->source = source;
}

bool TokenDeserializer__match_char(TokenDeserializer* self, char c){
    if(*self->curr == c) {
        self->curr++;
        return true;
    }
    return false;
}

c11_sv TokenDeserializer__read_string(TokenDeserializer* self, char c){
    const char* start = self->curr;
    while(*self->curr != c)
        self->curr++;
    c11_sv retval = {start, (int)(self->curr-start)};
    self->curr++;  // skip the delimiter
    return retval;
}

c11_string* TokenDeserializer__read_string_from_hex(TokenDeserializer* self, char c){
    c11_sv sv = TokenDeserializer__read_string(self, c);
    const char* s = sv.data;
    c11_sbuf ss;
    c11_sbuf__ctor(&ss);
    for(int i = 0; i < sv.size; i += 2) {
        char c = 0;
        if(s[i] >= '0' && s[i] <= '9')
            c += s[i] - '0';
        else if(s[i] >= 'a' && s[i] <= 'f')
            c += s[i] - 'a' + 10;
        else
            assert(false);
        c <<= 4;
        if(s[i + 1] >= '0' && s[i + 1] <= '9')
            c += s[i + 1] - '0';
        else if(s[i + 1] >= 'a' && s[i + 1] <= 'f')
            c += s[i + 1] - 'a' + 10;
        else
            assert(false);
        c11_sbuf__write_char(&ss, c);
    }
    return c11_sbuf__submit(&ss);
}

int TokenDeserializer__read_count(TokenDeserializer* self){
    assert(*self->curr == '=');
    self->curr++;
    return TokenDeserializer__read_uint(self, '\n');
}

int64_t TokenDeserializer__read_uint(TokenDeserializer* self, char c){
    int64_t out = 0;
    while(*self->curr != c) {
        out = out * 10 + (*self->curr - '0');
        self->curr++;
    }
    self->curr++;  // skip the delimiter
    return out;
}

double TokenDeserializer__read_float(TokenDeserializer* self, char c){
    c11_sv sv = TokenDeserializer__read_string(self, c);
    // TODO: optimize this
    c11_string* nullterm = c11_string__new2(sv.data, sv.size);
    char* end;
    double retval = strtod(nullterm->data, &end);
    c11_string__delete(nullterm);
    assert(*end == 0);
    return retval;
}
