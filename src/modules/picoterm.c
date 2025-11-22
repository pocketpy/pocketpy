#include "pocketpy/pocketpy.h"
#include "pocketpy/objects/base.h"
#include <stdio.h>
#include "pocketpy/common/vector.h"

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

void pk__add_module_picoterm() {
    py_Ref mod = py_newmodule("picoterm");

    py_bindfunc(mod, "enable_full_buffering_mode", picoterm_enable_full_buffering_mode);
    py_bindfunc(mod, "split_ansi_escaped_string", picoterm_split_ansi_escaped_string);
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
