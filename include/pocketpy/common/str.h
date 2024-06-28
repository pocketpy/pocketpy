#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "pocketpy/common/vector.h"
#include "pocketpy/common/utils.h"

#ifdef __cplusplus
extern "C" {
#endif

/* string_view */
typedef struct c11_string{
    const char* data;
    int size;
} c11_string;

int c11_string__cmp(c11_string self, c11_string other);
int c11_string__cmp2(c11_string self, const char* other, int size);
int c11_string__cmp3(c11_string self, const char* other);
int c11_string__index(c11_string self, char c);

typedef struct py_Str{
    int size;
    bool is_ascii;
    bool is_sso;
    union{
        char* _ptr;
        char _inlined[16];
    };
} py_Str;

PK_INLINE const char* py_Str__data(const py_Str* self){
    return self->is_sso ? self->_inlined : self->_ptr;
}

PK_INLINE c11_string py_Str__sv(const py_Str* self){
    c11_string retval;
    retval.data = py_Str__data(self);
    retval.size = self->size;
    return retval;
}

void py_Str__ctor(py_Str* self, const char* data);
void py_Str__ctor2(py_Str* self, const char* data, int size);
void py_Str__dtor(py_Str* self);
py_Str py_Str__copy(const py_Str* self);
py_Str py_Str__concat(const py_Str* self, const py_Str* other);
py_Str py_Str__concat2(const py_Str* self, const char* other, int size);
py_Str py_Str__slice(const py_Str* self, int start);
py_Str py_Str__slice2(const py_Str* self, int start, int stop);
py_Str py_Str__lower(const py_Str* self);
py_Str py_Str__upper(const py_Str* self);
py_Str py_Str__escape(const py_Str* self, char quote);
py_Str py_Str__strip(const py_Str* self, bool left, bool right);
py_Str py_Str__strip2(const py_Str* self, bool left, bool right, const py_Str* chars);
py_Str py_Str__replace(const py_Str* self, char old, char new_);
py_Str py_Str__replace2(const py_Str* self, const py_Str* old, const py_Str* new_);
py_Str py_Str__u8_getitem(const py_Str* self, int i);
py_Str py_Str__u8_slice(const py_Str* self, int start, int stop, int step);
int py_Str__u8_length(const py_Str* self);
int py_Str__cmp(const py_Str* self, const py_Str* other);
int py_Str__cmp2(const py_Str* self, const char* other, int size);
int py_Str__cmp3(const py_Str* self, const char* other);
int py_Str__unicode_index_to_byte(const py_Str* self, int i);
int py_Str__byte_index_to_unicode(const py_Str* self, int n);
int py_Str__index(const py_Str* self, const py_Str* sub, int start);
int py_Str__count(const py_Str* self, const py_Str* sub);
c11_vector/* T=c11_string */ py_Str__split(const py_Str* self, char sep);
c11_vector/* T=c11_string */ py_Str__split2(const py_Str* self, const py_Str* sep);

bool c11__isascii(const char* p, int size);
bool c11__is_unicode_Lo_char(int c);
int c11__u8_header(unsigned char c, bool suppress);

#ifdef __cplusplus
}
#endif