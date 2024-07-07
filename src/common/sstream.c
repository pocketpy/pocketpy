#include "pocketpy/common/sstream.h"
#include "pocketpy/common/config.h"
#include "pocketpy/common/str.h"
#include "pocketpy/common/utils.h"
#include "pocketpy/pocketpy.h"

#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <math.h>

void c11_sbuf__ctor(c11_sbuf* self) {
    c11_vector__ctor(&self->data, sizeof(char));
    c11_vector__reserve(&self->data, sizeof(c11_string) + 100);
    self->data.count = sizeof(c11_string);
}

void c11_sbuf__dtor(c11_sbuf* self) { c11_vector__dtor(&self->data); }

void c11_sbuf__write_char(c11_sbuf* self, char c) { c11_vector__push(char, &self->data, c); }

void c11_sbuf__write_int(c11_sbuf* self, int i) {
    // len('-2147483648') == 11
    c11_vector__reserve(&self->data, self->data.count + 11 + 1);
    char* p = (char*)self->data.data + self->data.count;
    int n = snprintf(p, 11 + 1, "%d", i);
    self->data.count += n;
}

void c11_sbuf__write_i64(c11_sbuf* self, int64_t val) {
    // len('-9223372036854775808') == 20
    c11_vector__reserve(&self->data, self->data.count + 20 + 1);
    char* p = (char*)self->data.data + self->data.count;
    int n = snprintf(p, 20 + 1, "%lld", (long long)val);
    self->data.count += n;
}

void c11_sbuf__write_f64(c11_sbuf* self, double val, int precision) {
    if(isinf(val)) {
        c11_sbuf__write_cstr(self, val > 0 ? "inf" : "-inf");
        return;
    }
    if(isnan(val)) {
        c11_sbuf__write_cstr(self, "nan");
        return;
    }
    char b[32];
    int size;
    if(precision < 0) {
        int prec = 17 - 1;  // std::numeric_limits<double>::max_digits10 == 17
        size = snprintf(b, sizeof(b), "%.*g", prec, val);
    } else {
        int prec = precision;
        size = snprintf(b, sizeof(b), "%.*f", prec, val);
    }
    c11_sbuf__write_cstr(self, b);
    bool all_is_digit = true;
    for(int i = 1; i < size; i++) {
        if(!isdigit(b[i])) {
            all_is_digit = false;
            break;
        }
    }
    if(all_is_digit) c11_sbuf__write_cstr(self, ".0");
}

void c11_sbuf__write_sv(c11_sbuf* self, c11_sv sv) {
    c11_sbuf__write_cstrn(self, sv.data, sv.size);
}

void c11_sbuf__write_cstr(c11_sbuf* self, const char* str) {
    c11_sbuf__write_cstrn(self, str, strlen(str));
}

void c11_sbuf__write_cstrn(c11_sbuf* self, const char* str, int n) {
    c11_vector__extend(char, &self->data, str, n);
}

void c11_sbuf__write_quoted(c11_sbuf* self, c11_sv sv, char quote) {
    assert(quote == '"' || quote == '\'');
    c11_sbuf__write_char(self, quote);
    for(int i = 0; i < sv.size; i++) {
        char c = sv.data[i];
        switch(c) {
            case '"':
            case '\'':
                if(c == quote) c11_sbuf__write_char(self, '\\');
                c11_sbuf__write_char(self, c);
                break;
            case '\\': c11_sbuf__write_cstrn(self, "\\\\", 2); break;
            case '\n': c11_sbuf__write_cstrn(self, "\\n", 2); break;
            case '\r': c11_sbuf__write_cstrn(self, "\\r", 2); break;
            case '\t': c11_sbuf__write_cstrn(self, "\\t", 2); break;
            case '\b': c11_sbuf__write_cstrn(self, "\\b", 2); break;
            default:
                if('\x00' <= c && c <= '\x1f') {
                    c11_sbuf__write_cstrn(self, "\\x", 2);
                    c11_sbuf__write_char(self, PK_HEX_TABLE[c >> 4]);
                    c11_sbuf__write_char(self, PK_HEX_TABLE[c & 0xf]);
                } else {
                    c11_sbuf__write_char(self, c);
                }
        }
    }
    c11_sbuf__write_char(self, quote);
}

void c11_sbuf__write_hex(c11_sbuf* self, unsigned char c, bool non_zero) {
    unsigned char high = c >> 4;
    unsigned char low = c & 0xf;
    if(non_zero) {
        if(high) c11_sbuf__write_char(self, PK_HEX_TABLE[high]);
        if(high || low) c11_sbuf__write_char(self, PK_HEX_TABLE[low]);
    } else {
        c11_sbuf__write_char(self, PK_HEX_TABLE[high]);
        c11_sbuf__write_char(self, PK_HEX_TABLE[low]);
    }
}

void c11_sbuf__write_ptr(c11_sbuf* self, void* p) {
    if(p == NULL) {
        c11_sbuf__write_cstr(self, "0x0");
        return;
    }
    c11_sbuf__write_cstr(self, "0x");
    uintptr_t p_t = (uintptr_t)(p);
    bool non_zero = true;
    for(int i = sizeof(void*) - 1; i >= 0; i--) {
        unsigned char cpnt = (p_t >> (i * 8)) & 0xff;
        c11_sbuf__write_hex(self, cpnt, non_zero);
        if(cpnt != 0) non_zero = false;
    }
}

c11_string* c11_sbuf__submit(c11_sbuf* self) {
    c11_vector__push(char, &self->data, '\0');
    c11_array arr = c11_vector__submit(&self->data);
    c11_string* retval = arr.data;
    retval->size = arr.count - sizeof(c11_string) - 1;
    return retval;
}

void pk_vsprintf(c11_sbuf* ss, const char* fmt, va_list args) {
    while(*fmt) {
        char c = *fmt;
        if(c != '%') {
            c11_sbuf__write_char(ss, c);
            fmt++;
            continue;
        }

        fmt++;
        c = *fmt;

        switch(c) {
            case 'd': {
                int i = va_arg(args, int);
                c11_sbuf__write_int(ss, i);
                break;
            }
            case 'i': {
                int64_t i = va_arg(args, int64_t);
                c11_sbuf__write_i64(ss, i);
                break;
            }
            case 'f': {
                double d = va_arg(args, double);
                c11_sbuf__write_f64(ss, d, -1);
                break;
            }
            case 's': {
                const char* s = va_arg(args, const char*);
                c11_sbuf__write_cstr(ss, s);
                break;
            }
            case 'q': {
                c11_sv sv = va_arg(args, c11_sv);
                c11_sbuf__write_quoted(ss, sv, '\'');
                break;
            }
            case 'v': {
                c11_sv sv = va_arg(args, c11_sv);
                c11_sbuf__write_sv(ss, sv);
                break;
            }
            case 'c': {
                char c = va_arg(args, int);
                c11_sbuf__write_char(ss, c);
                break;
            }
            case 'p': {
                void* p = va_arg(args, void*);
                c11_sbuf__write_ptr(ss, p);
                break;
            }
            case 't': {
                py_Type t = va_arg(args, int);
                c11_sbuf__write_cstr(ss, py_tpname(t));
                break;
            }
            case 'n': {
                py_Name n = va_arg(args, int);
                c11_sbuf__write_cstr(ss, py_name2str(n));
                break;
            }
            case '%': c11_sbuf__write_char(ss, '%'); break;
            default:
                c11_sbuf__write_char(ss, c);
                assert(false);  // invalid format
                break;
        }
        fmt++;
    }
}

void pk_sprintf(c11_sbuf* ss, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    pk_vsprintf(ss, fmt, args);
    va_end(args);
}

int py_replinput(char* buf) {
    int size = 0;
    bool multiline = false;
    printf(">>> ");

    while(true) {
        char c = getchar();
        if(c == EOF) break;

        if(c == '\n') {
            char last = '\0';
            if(size > 0) last = buf[size - 1];
            if(multiline) {
                if(last == '\n') {
                    break;  // 2 consecutive newlines to end multiline input
                } else {
                    printf("... ");
                }
            } else {
                if(last == ':' || last == '(' || last == '[' || last == '{') {
                    printf("... ");
                    multiline = true;
                } else {
                    break;
                }
            }
        }

        buf[size++] = c;
    }

    buf[size] = '\0';
    return size;
}