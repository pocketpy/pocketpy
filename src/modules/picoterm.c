#include "pocketpy/common/str.h"
#include "pocketpy/pocketpy.h"
#include "pocketpy/objects/base.h"
#include "pocketpy/common/vector.h"

#include <stdio.h>

const char* c11__u32_east_asian_width(int c);

static int c11__wcwidth(int c) {
    if(c >= 32 && c < 0x7f) return 1;
    if(c < 32) return 0;
    const char* w = c11__u32_east_asian_width(c);
    bool fullwidth = (w[0] == 'F' && w[1] == '\0') || (w[0] == 'W' && w[1] == '\0');
    return fullwidth ? 2 : 1;
}

static bool picoterm_enable_full_buffering_mode(int argc, py_Ref argv) {
    PY_CHECK_ARGC(0);
    static char buf[1024 * 32];  // 32KB
    setvbuf(stdout, buf, _IOFBF, sizeof(buf));
    py_newnone(py_retval());
    return true;
}

// https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797
typedef struct {
    c11_sv text;
    char suffix;
} AnsiEscapedToken;

static bool split_ansi_escaped_string(c11_sv sv, c11_vector* out_tokens);

static bool picoterm_split_ansi_escaped_string(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_str);
    c11_sv s = py_tosv(argv);
    c11_vector /*T=AnsiEscapedToken*/ tokens;
    c11_vector__ctor(&tokens, sizeof(AnsiEscapedToken));
    if(!split_ansi_escaped_string(s, &tokens)) {
        c11_vector__dtor(&tokens);
        return ValueError("invalid ANSI escape sequences");
    }
    py_newlistn(py_retval(), tokens.length);
    for(int i = 0; i < tokens.length; i++) {
        AnsiEscapedToken t = c11__getitem(AnsiEscapedToken, &tokens, i);
        py_ItemRef item = py_list_getitem(py_retval(), i);
        py_newstrv(item, t.text);
    }
    c11_vector__dtor(&tokens);
    return true;
}

static bool picoterm_wcwidth(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_int);
    int c = py_toint(py_arg(0));
    py_newint(py_retval(), c11__wcwidth(c));
    return true;
}

static bool picoterm_wcswidth(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_str);
    c11_sv sv = py_tosv(py_arg(0));
    c11_vector /*T=AnsiEscapedToken*/ tokens;
    c11_vector__ctor(&tokens, sizeof(AnsiEscapedToken));
    if(!split_ansi_escaped_string(sv, &tokens)) {
        c11_vector__dtor(&tokens);
        return ValueError("invalid ANSI escape sequences");
    }
    int total_width = 0;
    for(int i = 0; i < tokens.length; i++) {
        AnsiEscapedToken* p_token = c11__at(AnsiEscapedToken, &tokens, i);
        if(p_token->suffix != '\0') continue;
        const char* curr_char = p_token->text.data;
        const char* end_char = p_token->text.data + p_token->text.size;
        while(curr_char < end_char) {
            unsigned char c = *curr_char;
            int u8bytes = c11__u8_header(c, true);
            if(u8bytes == 0) return ValueError("invalid utf-8 header: %d", (int)c);
            int value = c11__u8_value(u8bytes, curr_char);
            total_width += c11__wcwidth(value);
            curr_char += u8bytes;
        }
    }
    c11_vector__dtor(&tokens);
    py_newint(py_retval(), total_width);
    return true;
}

static bool picoterm_sscanf(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    PY_CHECK_ARG_TYPE(0, tp_str);
    PY_CHECK_ARG_TYPE(1, tp_str);
    PY_CHECK_ARG_TYPE(2, tp_list);
    const char* input = py_tostr(py_arg(0));
    const char* format = py_tostr(py_arg(1));
    py_Ref output_list = py_arg(2);
    py_list_clear(output_list);

    const char* p1 = input;
    const char* p2 = format;

    while(*p1 != '\0' && *p2 != '\0') {
        if(*p2 == '%') {
            p2++;
            if(*p2 == 'd' || *p2 == 'i') {
                bool negative = false;
                if(*p1 == '-') {
                    negative = true;
                    p1++;
                }
                const char* start = p1;
                while(*p1 >= '0' && *p1 <= '9')
                    p1++;
                c11_sv num_sv = {.data = start, .size = p1 - start};
                if(num_sv.size == 0) break;

                int64_t value = 0;
                IntParsingResult res = c11__parse_uint(num_sv, &value, 10);
                if(res != IntParsing_SUCCESS) break;

                if(negative) value = -value;
                py_ItemRef item = py_list_emplace(output_list);
                py_newint(item, value);
                p2++;
            } else {
                return ValueError("unsupported format specifier: %%%c", *p2);
            }
        } else {
            if(*p1 != *p2) break;
            p1++;
            p2++;
        }
    }

    py_newbool(py_retval(), *p2 == '\0');
    return true;
}

void pk__add_module_picoterm() {
    py_Ref mod = py_newmodule("picoterm");

    py_bindfunc(mod, "enable_full_buffering_mode", picoterm_enable_full_buffering_mode);
    py_bindfunc(mod, "split_ansi_escaped_string", picoterm_split_ansi_escaped_string);
    py_bindfunc(mod, "wcwidth", picoterm_wcwidth);
    py_bindfunc(mod, "wcswidth", picoterm_wcswidth);
    py_bindfunc(mod, "sscanf", picoterm_sscanf);
}

static bool split_ansi_escaped_string(c11_sv sv, c11_vector* out_tokens) {
    const char* p = sv.data;
    int i = 0;
    while(i < sv.size) {
        if(p[i] == '\x1b') {
            i++;  // skip '\x1b'
            if(i >= sv.size || p[i] != '[') {
                return false;  // invalid escape sequence
            }

            int esc_start = i - 1;
            i++;  // skip '['

            c11_sv content;
            content.data = p + i;
            while(i < sv.size && !((p[i] >= 'A' && p[i] <= 'Z') || (p[i] >= 'a' && p[i] <= 'z'))) {
                i++;
            }
            content.size = p + i - content.data;
            if(i >= sv.size) {
                return false;  // invalid escape sequence
            }

            char suffix = p[i];
            i++;  // skip suffix

            AnsiEscapedToken token;
            token.text = (c11_sv){p + esc_start, i - esc_start};
            token.suffix = suffix;
            c11_vector__push(AnsiEscapedToken, out_tokens, token);
        } else if(p[i] == '\n') {
            AnsiEscapedToken token;
            token.text = (c11_sv){p + i, 1};
            token.suffix = '\n';
            c11_vector__push(AnsiEscapedToken, out_tokens, token);
            i++;
        } else {
            int text_start = i;
            while(i < sv.size && p[i] != '\x1b' && p[i] != '\n') {
                i++;
            }
            AnsiEscapedToken token;
            token.text = (c11_sv){p + text_start, i - text_start};
            token.suffix = '\0';
            c11_vector__push(AnsiEscapedToken, out_tokens, token);
        }
    }

    return true;
}
