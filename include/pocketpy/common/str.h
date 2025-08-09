#pragma once

#include "pocketpy/common/vector.h"
#include "pocketpy/common/utils.h"
#include "pocketpy/pocketpy.h"

#include <stdarg.h>

/* string */
typedef struct c11_string {
    // int size | char[] | '\0'
    int size;
    char data[];  // flexible array member
} c11_string;

c11_string* pk_tostr(py_Ref self);

/* bytes */
typedef struct c11_bytes {
    int size;
    unsigned char data[];  // flexible array member
} c11_bytes;

typedef struct {
    int start;
    int end;
    char data[4];
} c11_u32_range;

bool c11_bytes__eq(c11_bytes* self, c11_bytes* other);

int c11_sv__cmp(c11_sv self, c11_sv other);
int c11_sv__cmp2(c11_sv self, const char* other);

bool c11__streq(const char* a, const char* b);
bool c11__sveq(c11_sv a, c11_sv b);
bool c11__sveq2(c11_sv a, const char* b);

c11_string* c11_string__new(const char* data);
c11_string* c11_string__new2(const char* data, int size);
c11_string* c11_string__new3(const char* fmt, ...);
void c11_string__ctor(c11_string* self, const char* data);
void c11_string__ctor2(c11_string* self, const char* data, int size);
void c11_string__ctor3(c11_string* self, int size);
c11_string* c11_string__copy(c11_string* self);
void c11_string__delete(c11_string* self);
c11_sv c11_string__sv(c11_string* self);

int c11_sv__u8_length(c11_sv self);
c11_sv c11_sv__u8_getitem(c11_sv self, int i);
c11_string* c11_sv__u8_slice(c11_sv self, int start, int stop, int step);

// general string operations
c11_sv c11_sv__slice(c11_sv sv, int start);
c11_sv c11_sv__slice2(c11_sv sv, int start, int stop);
c11_sv c11_sv__strip(c11_sv sv, c11_sv chars, bool left, bool right);
int c11_sv__index(c11_sv self, char c);
int c11_sv__rindex(c11_sv self, char c);
int c11_sv__index2(c11_sv self, c11_sv sub, int start);
int c11_sv__count(c11_sv self, c11_sv sub);
bool c11_sv__startswith(c11_sv self, c11_sv prefix);
bool c11_sv__endswith(c11_sv self, c11_sv suffix);
uint64_t c11_sv__hash(c11_sv self);

c11_string* c11_sv__replace(c11_sv self, char old, char new_);
c11_string* c11_sv__replace2(c11_sv self, c11_sv old, c11_sv new_);

c11_vector /* T=c11_sv */ c11_sv__split(c11_sv self, char sep);
c11_vector /* T=c11_sv */ c11_sv__split2(c11_sv self, c11_sv sep);
c11_vector /* T=c11_sv */ c11_sv__splitwhitespace(c11_sv self);

// misc
int c11__unicode_index_to_byte(const char* data, int i);
int c11__byte_index_to_unicode(const char* data, int n);

bool c11__is_unicode_Lo_char(int c);
const char* c11__search_u32_ranges(int c, const c11_u32_range* p, int n_ranges);
int c11__u8_header(unsigned char c, bool suppress);
int c11__u8_value(int u8bytes, const char* data);
int c11__u32_to_u8(uint32_t utf32_char, char utf8_output[4]);

char* c11_strdup(const char* str);
unsigned char* c11_memdup(const unsigned char* data, int size);

typedef enum IntParsingResult {
    IntParsing_SUCCESS,
    IntParsing_FAILURE,
    IntParsing_OVERFLOW,
} IntParsingResult;

IntParsingResult c11__parse_uint(c11_sv text, int64_t* out, int base);
