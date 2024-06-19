#include "pocketpy/compiler/lexer.hpp"
#include "pocketpy/common/config.h"
#include "pocketpy/common/str.h"
#include "pocketpy/common/smallmap.h"
#include "pocketpy/compiler/lexer.h"

#include <cstdarg>

namespace pkpy {

static bool is_possible_number_char(char c) noexcept{
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

bool Lexer::match_n_chars(int n, char c0) noexcept{
    const char* c = curr_char;
    for(int i = 0; i < n; i++) {
        if(*c == '\0') return false;
        if(*c != c0) return false;
        c++;
    }
    for(int i = 0; i < n; i++)
        eatchar_include_newline();
    return true;
}

bool Lexer::match_string(const char* s) noexcept{
    int s_len = strlen(s);
    bool ok = strncmp(curr_char, s, s_len) == 0;
    if(ok)
        for(int i = 0; i < s_len; i++)
            eatchar_include_newline();
    return ok;
}

int Lexer::eat_spaces() noexcept{
    int count = 0;
    while(true) {
        switch(peekchar()) {
            case ' ': count += 1; break;
            case '\t': count += 4; break;
            default: return count;
        }
        eatchar();
    }
}

bool Lexer::eat_indentation() noexcept{
    if(brackets_level > 0) return true;
    int spaces = eat_spaces();
    if(peekchar() == '#') skip_line_comment();
    if(peekchar() == '\0' || peekchar() == '\n') return true;
    // https://docs.python.org/3/reference/lexical_analysis.html#indentation
    if(spaces > indents.back()) {
        indents.push_back(spaces);
        nexts.push_back(Token{TK("@indent"), token_start, 0, current_line, brackets_level, {}});
    } else if(spaces < indents.back()) {
        while(spaces < indents.back()) {
            indents.pop_back();
            nexts.push_back(Token{TK("@dedent"), token_start, 0, current_line, brackets_level, {}});
        }
        if(spaces != indents.back()) { return false; }
    }
    return true;
}

char Lexer::eatchar() noexcept{
    char c = peekchar();
    assert(c != '\n');  // eatchar() cannot consume a newline
    curr_char++;
    return c;
}

char Lexer::eatchar_include_newline() noexcept{
    char c = peekchar();
    curr_char++;
    if(c == '\n') {
        current_line++;
        c11_vector__push(const char*, &src->line_starts, curr_char);
    }
    return c;
}

Error* Lexer::eat_name() noexcept{
    curr_char--;
    while(true) {
        unsigned char c = peekchar();
        int u8bytes = c11__u8_header(c, true);
        if(u8bytes == 0) return SyntaxError("invalid char: %c", c);
        if(u8bytes == 1) {
            if(isalpha(c) || c == '_' || isdigit(c)) {
                curr_char++;
                continue;
            } else {
                break;
            }
        }
        // handle multibyte char
        Str u8str(curr_char, u8bytes);
        if(u8str.size != u8bytes) return SyntaxError("invalid utf8 sequence: %s", u8str.c_str());
        uint32_t value = 0;
        for(int k = 0; k < u8bytes; k++) {
            uint8_t b = u8str[k];
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
        if(c11__is_unicode_Lo_char(value))
            curr_char += u8bytes;
        else
            break;
    }

    int length = (int)(curr_char - token_start);
    if(length == 0) return SyntaxError("@id contains invalid char");
    std::string_view name(token_start, length);

    if(src->mode == JSON_MODE) {
        if(name == "true") {
            add_token(TK("True"));
        } else if(name == "false") {
            add_token(TK("False"));
        } else if(name == "null") {
            add_token(TK("None"));
        } else {
            return SyntaxError("invalid JSON token");
        }
        return NULL;
    }

    const auto KW_BEGIN = kTokens + TK("False");
    const auto KW_END = kTokens + kTokenCount;

    auto it = lower_bound(KW_BEGIN, KW_END, name);
    if(it != KW_END && *it == name) {
        add_token(it - kTokens);
    } else {
        add_token(TK("@id"));
    }
    return NULL;
}

void Lexer::skip_line_comment() noexcept{
    char c;
    while((c = peekchar()) != '\0') {
        if(c == '\n') return;
        eatchar();
    }
}

bool Lexer::matchchar(char c) noexcept{
    if(peekchar() != c) return false;
    eatchar_include_newline();
    return true;
}

void Lexer::add_token(TokenIndex type, TokenValue value) noexcept{
    switch(type) {
        case TK("{"):
        case TK("["):
        case TK("("): brackets_level++; break;
        case TK(")"):
        case TK("]"):
        case TK("}"): brackets_level--; break;
    }
    auto token = Token{type,
                       token_start,
                       (int)(curr_char - token_start),
                       current_line - ((type == TK("@eol")) ? 1 : 0),
                       brackets_level,
                       value};
    // handle "not in", "is not", "yield from"
    if(!nexts.empty()) {
        auto& back = nexts.back();
        if(back.type == TK("not") && type == TK("in")) {
            back.type = TK("not in");
            return;
        }
        if(back.type == TK("is") && type == TK("not")) {
            back.type = TK("is not");
            return;
        }
        if(back.type == TK("yield") && type == TK("from")) {
            back.type = TK("yield from");
            return;
        }
        nexts.push_back(token);
    }
}

void Lexer::add_token_2(char c, TokenIndex one, TokenIndex two) noexcept{
    if(matchchar(c))
        add_token(two);
    else
        add_token(one);
}

Error* Lexer::eat_string_until(char quote, bool raw, Str* out) noexcept{
    bool quote3 = match_n_chars(2, quote);
    small_vector_2<char, 32> buff;
    while(true) {
        char c = eatchar_include_newline();
        if(c == quote) {
            if(quote3 && !match_n_chars(2, quote)) {
                buff.push_back(c);
                continue;
            }
            break;
        }
        if(c == '\0') {
            if(quote3 && src->mode == REPL_MODE) return NeedMoreLines();
            return SyntaxError("EOL while scanning string literal");
        }
        if(c == '\n') {
            if(!quote3)
                return SyntaxError("EOL while scanning string literal");
            else {
                buff.push_back(c);
                continue;
            }
        }
        if(!raw && c == '\\') {
            switch(eatchar_include_newline()) {
                case '"': buff.push_back('"'); break;
                case '\'': buff.push_back('\''); break;
                case '\\': buff.push_back('\\'); break;
                case 'n': buff.push_back('\n'); break;
                case 'r': buff.push_back('\r'); break;
                case 't': buff.push_back('\t'); break;
                case 'b': buff.push_back('\b'); break;
                case 'x': {
                    char hex[3] = {eatchar(), eatchar(), '\0'};
                    size_t parsed;
                    char code;
                    try {
                        code = (char)std::stoi(hex, &parsed, 16);
                    } catch(...) {
                        return SyntaxError("invalid hex char");
                    }
                    if(parsed != 2) return SyntaxError("invalid hex char");
                    buff.push_back(code);
                } break;
                default: return SyntaxError("invalid escape char");
            }
        } else {
            buff.push_back(c);
        }
    }
    *out = Str(buff.data(), buff.size());
    return nullptr;
}

Error* Lexer::eat_string(char quote, StringType type) noexcept{
    Str s;
    Error* err = eat_string_until(quote, type == StringType::RAW_STRING, &s);
    if(err) return err;
    if(type == StringType::F_STRING) {
        add_token(TK("@fstr"), s);
    }else if(type == StringType::NORMAL_BYTES) {
        add_token(TK("@bytes"), s);
    }else{
        add_token(TK("@str"), s);
    }
    return NULL;
}

Error* Lexer::eat_number() noexcept{
    const char* i = token_start;
    while(is_possible_number_char(*i))
        i++;

    bool is_scientific_notation = false;
    if(*(i - 1) == 'e' && (*i == '+' || *i == '-')) {
        i++;
        while(isdigit(*i) || *i == 'j')
            i++;
        is_scientific_notation = true;
    }

    std::string_view text(token_start, i - token_start);
    this->curr_char = i;

    if(text[0] != '.' && !is_scientific_notation) {
        // try long
        if(i[-1] == 'L') {
            add_token(TK("@long"));
            return NULL;
        }
        // try integer
        i64 int_out;
        switch(parse_uint(text, &int_out, -1)) {
            case IntParsingResult::Success: add_token(TK("@num"), int_out); return NULL;
            case IntParsingResult::Overflow: return SyntaxError("int literal is too large");
            case IntParsingResult::Failure: break;  // do nothing
        }
    }

    // try float
    double float_out;
    char* p_end;
    try {
        float_out = std::strtod(text.data(), &p_end);
    } catch(...) {
        return SyntaxError("invalid number literal");
    }

    if(p_end == text.data() + text.size()) {
        add_token(TK("@num"), (f64)float_out);
        return NULL;
    }

    if(i[-1] == 'j' && p_end == text.data() + text.size() - 1) {
        add_token(TK("@imag"), (f64)float_out);
        return NULL;
    }

    return SyntaxError("invalid number literal");
}

Error* Lexer::lex_one_token(bool* eof) noexcept{
    *eof = false;
    while(peekchar() != '\0') {
        token_start = curr_char;
        char c = eatchar_include_newline();
        switch(c) {
            case '\'':
            case '"': {
                Error* err = eat_string(c, StringType::NORMAL_STRING);
                if(err) return err;
                return NULL;
            }
            case '#': skip_line_comment(); break;
            case '~': add_token(TK("~")); return NULL;
            case '{': add_token(TK("{")); return NULL;
            case '}': add_token(TK("}")); return NULL;
            case ',': add_token(TK(",")); return NULL;
            case ':': add_token(TK(":")); return NULL;
            case ';': add_token(TK(";")); return NULL;
            case '(': add_token(TK("(")); return NULL;
            case ')': add_token(TK(")")); return NULL;
            case '[': add_token(TK("[")); return NULL;
            case ']': add_token(TK("]")); return NULL;
            case '@': add_token(TK("@")); return NULL;
            case '\\': {
                // line continuation character
                char c = eatchar_include_newline();
                if(c != '\n') {
                    if(src->mode == REPL_MODE && c == '\0') return NeedMoreLines();
                    return SyntaxError("expected newline after line continuation character");
                }
                eat_spaces();
                return NULL;
            }
            case '%': add_token_2('=', TK("%"), TK("%=")); return NULL;
            case '&': add_token_2('=', TK("&"), TK("&=")); return NULL;
            case '|': add_token_2('=', TK("|"), TK("|=")); return NULL;
            case '^': add_token_2('=', TK("^"), TK("^=")); return NULL;
            case '.': {
                if(matchchar('.')) {
                    if(matchchar('.')) {
                        add_token(TK("..."));
                    } else {
                        add_token(TK(".."));
                    }
                } else {
                    char next_char = peekchar();
                    if(next_char >= '0' && next_char <= '9') {
                        Error* err = eat_number();
                        if(err) return err;
                    } else {
                        add_token(TK("."));
                    }
                }
                return NULL;
            }
            case '=': add_token_2('=', TK("="), TK("==")); return NULL;
            case '+': add_token_2('=', TK("+"), TK("+=")); return NULL;
            case '>': {
                if(matchchar('='))
                    add_token(TK(">="));
                else if(matchchar('>'))
                    add_token_2('=', TK(">>"), TK(">>="));
                else
                    add_token(TK(">"));
                return NULL;
            }
            case '<': {
                if(matchchar('='))
                    add_token(TK("<="));
                else if(matchchar('<'))
                    add_token_2('=', TK("<<"), TK("<<="));
                else
                    add_token(TK("<"));
                return NULL;
            }
            case '-': {
                if(matchchar('='))
                    add_token(TK("-="));
                else if(matchchar('>'))
                    add_token(TK("->"));
                else
                    add_token(TK("-"));
                return NULL;
            }
            case '!':
                if(matchchar('=')){
                    add_token(TK("!="));
                }else{
                    Error* err = SyntaxError("expected '=' after '!'");
                    if(err) return err;
                }
                break;
            case '*':
                if(matchchar('*')) {
                    add_token(TK("**"));  // '**'
                } else {
                    add_token_2('=', TK("*"), TK("*="));
                }
                return NULL;
            case '/':
                if(matchchar('/')) {
                    add_token_2('=', TK("//"), TK("//="));
                } else {
                    add_token_2('=', TK("/"), TK("/="));
                }
                return NULL;
            case ' ':
            case '\t': eat_spaces(); break;
            case '\n': {
                add_token(TK("@eol"));
                if(!eat_indentation()){
                    return IndentationError("unindent does not match any outer indentation level");
                }
                return NULL;
            }
            default: {
                if(c == 'f') {
                    if(matchchar('\'')) return eat_string('\'', StringType::F_STRING);
                    if(matchchar('"')) return eat_string('"', StringType::F_STRING);
                } else if(c == 'r') {
                    if(matchchar('\'')) return eat_string('\'', StringType::RAW_STRING);
                    if(matchchar('"')) return eat_string('"', StringType::RAW_STRING);
                } else if(c == 'b') {
                    if(matchchar('\'')) return eat_string('\'', StringType::NORMAL_BYTES);
                    if(matchchar('"')) return eat_string('"', StringType::NORMAL_BYTES);
                }
                if(c >= '0' && c <= '9') return eat_number();
                return eat_name();
            }
        }
    }

    token_start = curr_char;
    while(indents.size() > 1) {
        indents.pop_back();
        add_token(TK("@dedent"));
        return NULL;
    }
    add_token(TK("@eof"));
    *eof = true;
    return NULL;
}

Error* Lexer::_error(bool lexer_err, const char* type, const char* msg, va_list* args, i64 userdata) noexcept{
    Error* err = (Error*)malloc(sizeof(Error));
    err->type = type;
    err->src = src;
    PK_INCREF(src);
    if(lexer_err){
        err->lineno = current_line;
        err->cursor = curr_char;
        if(*curr_char == '\n') {
            err->lineno--;
            err->cursor--;
        }
    }else{
        err->lineno = -1;
        err->cursor = NULL;
    }
    if(args){
        vsnprintf(err->msg, sizeof(err->msg), msg, *args);
    }else{
        std::strncpy(err->msg, msg, sizeof(err->msg));
    }
    err->userdata = userdata;
    return err;
}

Error* Lexer::SyntaxError(const char* fmt, ...) noexcept{
    va_list args;
    va_start(args, fmt);
    Error* err = _error(true, "SyntaxError", fmt, &args);
    va_end(args);
    return err;
}

Error* Lexer::run() noexcept{
    assert(!this->used);
    this->used = true;
    if(src->is_precompiled) {
        return from_precompiled();
    }
    // push initial tokens
    this->nexts.push_back(Token{TK("@sof"), token_start, 0, current_line, brackets_level, {}});
    this->indents.push_back(0);

    bool eof = false;
    while(!eof) {
        Error* err = lex_one_token(&eof);
        if(err) return err;
    }
    return NULL;
}

Error* Lexer::from_precompiled() noexcept{
    pkpy_TokenDeserializer deserializer;
    pkpy_TokenDeserializer__ctor(&deserializer, pkpy_Str__data(&src->source));

    deserializer.curr += 5;  // skip "pkpy:"
    c11_string version = pkpy_TokenDeserializer__read_string(&deserializer, '\n');

    if(c11_string__cmp3(version, PK_VERSION) != 0) {
        return SyntaxError("precompiled version mismatch");
    }
    if(pkpy_TokenDeserializer__read_uint(&deserializer, '\n') != (i64)src->mode){
        return SyntaxError("precompiled mode mismatch");
    }

    int count = pkpy_TokenDeserializer__read_count(&deserializer);
    c11_vector* precompiled_tokens = &src->_precompiled_tokens;
    for(int i = 0; i < count; i++) {
        c11_string item = pkpy_TokenDeserializer__read_string(&deserializer, '\n');
        pkpy_Str copied_item;
        pkpy_Str__ctor2(&copied_item, item.data, item.size);
        c11_vector__push(pkpy_Str, precompiled_tokens, copied_item);
    }

    count = pkpy_TokenDeserializer__read_count(&deserializer);
    for(int i = 0; i < count; i++) {
        Token t;
        t.type = (unsigned char)pkpy_TokenDeserializer__read_uint(&deserializer, ',');
        if(is_raw_string_used(t.type)) {
            i64 index = pkpy_TokenDeserializer__read_uint(&deserializer, ',');
            pkpy_Str* p = c11__at(pkpy_Str, precompiled_tokens, index);
            t.start = pkpy_Str__data(p);
            t.length = c11__getitem(pkpy_Str, precompiled_tokens, index).size;
        } else {
            t.start = NULL;
            t.length = 0;
        }

        if(pkpy_TokenDeserializer__match_char(&deserializer, ',')) {
            t.line = nexts.back().line;
        } else {
            t.line = (int)pkpy_TokenDeserializer__read_uint(&deserializer, ',');
        }

        if(pkpy_TokenDeserializer__match_char(&deserializer, ',')) {
            t.brackets_level = nexts.back().brackets_level;
        } else {
            t.brackets_level = (int)pkpy_TokenDeserializer__read_uint(&deserializer, ',');
        }

        char type = (*deserializer.curr++);      // read_char
        switch(type) {
            case 'I':
                t.value = pkpy_TokenDeserializer__read_uint(&deserializer, '\n');
                break;
            case 'F':
                t.value = pkpy_TokenDeserializer__read_float(&deserializer, '\n');
                break;
            case 'S': {
                pkpy_Str res = pkpy_TokenDeserializer__read_string_from_hex(&deserializer, '\n');
                t.value = Str(std::move(res));
            } break;
            default:
                t.value = {};
                break;
        }
        nexts.push_back(t);
    }
    return NULL;
}

Error* Lexer::precompile(Str* out) noexcept{
    assert(!src->is_precompiled);
    Error* err = run();
    if(err) return err;
    SStream ss;
    ss << "pkpy:" PK_VERSION << '\n';       // L1: version string
    ss << (int)src->mode << '\n';           // L2: mode

    c11_smallmap_s2n token_indices;
    c11_smallmap_s2n__ctor(&token_indices);

    for(auto token: nexts) {
        if(is_raw_string_used(token.type)) {
            c11_string token_sv = {token.start, token.length};
            if(!c11_smallmap_s2n__contains(&token_indices, token_sv)) {
                c11_smallmap_s2n__set(&token_indices, token_sv, 0);
                // assert no '\n' in token.sv()
                for(char c: token.sv())
                    assert(c != '\n');
            }
        }
    }
    ss << "=" << (int)token_indices.count << '\n';  // L3: raw string count
    uint16_t index = 0;
    for(int i=0; i<token_indices.count; i++){
        auto kv = c11__at(c11_smallmap_s2n_KV, &token_indices, i);
        ss << kv->key << '\n';  // L4: raw strings
        kv->value = index++;
    }

    ss << "=" << (int)nexts.size() << '\n';  // L5: token count
    for(int i = 0; i < nexts.size(); i++) {
        const Token& token = nexts[i];
        ss << (int)token.type << ',';
        if(is_raw_string_used(token.type)) {
            uint16_t *p = c11_smallmap_s2n__try_get(&token_indices, {token.start, token.length});
            assert(p != NULL);
            ss << (int)*p << ',';
        }
        if(i > 0 && nexts[i - 1].line == token.line)
            ss << ',';
        else
            ss << token.line << ',';
        if(i > 0 && nexts[i - 1].brackets_level == token.brackets_level)
            ss << ',';
        else
            ss << token.brackets_level << ',';
        // visit token value
        std::visit(
            [&ss](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr(std::is_same_v<T, i64>) {
                    ss << 'I' << arg;
                } else if constexpr(std::is_same_v<T, f64>) {
                    ss << 'F' << arg;
                } else if constexpr(std::is_same_v<T, Str>) {
                    ss << 'S';
                    for(char c: arg)
                        ss.write_hex((unsigned char)c);
                }
                ss << '\n';
            },
            token.value);
    }
    *out = ss.str();
    c11_smallmap_s2n__dtor(&token_indices);
    return NULL;
}

IntParsingResult parse_uint(std::string_view text, i64* out, int base) noexcept{
    *out = 0;

    if(base == -1) {
        if(text.substr(0, 2) == "0b")
            base = 2;
        else if(text.substr(0, 2) == "0o")
            base = 8;
        else if(text.substr(0, 2) == "0x")
            base = 16;
        else
            base = 10;
    }

    if(base == 10) {
        // 10-base  12334
        if(text.length() == 0) return IntParsingResult::Failure;
        for(char c: text) {
            if(c >= '0' && c <= '9') {
                *out = (*out * 10) + (c - '0');
            } else {
                return IntParsingResult::Failure;
            }
        }
        const std::string_view INT64_MAX_S = "9223372036854775807";
        if(text.length() > INT64_MAX_S.length()) return IntParsingResult::Overflow;
        return IntParsingResult::Success;
    } else if(base == 2) {
        // 2-base   0b101010
        if(text.substr(0, 2) == "0b") text.remove_prefix(2);
        if(text.length() == 0) return IntParsingResult::Failure;
        for(char c: text) {
            if(c == '0' || c == '1') {
                *out = (*out << 1) | (c - '0');
            } else {
                return IntParsingResult::Failure;
            }
        }
        const std::string_view INT64_MAX_S = "111111111111111111111111111111111111111111111111111111111111111";
        if(text.length() > INT64_MAX_S.length()) return IntParsingResult::Overflow;
        return IntParsingResult::Success;
    } else if(base == 8) {
        // 8-base   0o123
        if(text.substr(0, 2) == "0o") text.remove_prefix(2);
        if(text.length() == 0) return IntParsingResult::Failure;
        for(char c: text) {
            if(c >= '0' && c <= '7') {
                *out = (*out << 3) | (c - '0');
            } else {
                return IntParsingResult::Failure;
            }
        }
        const std::string_view INT64_MAX_S = "777777777777777777777";
        if(text.length() > INT64_MAX_S.length()) return IntParsingResult::Overflow;
        return IntParsingResult::Success;
    } else if(base == 16) {
        // 16-base  0x123
        if(text.substr(0, 2) == "0x") text.remove_prefix(2);
        if(text.length() == 0) return IntParsingResult::Failure;
        for(char c: text) {
            if(c >= '0' && c <= '9') {
                *out = (*out << 4) | (c - '0');
            } else if(c >= 'a' && c <= 'f') {
                *out = (*out << 4) | (c - 'a' + 10);
            } else if(c >= 'A' && c <= 'F') {
                *out = (*out << 4) | (c - 'A' + 10);
            } else {
                return IntParsingResult::Failure;
            }
        }
        const std::string_view INT64_MAX_S = "7fffffffffffffff";
        if(text.length() > INT64_MAX_S.length()) return IntParsingResult::Overflow;
        return IntParsingResult::Success;
    }
    return IntParsingResult::Failure;
}

}  // namespace pkpy
