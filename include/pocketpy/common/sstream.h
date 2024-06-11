#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "pocketpy/common/vector.h"
#include "pocketpy/common/str.h"
#include <stdint.h>

typedef struct pkpy_SStream {
    c11_vector data;
} pkpy_SStream;

void pkpy_SStream__ctor(pkpy_SStream* self);
void pkpy_SStream__dtor(pkpy_SStream* self);
void pkpy_SStream__append_cstr(pkpy_SStream* self, const char* str);
void pkpy_SStream__append_cstrn(pkpy_SStream* self, const char* str, int n);
void pkpy_SStream__append_Str(pkpy_SStream* self, pkpy_Str* str);
void pkpy_SStream__append_char(pkpy_SStream* self, char c);
void pkpy_SStream__append_int(pkpy_SStream* self, int i);
void pkpy_SStream__append_int64(pkpy_SStream* self, int64_t i);
pkpy_Str pkpy_SStream__to_Str(pkpy_SStream* self);

#ifdef __cplusplus
}
#endif
