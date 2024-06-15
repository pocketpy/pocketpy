#pragma once

#include <stdbool.h>
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

typedef struct pkpy_Str{
    int size;
    bool is_ascii;
    bool is_sso;
    union{
        char* _ptr;
        char _inlined[16];
    };
} pkpy_Str;

PK_INLINE const char* pkpy_Str__data(const pkpy_Str* self){
    return self->is_sso ? self->_inlined : self->_ptr;
}

PK_INLINE c11_string pkpy_Str__sv(const pkpy_Str* self){
    return (c11_string){pkpy_Str__data(self), self->size};
}

void pkpy_Str__ctor(pkpy_Str* self, const char* data);
void pkpy_Str__ctor2(pkpy_Str* self, const char* data, int size);
void pkpy_Str__dtor(pkpy_Str* self);
pkpy_Str pkpy_Str__copy(const pkpy_Str* self);
pkpy_Str pkpy_Str__concat(const pkpy_Str* self, const pkpy_Str* other);
pkpy_Str pkpy_Str__concat2(const pkpy_Str* self, const char* other, int size);
pkpy_Str pkpy_Str__slice(const pkpy_Str* self, int start);
pkpy_Str pkpy_Str__slice2(const pkpy_Str* self, int start, int stop);
pkpy_Str pkpy_Str__lower(const pkpy_Str* self);
pkpy_Str pkpy_Str__upper(const pkpy_Str* self);
pkpy_Str pkpy_Str__escape(const pkpy_Str* self, char quote);
pkpy_Str pkpy_Str__strip(const pkpy_Str* self, bool left, bool right);
pkpy_Str pkpy_Str__strip2(const pkpy_Str* self, bool left, bool right, const pkpy_Str* chars);
pkpy_Str pkpy_Str__replace(const pkpy_Str* self, char old, char new_);
pkpy_Str pkpy_Str__replace2(const pkpy_Str* self, const pkpy_Str* old, const pkpy_Str* new_);
pkpy_Str pkpy_Str__u8_getitem(const pkpy_Str* self, int i);
pkpy_Str pkpy_Str__u8_slice(const pkpy_Str* self, int start, int stop, int step);
int pkpy_Str__u8_length(const pkpy_Str* self);
int pkpy_Str__cmp(const pkpy_Str* self, const pkpy_Str* other);
int pkpy_Str__cmp2(const pkpy_Str* self, const char* other, int size);
int pkpy_Str__cmp3(const pkpy_Str* self, const char* other);
int pkpy_Str__unicode_index_to_byte(const pkpy_Str* self, int i);
int pkpy_Str__byte_index_to_unicode(const pkpy_Str* self, int n);
int pkpy_Str__index(const pkpy_Str* self, const pkpy_Str* sub, int start);
int pkpy_Str__count(const pkpy_Str* self, const pkpy_Str* sub);
c11_vector/* T=c11_string */ pkpy_Str__split(const pkpy_Str* self, char sep);
c11_vector/* T=c11_string */ pkpy_Str__split2(const pkpy_Str* self, const pkpy_Str* sep);

bool c11__isascii(const char* p, int size);
bool c11__is_unicode_Lo_char(int c);
int c11__u8_header(unsigned char c, bool suppress);

#ifdef __cplusplus
}
#endif