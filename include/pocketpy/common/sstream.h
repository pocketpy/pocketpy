#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "pocketpy/common/vector.h"
#include "pocketpy/common/str.h"
#include "pocketpy/common/utils.h"

#include <stdint.h>

typedef struct pkpy_SStream {
    c11_vector data;
} pkpy_SStream;

typedef struct pkpy_AnyStr {
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
} pkpy_AnyStr;

inline pkpy_AnyStr pkpy_AnyStr__int(int x) { pkpy_AnyStr s; s.type = 1; s._int = x; return s; }
inline pkpy_AnyStr pkpy_AnyStr__i64(int64_t x) { pkpy_AnyStr s; s.type = 2; s._i64 = x; return s; }
inline pkpy_AnyStr pkpy_AnyStr__float(float x) { pkpy_AnyStr s; s.type = 3; s._float = x; return s; }
inline pkpy_AnyStr pkpy_AnyStr__double(double x) { pkpy_AnyStr s; s.type = 4; s._double = x; return s; }
inline pkpy_AnyStr pkpy_AnyStr__char(char x) { pkpy_AnyStr s; s.type = 5; s._char = x; return s; }
inline pkpy_AnyStr pkpy_AnyStr__str(const pkpy_Str* x) { pkpy_AnyStr s; s.type = 6; s._str = x; return s; }
inline pkpy_AnyStr pkpy_AnyStr__sv(c11_string x) { pkpy_AnyStr s; s.type = 7; s._sv = x; return s; }
inline pkpy_AnyStr pkpy_AnyStr__cstr(const char* x) { pkpy_AnyStr s; s.type = 8; s._cstr = x; return s; }
inline pkpy_AnyStr pkpy_AnyStr__ptr(void* x) { pkpy_AnyStr s; s.type = 9; s._ptr = x; return s; }

void pkpy_SStream__ctor(pkpy_SStream* self);
void pkpy_SStream__dtor(pkpy_SStream* self);

void pkpy_SStream__write_int(pkpy_SStream* self, int);
void pkpy_SStream__write_i64(pkpy_SStream* self, int64_t);
void pkpy_SStream__write_float(pkpy_SStream* self, float, int precision);
void pkpy_SStream__write_double(pkpy_SStream* self, double, int precision);
void pkpy_SStream__write_char(pkpy_SStream* self, char);
void pkpy_SStream__write_Str(pkpy_SStream* self, const pkpy_Str*);
void pkpy_SStream__write_sv(pkpy_SStream* self, c11_string);
void pkpy_SStream__write_cstr(pkpy_SStream* self, const char*);
void pkpy_SStream__write_cstrn(pkpy_SStream* self, const char*, int);
void pkpy_SStream__write_hex(pkpy_SStream* self, unsigned char, bool non_zero);
void pkpy_SStream__write_ptr(pkpy_SStream* self, void*);
void pkpy_SStream__write_any(pkpy_SStream* self, const char* fmt, const pkpy_AnyStr* args, int n);

// Submit the stream and return the final string. The stream becomes invalid after this call
pkpy_Str pkpy_SStream__submit(pkpy_SStream* self);

#define pkpy__anystr(x) _Generic((x), \
    int: pkpy_AnyStr__int, \
    int64_t: pkpy_AnyStr__i64, \
    float: pkpy_AnyStr__float, \
    double: pkpy_AnyStr__double, \
    char: pkpy_AnyStr__char, \
    const pkpy_Str*: pkpy_AnyStr__str, \
    c11_string: pkpy_AnyStr__sv, \
    const char*: pkpy_AnyStr__cstr, \
    void*: pkpy_AnyStr__ptr \
)(x)

#define pkpy__anystr_list_1(a) (pkpy_AnyStr[]){pkpy__anystr(a)}, 1
#define pkpy__anystr_list_2(a, b) (pkpy_AnyStr[]){pkpy__anystr(a), pkpy__anystr(b)}, 2
#define pkpy__anystr_list_3(a, b, c) (pkpy_AnyStr[]){pkpy__anystr(a), pkpy__anystr(b), pkpy__anystr(c)}, 3
#define pkpy__anystr_list_4(a, b, c, d) (pkpy_AnyStr[]){pkpy__anystr(a), pkpy__anystr(b), pkpy__anystr(c), pkpy__anystr(d)}, 4

#define pkpy__anystr_list_dispatcher(...) PK_NARGS_SEQ(__VA_ARGS__, pkpy__anystr_list_4, pkpy__anystr_list_3, pkpy__anystr_list_2, pkpy__anystr_list_1, 0)
#define pkpy__anystr_list(...) pkpy__anystr_list_dispatcher(__VA_ARGS__)(__VA_ARGS__) 

#define pkpy_SStream__write(self, fmt, ...) pkpy_SStream__write_any(self, fmt, pkpy__anystr_list(__VA_ARGS__))

#ifdef __cplusplus
}
#endif
