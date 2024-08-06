#pragma once

#include "pocketpy/common/vector.h"
#include "pocketpy/common/str.h"
#include "pocketpy/common/utils.h"

#include <stdint.h>

typedef struct c11_sbuf {
    c11_vector data;
} c11_sbuf;

void c11_sbuf__ctor(c11_sbuf* self);
void c11_sbuf__dtor(c11_sbuf* self);

void c11_sbuf__write_int(c11_sbuf* self, int);
void c11_sbuf__write_i64(c11_sbuf* self, int64_t);
void c11_sbuf__write_f64(c11_sbuf* self, double, int precision);
void c11_sbuf__write_char(c11_sbuf* self, char);
void c11_sbuf__write_pad(c11_sbuf* self, int count, char pad);
void c11_sbuf__write_sv(c11_sbuf* self, c11_sv);
void c11_sbuf__write_cstr(c11_sbuf* self, const char*);
void c11_sbuf__write_cstrn(c11_sbuf* self, const char*, int);
void c11_sbuf__write_quoted(c11_sbuf* self, c11_sv, char quote);
void c11_sbuf__write_hex(c11_sbuf* self, unsigned char, bool non_zero);
void c11_sbuf__write_ptr(c11_sbuf* self, void*);
// Submit the stream and return the final string. The stream becomes invalid after this call
c11_string* c11_sbuf__submit(c11_sbuf* self);
void c11_sbuf__py_submit(c11_sbuf* self, py_Ref out);

void pk_vsprintf(c11_sbuf* ss, const char* fmt, va_list args);
void pk_sprintf(c11_sbuf* ss, const char* fmt, ...);
