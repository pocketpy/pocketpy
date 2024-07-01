#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "pocketpy/common/vector.h"
#include "pocketpy/common/utils.h"

#ifdef __cplusplus
extern "C" {
#endif

/* string_view */
typedef struct c11_sv{
    const char* data;
    int size;
} c11_sv;

int c11_sv__cmp(c11_sv self, c11_sv other);
int c11_sv__cmp2(c11_sv self, const char* other, int size);
int c11_sv__cmp3(c11_sv self, const char* other);

// int size | char[] | '\0'
typedef const char c11_string;

c11_string* c11_string__new(const char* data);
c11_string* c11_string__new2(const char* data, int size);
c11_string* c11_string__copy(c11_string* self);
void c11_string__delete(c11_string* self);
int c11_string__len(c11_string* self);
c11_sv c11_string__sv(c11_string* self);
c11_string* c11_string__replace(c11_string* self, char old, char new_);

int c11_string__u8_length(const c11_string* self);
c11_sv c11_string__u8_getitem(c11_string* self, int i);
c11_string* c11_string__u8_slice(c11_string* self, int start, int stop, int step);

// general string operations
void c11_sv__quote(c11_sv sv, char quote, c11_vector* buf);
void c11_sv__lower(c11_sv sv, c11_vector* buf);
void c11_sv__upper(c11_sv sv, c11_vector* buf);
c11_sv c11_sv__slice(c11_sv sv, int start);
c11_sv c11_sv__slice2(c11_sv sv, int start, int stop);
c11_sv c11_sv__strip(c11_sv sv, bool left, bool right);
int c11_sv__index(c11_sv self, char c);
int c11_sv__index2(c11_sv self, c11_sv sub, int start);
int c11_sv__count(c11_sv self, c11_sv sub);

c11_vector/* T=c11_sv */ c11_sv__split(c11_sv self, char sep);
c11_vector/* T=c11_sv */ c11_sv__split2(c11_sv self, c11_sv sep);

// misc
int c11__unicode_index_to_byte(const char* data, int i);
int c11__byte_index_to_unicode(const char* data, int n);

bool c11__is_unicode_Lo_char(int c);
int c11__u8_header(unsigned char c, bool suppress);

#ifdef __cplusplus
}
#endif