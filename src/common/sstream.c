#include "pocketpy/common/sstream.h"
#include "pocketpy/common/config.h"
#include "pocketpy/common/utils.h"
#include "pocketpy/pocketpy.h"

#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <math.h>

const static int C11_STRING_HEADER_SIZE = sizeof(int);

void pk_SStream__ctor(pk_SStream* self) {
    c11_vector__ctor(&self->data, sizeof(char));
    c11_vector__reserve(&self->data, 100 + C11_STRING_HEADER_SIZE);
    self->data.count = C11_STRING_HEADER_SIZE;
}

void pk_SStream__dtor(pk_SStream* self) { c11_vector__dtor(&self->data); }

void pk_SStream__write_char(pk_SStream* self, char c) { c11_vector__push(char, &self->data, c); }

void pk_SStream__write_int(pk_SStream* self, int i) {
    // len('-2147483648') == 11
    c11_vector__reserve(&self->data, self->data.count + 11 + 1);
    int n = sprintf(self->data.data, "%d", i);
    self->data.count += n;
}

void pk_SStream__write_i64(pk_SStream* self, int64_t val) {
    // len('-9223372036854775808') == 20
    c11_vector__reserve(&self->data, self->data.count + 20 + 1);
    int n = sprintf(self->data.data, "%lld", (long long)val);
    self->data.count += n;
}

void pk_SStream__write_f64(pk_SStream* self, double val, int precision) {
    if(isinf(val)) {
        pk_SStream__write_cstr(self, val > 0 ? "inf" : "-inf");
        return;
    }
    if(isnan(val)) {
        pk_SStream__write_cstr(self, "nan");
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
    pk_SStream__write_cstr(self, b);
    bool all_is_digit = true;
    for(int i = 1; i < size; i++) {
        if(!isdigit(b[i])) {
            all_is_digit = false;
            break;
        }
    }
    if(all_is_digit) pk_SStream__write_cstr(self, ".0");
}

void pk_SStream__write_sv(pk_SStream* self, c11_stringview sv) {
    pk_SStream__write_cstrn(self, sv.data, sv.size);
}

void pk_SStream__write_cstr(pk_SStream* self, const char* str) {
    pk_SStream__write_cstrn(self, str, strlen(str));
}

void pk_SStream__write_cstrn(pk_SStream* self, const char* str, int n) {
    c11_vector__extend(char, &self->data, str, n);
}

void pk_SStream__write_hex(pk_SStream* self, unsigned char c, bool non_zero) {
    unsigned char high = c >> 4;
    unsigned char low = c & 0xf;
    if(non_zero) {
        if(high) pk_SStream__write_char(self, PK_HEX_TABLE[high]);
        if(high || low) pk_SStream__write_char(self, PK_HEX_TABLE[low]);
    } else {
        pk_SStream__write_char(self, PK_HEX_TABLE[high]);
        pk_SStream__write_char(self, PK_HEX_TABLE[low]);
    }
}

void pk_SStream__write_ptr(pk_SStream* self, void* p) {
    if(p == NULL) {
        pk_SStream__write_cstr(self, "0x0");
        return;
    }
    pk_SStream__write_cstr(self, "0x");
    uintptr_t p_t = (uintptr_t)(p);
    bool non_zero = true;
    for(int i = sizeof(void*) - 1; i >= 0; i--) {
        unsigned char cpnt = (p_t >> (i * 8)) & 0xff;
        pk_SStream__write_hex(self, cpnt, non_zero);
        if(cpnt != 0) non_zero = false;
    }
}

c11_string* pk_SStream__submit(pk_SStream* self) {
    c11_vector__push(char, &self->data, '\0');
    c11_array arr = c11_vector__submit(&self->data);
    int* p = arr.data;
    *p = arr.count - C11_STRING_HEADER_SIZE - 1;
    return (c11_string*)(p + 1);
}

void pk_vsprintf(pk_SStream* ss, const char* fmt, va_list args) {
    while(fmt) {
        char c = *fmt;
        if(c != '%') {
            pk_SStream__write_char(ss, c);
            fmt++;
            continue;
        }

        fmt++;
        c = *fmt;

        switch(c) {
            case 'd': {
                int i = va_arg(args, int);
                pk_SStream__write_int(ss, i);
                break;
            }
            case 'i': {
                int64_t i = va_arg(args, int64_t);
                pk_SStream__write_i64(ss, i);
                break;
            }
            case 'f': {
                double d = va_arg(args, double);
                pk_SStream__write_f64(ss, d, -1);
                break;
            }
            case 's': {
                const char* s = va_arg(args, const char*);
                pk_SStream__write_cstr(ss, s);
                break;
            }
            case 'q': {
                const char* s = va_arg(args, const char*);
                c11_stringview sv = {s, strlen(s)};
                c11_sv__quote(sv, '\'', &ss->data);
                break;
            }
            case 'c': {
                char c = va_arg(args, int);
                pk_SStream__write_char(ss, c);
                break;
            }
            case 'p': {
                void* p = va_arg(args, void*);
                pk_SStream__write_ptr(ss, p);
                break;
            }
            case 't': {
                py_Type t = va_arg(args, int);
                pk_SStream__write_cstr(ss, py_tpname(t));
                break;
            }
            case 'n': {
                py_Name n = va_arg(args, int);
                pk_SStream__write_cstr(ss, py_name2str(n));
                break;
            }
            case '%': pk_SStream__write_char(ss, '%'); break;
            default:
                pk_SStream__write_char(ss, c);
                assert(false);  // invalid format
                break;
        }
        fmt++;
    }
}

void pk_sprintf(pk_SStream* ss, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    pk_vsprintf(ss, fmt, args);
    va_end(args);
}

const char* py_fmt(const char* fmt, ...) {
    PK_THREAD_LOCAL pk_SStream ss;
    if(ss.data.elem_size == 0) {
        pk_SStream__ctor(&ss);
    } else {
        c11_vector__clear(&ss.data);
    }
    va_list args;
    va_start(args, fmt);
    pk_vsprintf(&ss, fmt, args);
    va_end(args);
    pk_SStream__write_char(&ss, '\0');
    return (const char*)ss.data.data;
}
