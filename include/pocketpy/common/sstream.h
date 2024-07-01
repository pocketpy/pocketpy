#pragma once

#include "pocketpy/common/vector.h"
#include "pocketpy/common/str.h"
#include "pocketpy/common/utils.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct pk_SStream {
    c11_vector data;
} pk_SStream;

void pk_SStream__ctor(pk_SStream* self);
void pk_SStream__dtor(pk_SStream* self);

void pk_SStream__write_int(pk_SStream* self, int);
void pk_SStream__write_i64(pk_SStream* self, int64_t);
void pk_SStream__write_f64(pk_SStream* self, double, int precision);
void pk_SStream__write_char(pk_SStream* self, char);
void pk_SStream__write_sv(pk_SStream* self, c11_stringview);
void pk_SStream__write_cstr(pk_SStream* self, const char*);
void pk_SStream__write_cstrn(pk_SStream* self, const char*, int);
void pk_SStream__write_hex(pk_SStream* self, unsigned char, bool non_zero);
void pk_SStream__write_ptr(pk_SStream* self, void*);
// Submit the stream and return the final string. The stream becomes invalid after this call
c11_string* pk_SStream__submit(pk_SStream* self);

void pk_vsprintf(pk_SStream* ss, const char* fmt, va_list args);
void pk_sprintf(pk_SStream* ss, const char* fmt, ...);

#ifdef __cplusplus
}
#endif
