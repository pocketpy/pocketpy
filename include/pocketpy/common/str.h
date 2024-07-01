#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "pocketpy/common/vector.h"
#include "pocketpy/common/utils.h"

#ifdef __cplusplus
extern "C" {
#endif

/* string_view */
typedef struct c11_stringview{
    const char* data;
    int size;
} c11_stringview;

int c11_string__cmp(c11_stringview self, c11_stringview other);
int c11_string__cmp2(c11_stringview self, const char* other, int size);
int c11_string__cmp3(c11_stringview self, const char* other);

// int size | char[] | '\0'
typedef const char c11_string;

c11_string* c11_string__new(const char* data);
c11_string* c11_string__new2(const char* data, int size);
c11_string* c11_string__copy(c11_string* self);
void c11_string__delete(c11_string* self);
int c11_string__len(c11_string* self);
c11_stringview c11_string__view(c11_string* self);
c11_string* c11_string__replace(c11_string* self, char old, char new_);

int c11_string__u8_length(const c11_string* self);
c11_stringview c11_string__u8_getitem(c11_string* self, int i);
c11_string* c11_string__u8_slice(c11_string* self, int start, int stop, int step);

// general string operations
void c11_sv__quote(c11_stringview sv, char quote, c11_vector* buf);
void c11_sv__lower(c11_stringview sv, c11_vector* buf);
void c11_sv__upper(c11_stringview sv, c11_vector* buf);
c11_stringview c11_sv__slice(c11_stringview sv, int start);
c11_stringview c11_sv__slice2(c11_stringview sv, int start, int stop);
c11_stringview c11_sv__strip(c11_stringview sv, bool left, bool right);
int c11_sv__index(c11_stringview self, char c);
int c11_sv__index2(c11_stringview self, c11_stringview sub, int start);
int c11_sv__count(c11_stringview self, c11_stringview sub);

c11_vector/* T=c11_stringview */ c11_sv__split(c11_stringview self, char sep);
c11_vector/* T=c11_stringview */ c11_sv__split2(c11_stringview self, c11_stringview sep);

// misc
int c11__unicode_index_to_byte(const char* data, int i);
int c11__byte_index_to_unicode(const char* data, int n);

bool c11__is_unicode_Lo_char(int c);
int c11__u8_header(unsigned char c, bool suppress);

#ifdef __cplusplus
}
#endif