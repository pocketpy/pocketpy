#pragma once

#include <stdint.h>

#include "pocketpy/common/vector.h"
#include "pocketpy/common/str.h"


typedef struct c11_serializer {
    c11_vector data;
} c11_serializer;

void c11_serializer__ctor(c11_serializer* self, int16_t magic, int8_t major_ver, int8_t minor_ver);
void c11_serializer__dtor(c11_serializer* self);
void c11_serializer__write_cstr(c11_serializer* self, const char*);
void c11_serializer__write_bytes(c11_serializer* self, const void* data, int size);
void* c11_serializer__submit(c11_serializer* self, int* size);

typedef struct c11_deserializer {
    char error_msg[64];
    const uint8_t* data;
    int size;
    int index;
    int8_t major_ver;
    int8_t minor_ver;
} c11_deserializer;

void c11_deserializer__ctor(c11_deserializer* self, const void* data, int size);
void c11_deserializer__dtor(c11_deserializer* self);
bool c11_deserializer__check_header(c11_deserializer* self, int16_t magic, int8_t major_ver, int8_t minor_ver_min);
const char* c11_deserializer__read_cstr(c11_deserializer* self);
void* c11_deserializer__read_bytes(c11_deserializer* self, int size);


#define DEF_ATOMIC_INLINE_RW(name, T) \
    static inline void c11_serializer__write_##name(c11_serializer* self, T value){ \
        c11_serializer__write_bytes(self, &value, sizeof(T)); \
    } \
    static inline T c11_deserializer__read_##name(c11_deserializer* self){ \
        const void* p = self->data + self->index; \
        self->index += sizeof(T); \
        T retval;\
        memcpy(&retval, p, sizeof(T)); \
        return retval; \
    }

DEF_ATOMIC_INLINE_RW(i8, int8_t)
DEF_ATOMIC_INLINE_RW(i16, int16_t)
DEF_ATOMIC_INLINE_RW(i32, int32_t)
DEF_ATOMIC_INLINE_RW(i64, int64_t)
DEF_ATOMIC_INLINE_RW(f32, float)
DEF_ATOMIC_INLINE_RW(f64, double)
DEF_ATOMIC_INLINE_RW(type, py_Type)

#undef DEF_ATOMIC_INLINE_RW

