#pragma once

#include <stdbool.h>
#include "pocketpy/common/str.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct pkpy_TokenDeserializer {
    const char* curr;
    const char* source;
} pkpy_TokenDeserializer;

void pkpy_TokenDeserializer__ctor(pkpy_TokenDeserializer* self, const char* source);
bool pkpy_TokenDeserializer__match_char(pkpy_TokenDeserializer* self, char c);
c11_string pkpy_TokenDeserializer__read_string(pkpy_TokenDeserializer* self, char c);
pkpy_Str pkpy_TokenDeserializer__read_string_from_hex(pkpy_TokenDeserializer* self, char c);
int pkpy_TokenDeserializer__read_count(pkpy_TokenDeserializer* self);
int64_t pkpy_TokenDeserializer__read_uint(pkpy_TokenDeserializer* self, char c);
double pkpy_TokenDeserializer__read_float(pkpy_TokenDeserializer* self, char c);

#ifdef __cplusplus
}
#endif