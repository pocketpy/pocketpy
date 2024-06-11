#include "pocketpy/compiler/lexer.hpp"
#include "pocketpy/common/gil.hpp"
#include "pocketpy/common/version.h"
#include "pocketpy/common/str.h"

#include <cstdarg>

namespace pkpy {

// clang-format off
static const uint32_t kLoRangeA[] = {170,186,443,448,660,1488,1519,1568,1601,1646,1649,1749,1774,1786,1791,1808,1810,1869,1969,1994,2048,2112,2144,2208,2230,2308,2365,2384,2392,2418,2437,2447,2451,2474,2482,2486,2493,2510,2524,2527,2544,2556,2565,2575,2579,2602,2610,2613,2616,2649,2654,2674,2693,2703,2707,2730,2738,2741,2749,2768,2784,2809,2821,2831,2835,2858,2866,2869,2877,2908,2911,2929,2947,2949,2958,2962,2969,2972,2974,2979,2984,2990,3024,3077,3086,3090,3114,3133,3160,3168,3200,3205,3214,3218,3242,3253,3261,3294,3296,3313,3333,3342,3346,3389,3406,3412,3423,3450,3461,3482,3507,3517,3520,3585,3634,3648,3713,3716,3718,3724,3749,3751,3762,3773,3776,3804,3840,3904,3913,3976,4096,4159,4176,4186,4193,4197,4206,4213,4238,4352,4682,4688,4696,4698,4704,4746,4752,4786,4792,4800,4802,4808,4824,4882,4888,4992,5121,5743,5761,5792,5873,5888,5902,5920,5952,5984,5998,6016,6108,6176,6212,6272,6279,6314,6320,6400,6480,6512,6528,6576,6656,6688,6917,6981,7043,7086,7098,7168,7245,7258,7401,7406,7413,7418,8501,11568,11648,11680,11688,11696,11704,11712,11720,11728,11736,12294,12348,12353,12447,12449,12543,12549,12593,12704,12784,13312,19968,40960,40982,42192,42240,42512,42538,42606,42656,42895,42999,43003,43011,43015,43020,43072,43138,43250,43259,43261,43274,43312,43360,43396,43488,43495,43514,43520,43584,43588,43616,43633,43642,43646,43697,43701,43705,43712,43714,43739,43744,43762,43777,43785,43793,43808,43816,43968,44032,55216,55243,63744,64112,64285,64287,64298,64312,64318,64320,64323,64326,64467,64848,64914,65008,65136,65142,65382,65393,65440,65474,65482,65490,65498,65536,65549,65576,65596,65599,65616,65664,66176,66208,66304,66349,66370,66384,66432,66464,66504,66640,66816,66864,67072,67392,67424,67584,67592,67594,67639,67644,67647,67680,67712,67808,67828,67840,67872,67968,68030,68096,68112,68117,68121,68192,68224,68288,68297,68352,68416,68448,68480,68608,68864,69376,69415,69424,69600,69635,69763,69840,69891,69956,69968,70006,70019,70081,70106,70108,70144,70163,70272,70280,70282,70287,70303,70320,70405,70415,70419,70442,70450,70453,70461,70480,70493,70656,70727,70751,70784,70852,70855,71040,71128,71168,71236,71296,71352,71424,71680,71935,72096,72106,72161,72163,72192,72203,72250,72272,72284,72349,72384,72704,72714,72768,72818,72960,72968,72971,73030,73056,73063,73066,73112,73440,73728,74880,77824,82944,92160,92736,92880,92928,93027,93053,93952,94032,94208,100352,110592,110928,110948,110960,113664,113776,113792,113808,123136,123214,123584,124928,126464,126469,126497,126500,126503,126505,126516,126521,126523,126530,126535,126537,126539,126541,126545,126548,126551,126553,126555,126557,126559,126561,126564,126567,126572,126580,126585,126590,126592,126603,126625,126629,126635,131072,173824,177984,178208,183984,194560};
static const uint32_t kLoRangeB[] = {170,186,443,451,660,1514,1522,1599,1610,1647,1747,1749,1775,1788,1791,1808,1839,1957,1969,2026,2069,2136,2154,2228,2237,2361,2365,2384,2401,2432,2444,2448,2472,2480,2482,2489,2493,2510,2525,2529,2545,2556,2570,2576,2600,2608,2611,2614,2617,2652,2654,2676,2701,2705,2728,2736,2739,2745,2749,2768,2785,2809,2828,2832,2856,2864,2867,2873,2877,2909,2913,2929,2947,2954,2960,2965,2970,2972,2975,2980,2986,3001,3024,3084,3088,3112,3129,3133,3162,3169,3200,3212,3216,3240,3251,3257,3261,3294,3297,3314,3340,3344,3386,3389,3406,3414,3425,3455,3478,3505,3515,3517,3526,3632,3635,3653,3714,3716,3722,3747,3749,3760,3763,3773,3780,3807,3840,3911,3948,3980,4138,4159,4181,4189,4193,4198,4208,4225,4238,4680,4685,4694,4696,4701,4744,4749,4784,4789,4798,4800,4805,4822,4880,4885,4954,5007,5740,5759,5786,5866,5880,5900,5905,5937,5969,5996,6000,6067,6108,6210,6264,6276,6312,6314,6389,6430,6509,6516,6571,6601,6678,6740,6963,6987,7072,7087,7141,7203,7247,7287,7404,7411,7414,7418,8504,11623,11670,11686,11694,11702,11710,11718,11726,11734,11742,12294,12348,12438,12447,12538,12543,12591,12686,12730,12799,19893,40943,40980,42124,42231,42507,42527,42539,42606,42725,42895,42999,43009,43013,43018,43042,43123,43187,43255,43259,43262,43301,43334,43388,43442,43492,43503,43518,43560,43586,43595,43631,43638,43642,43695,43697,43702,43709,43712,43714,43740,43754,43762,43782,43790,43798,43814,43822,44002,55203,55238,55291,64109,64217,64285,64296,64310,64316,64318,64321,64324,64433,64829,64911,64967,65019,65140,65276,65391,65437,65470,65479,65487,65495,65500,65547,65574,65594,65597,65613,65629,65786,66204,66256,66335,66368,66377,66421,66461,66499,66511,66717,66855,66915,67382,67413,67431,67589,67592,67637,67640,67644,67669,67702,67742,67826,67829,67861,67897,68023,68031,68096,68115,68119,68149,68220,68252,68295,68324,68405,68437,68466,68497,68680,68899,69404,69415,69445,69622,69687,69807,69864,69926,69956,70002,70006,70066,70084,70106,70108,70161,70187,70278,70280,70285,70301,70312,70366,70412,70416,70440,70448,70451,70457,70461,70480,70497,70708,70730,70751,70831,70853,70855,71086,71131,71215,71236,71338,71352,71450,71723,71935,72103,72144,72161,72163,72192,72242,72250,72272,72329,72349,72440,72712,72750,72768,72847,72966,72969,73008,73030,73061,73064,73097,73112,73458,74649,75075,78894,83526,92728,92766,92909,92975,93047,93071,94026,94032,100343,101106,110878,110930,110951,111355,113770,113788,113800,113817,123180,123214,123627,125124,126467,126495,126498,126500,126503,126514,126519,126521,126523,126530,126535,126537,126539,126543,126546,126548,126551,126553,126555,126557,126559,126562,126564,126570,126578,126583,126588,126590,126601,126619,126627,126633,126651,173782,177972,178205,183969,191456,195101};
// clang-format on

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

static bool is_unicode_Lo_char(uint32_t c) noexcept{
    // open a hole for carrot
    if(c == U'🥕') return true;
    auto index = std::lower_bound(kLoRangeA, kLoRangeA + 476, c) - kLoRangeA;
    if(c == kLoRangeA[index]) return true;
    index -= 1;
    if(index < 0) return false;
    return c >= kLoRangeA[index] && c <= kLoRangeB[index];
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
        c11_vector__push_back(const char*, &src->line_starts, curr_char);
    }
    return c;
}

Error* Lexer::eat_name() noexcept{
    curr_char--;
    while(true) {
        unsigned char c = peekchar();
        int u8bytes = pkpy_utils__u8_header(c, true);
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
        if(is_unicode_Lo_char(value))
            curr_char += u8bytes;
        else
            break;
    }

    int length = (int)(curr_char - token_start);
    if(length == 0) return SyntaxError("@id contains invalid char");
    std::string_view name(token_start, length);

    if(src->mode == PK_JSON_MODE) {
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

    auto it = std::lower_bound(KW_BEGIN, KW_END, name);
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
            if(quote3 && src->mode == PK_REPL_MODE) return NeedMoreLines();
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
                    if(src->mode == PK_REPL_MODE && c == '\0') return NeedMoreLines();
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
            case '+':
                if(matchchar('+')) {
                    add_token(TK("++"));
                } else {
                    add_token_2('=', TK("+"), TK("+="));
                }
                return NULL;
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
                if(matchchar('-')) {
                    add_token(TK("--"));
                } else {
                    if(matchchar('='))
                        add_token(TK("-="));
                    else if(matchchar('>'))
                        add_token(TK("->"));
                    else
                        add_token(TK("-"));
                }
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
    PK_THREAD_LOCAL Error err;
    err.type = type;
    err.src = src;
    if(lexer_err){
        err.lineno = current_line;
        err.cursor = curr_char;
        if(*curr_char == '\n') {
            err.lineno--;
            err.cursor--;
        }
    }else{
        err.lineno = -1;
        err.cursor = NULL;
    }
    if(args){
        vsnprintf(err.msg, sizeof(err.msg), msg, *args);
    }else{
        std::strncpy(err.msg, msg, sizeof(err.msg));
    }
    err.userdata = userdata;
    return &err;
}

Error* Lexer::SyntaxError(const char* fmt, ...) noexcept{
    va_list args;
    va_start(args, fmt);
    Error* err = _error(true, "SyntaxError", fmt, &args);
    va_end(args);
    return err;
}

Lexer::Lexer(VM* vm, SourceData src) noexcept : vm(vm), src(src){
    this->token_start = src.source().c_str();
    this->curr_char = src.source().c_str();
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
    TokenDeserializer deserializer(src.source().c_str());
    deserializer.curr += 5;  // skip "pkpy:"
    std::string_view version = deserializer.read_string('\n');

    if(version != PK_VERSION){
        return SyntaxError("precompiled version mismatch");
    }
    if(deserializer.read_uint('\n') != (i64)src->mode){
        return SyntaxError("precompiled mode mismatch");
    }

    int count = deserializer.read_count();
    auto precompiled_tokens = &src->_precompiled_tokens;
    for(int i = 0; i < count; i++) {
        c11_vector__push_back(Str, precompiled_tokens, Str(deserializer.read_string('\n')));
    }

    count = deserializer.read_count();
    for(int i = 0; i < count; i++) {
        Token t;
        t.type = (unsigned char)deserializer.read_uint(',');
        if(is_raw_string_used(t.type)) {
            i64 index = deserializer.read_uint(',');
            t.start = c11__getitem(Str, precompiled_tokens, index).c_str();
            t.length = c11__getitem(Str, precompiled_tokens, index).size;
        } else {
            t.start = nullptr;
            t.length = 0;
        }

        if(deserializer.match_char(',')) {
            t.line = nexts.back().line;
        } else {
            t.line = (int)deserializer.read_uint(',');
        }

        if(deserializer.match_char(',')) {
            t.brackets_level = nexts.back().brackets_level;
        } else {
            t.brackets_level = (int)deserializer.read_uint(',');
        }

        char type = deserializer.read_char();
        switch(type) {
            case 'I': t.value = deserializer.read_uint('\n'); break;
            case 'F': t.value = deserializer.read_float('\n'); break;
            case 'S': t.value = deserializer.read_string_from_hex('\n'); break;
            default: t.value = {}; break;
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

    small_map<std::string_view, int> token_indices;
    for(auto token: nexts) {
        if(is_raw_string_used(token.type)) {
            if(!token_indices.contains(token.sv())) {
                token_indices.insert(token.sv(), 0);
                // assert no '\n' in token.sv()
                for(char c: token.sv())
                    assert(c != '\n');
            }
        }
    }
    ss << "=" << (int)token_indices.size() << '\n';  // L3: raw string count
    int index = 0;
    for(auto& kv: token_indices) {
        ss << kv.first << '\n';  // L4: raw strings
        kv.second = index++;
    }

    ss << "=" << (int)nexts.size() << '\n';  // L5: token count
    for(int i = 0; i < nexts.size(); i++) {
        const Token& token = nexts[i];
        ss << (int)token.type << ',';
        if(is_raw_string_used(token.type)) { ss << token_indices[token.sv()] << ','; }
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
    return NULL;
}

std::string_view TokenDeserializer::read_string(char c) noexcept{
    const char* start = curr;
    while(*curr != c)
        curr++;
    std::string_view retval(start, curr - start);
    curr++;  // skip the delimiter
    return retval;
}

Str TokenDeserializer::read_string_from_hex(char c) noexcept{
    std::string_view s = read_string(c);
    char* buffer = (char*)std::malloc(s.size() / 2 + 1);
    for(int i = 0; i < s.size(); i += 2) {
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
        buffer[i / 2] = c;
    }
    buffer[s.size() / 2] = 0;
    return pair<char*, int>(buffer, s.size() / 2);
}

int TokenDeserializer::read_count() noexcept{
    assert(*curr == '=');
    curr++;
    return read_uint('\n');
}

i64 TokenDeserializer::read_uint(char c) noexcept{
    i64 out = 0;
    while(*curr != c) {
        out = out * 10 + (*curr - '0');
        curr++;
    }
    curr++;  // skip the delimiter
    return out;
}

f64 TokenDeserializer::read_float(char c) noexcept{
    std::string_view sv = read_string(c);
    return std::stod(std::string(sv));
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
