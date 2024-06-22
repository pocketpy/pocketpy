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

typedef struct pk_AnyStr {
    int type;
    union {
        int _int;
        int64_t _i64;
        float _float;
        double _double;
        char _char;
        const pkpy_Str* _str;
        c11_string _sv;
        const char* _cstr;
        void* _ptr;
    };
} pk_AnyStr;

PK_INLINE pk_AnyStr pk_AnyStr__int(int x) { pk_AnyStr s; s.type = 1; s._int = x; return s; }
PK_INLINE pk_AnyStr pk_AnyStr__i64(int64_t x) { pk_AnyStr s; s.type = 2; s._i64 = x; return s; }
PK_INLINE pk_AnyStr pk_AnyStr__float(float x) { pk_AnyStr s; s.type = 3; s._float = x; return s; }
PK_INLINE pk_AnyStr pk_AnyStr__double(double x) { pk_AnyStr s; s.type = 4; s._double = x; return s; }
PK_INLINE pk_AnyStr pk_AnyStr__char(char x) { pk_AnyStr s; s.type = 5; s._char = x; return s; }
PK_INLINE pk_AnyStr pk_AnyStr__str(const pkpy_Str* x) { pk_AnyStr s; s.type = 6; s._str = x; return s; }
PK_INLINE pk_AnyStr pk_AnyStr__sv(c11_string x) { pk_AnyStr s; s.type = 7; s._sv = x; return s; }
PK_INLINE pk_AnyStr pk_AnyStr__cstr(const char* x) { pk_AnyStr s; s.type = 8; s._cstr = x; return s; }
PK_INLINE pk_AnyStr pk_AnyStr__ptr(void* x) { pk_AnyStr s; s.type = 9; s._ptr = x; return s; }

void pk_SStream__ctor(pk_SStream* self);
void pk_SStream__ctor2(pk_SStream* self, int capacity);
void pk_SStream__dtor(pk_SStream* self);

void pk_SStream__write_int(pk_SStream* self, int);
void pk_SStream__write_i64(pk_SStream* self, int64_t);
void pk_SStream__write_float(pk_SStream* self, float, int precision);
void pk_SStream__write_double(pk_SStream* self, double, int precision);
void pk_SStream__write_char(pk_SStream* self, char);
void pk_SStream__write_Str(pk_SStream* self, const pkpy_Str*);
void pk_SStream__write_sv(pk_SStream* self, c11_string);
void pk_SStream__write_cstr(pk_SStream* self, const char*);
void pk_SStream__write_cstrn(pk_SStream* self, const char*, int);
void pk_SStream__write_hex(pk_SStream* self, unsigned char, bool non_zero);
void pk_SStream__write_ptr(pk_SStream* self, void*);

void pk_SStream__write_any(pk_SStream* self, const char* fmt, const pk_AnyStr* args, int n);
const char* pk_format_any(const char* fmt, const pk_AnyStr* args, int n);

// Submit the stream and return the final string. The stream becomes invalid after this call
pkpy_Str pk_SStream__submit(pk_SStream* self);

#define pk__anystr(x) _Generic((x), \
    int: pk_AnyStr__int, \
    int64_t: pk_AnyStr__i64, \
    float: pk_AnyStr__float, \
    double: pk_AnyStr__double, \
    char: pk_AnyStr__char, \
    const pkpy_Str*: pk_AnyStr__str, \
    c11_string: pk_AnyStr__sv, \
    const char*: pk_AnyStr__cstr, \
    void*: pk_AnyStr__ptr \
)(x)

#define pk__anystr_list_1(a) (pk_AnyStr[]){pk__anystr(a)}, 1
#define pk__anystr_list_2(a, b) (pk_AnyStr[]){pk__anystr(a), pk__anystr(b)}, 2
#define pk__anystr_list_3(a, b, c) (pk_AnyStr[]){pk__anystr(a), pk__anystr(b), pk__anystr(c)}, 3
#define pk__anystr_list_4(a, b, c, d) (pk_AnyStr[]){pk__anystr(a), pk__anystr(b), pk__anystr(c), pk__anystr(d)}, 4

#define pk__anystr_list_dispatcher(...) PK_NARGS_SEQ(__VA_ARGS__, pk__anystr_list_4, pk__anystr_list_3, pk__anystr_list_2, pk__anystr_list_1, 0)
#define pk__anystr_list(...) pk__anystr_list_dispatcher(__VA_ARGS__)(__VA_ARGS__) 

#define pk_SStream__write(self, fmt, ...) pk_SStream__write_any(self, fmt, pk__anystr_list(__VA_ARGS__))
#define pk_format(fmt, ...) pk_format_any(fmt, pk__anystr_list(__VA_ARGS__))

#ifdef __cplusplus
}
#endif
