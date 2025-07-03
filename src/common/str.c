#include "pocketpy/common/str.h"
#include "pocketpy/common/sstream.h"
#include "pocketpy/common/utils.h"

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

c11_string* c11_string__new(const char* data) { return c11_string__new2(data, strlen(data)); }

c11_string* c11_string__new2(const char* data, int size) {
    c11_string* retval = PK_MALLOC(sizeof(c11_string) + size + 1);
    c11_string__ctor2(retval, data, size);
    return retval;
}

c11_string* c11_string__new3(const char* fmt, ...) {
    c11_sbuf buf;
    c11_sbuf__ctor(&buf);
    va_list args;
    va_start(args, fmt);
    // c11_sbuf__write_vfmt(&buf, fmt, args);
    pk_vsprintf(&buf, fmt, args);
    va_end(args);
    return c11_sbuf__submit(&buf);
}

void c11_string__ctor(c11_string* self, const char* data) {
    c11_string__ctor2(self, data, strlen(data));
}

void c11_string__ctor2(c11_string* self, const char* data, int size) {
    self->size = size;
    char* p = (char*)self->data;
    memcpy(p, data, size);
    p[size] = '\0';
}

void c11_string__ctor3(c11_string* self, int size) {
    self->size = size;
    char* p = (char*)self->data;
    p[size] = '\0';
}

c11_string* c11_string__copy(c11_string* self) {
    int total_size = sizeof(c11_string) + self->size + 1;
    c11_string* retval = PK_MALLOC(total_size);
    memcpy(retval, self, total_size);
    return retval;
}

void c11_string__delete(c11_string* self) { PK_FREE(self); }

c11_sv c11_string__sv(c11_string* self) { return (c11_sv){self->data, self->size}; }

c11_string* c11_sv__replace(c11_sv self, char old, char new_) {
    c11_string* retval = c11_string__new2(self.data, self.size);
    char* p = (char*)retval->data;
    for(int i = 0; i < retval->size; i++) {
        if(p[i] == old) p[i] = new_;
    }
    return retval;
}

c11_string* c11_sv__replace2(c11_sv self, c11_sv old, c11_sv new_) {
    c11_sbuf buf;
    c11_sbuf__ctor(&buf);
    int start = 0;
    while(true) {
        int i = c11_sv__index2(self, old, start);
        if(i == -1) break;
        c11_sv tmp = c11_sv__slice2(self, start, i);
        c11_sbuf__write_sv(&buf, tmp);
        c11_sbuf__write_sv(&buf, new_);
        start = i + old.size;
    }
    c11_sv tmp = c11_sv__slice2(self, start, self.size);
    c11_sbuf__write_sv(&buf, tmp);
    return c11_sbuf__submit(&buf);
}

int c11_sv__u8_length(c11_sv sv) { return c11__byte_index_to_unicode(sv.data, sv.size); }

c11_sv c11_sv__u8_getitem(c11_sv sv, int i) {
    i = c11__unicode_index_to_byte(sv.data, i);
    int size = c11__u8_header(sv.data[i], false);
    return c11_sv__slice2(sv, i, i + size);
}

c11_string* c11_sv__u8_slice(c11_sv sv, int start, int stop, int step) {
    c11_sbuf ss;
    c11_sbuf__ctor(&ss);
    assert(step != 0);
    for(int i = start; step > 0 ? i < stop : i > stop; i += step) {
        c11_sv unicode = c11_sv__u8_getitem(sv, i);
        c11_sbuf__write_sv(&ss, unicode);
    }
    return c11_sbuf__submit(&ss);
}

/////////////////////////////////////////
c11_sv c11_sv__slice(c11_sv sv, int start) { return c11_sv__slice2(sv, start, sv.size); }

c11_sv c11_sv__slice2(c11_sv sv, int start, int stop) {
    if(start < 0) start = 0;
    if(stop < start) stop = start;
    if(stop > sv.size) stop = sv.size;
    return (c11_sv){sv.data + start, stop - start};
}

c11_sv c11_sv__strip(c11_sv sv, c11_sv chars, bool left, bool right) {
    int L = 0;
    int R = c11_sv__u8_length(sv);
    if(left) {
        while(L < R) {
            c11_sv tmp = c11_sv__u8_getitem(sv, L);
            bool found = c11_sv__index2(chars, tmp, 0) != -1;
            if(!found) break;
            L++;
        }
    }
    if(right) {
        while(L < R) {
            c11_sv tmp = c11_sv__u8_getitem(sv, R - 1);
            bool found = c11_sv__index2(chars, tmp, 0) != -1;
            if(!found) break;
            R--;
        }
    }
    int start = c11__unicode_index_to_byte(sv.data, L);
    int stop = c11__unicode_index_to_byte(sv.data, R);
    return c11_sv__slice2(sv, start, stop);
}

int c11_sv__index(c11_sv self, char c) {
    for(int i = 0; i < self.size; i++) {
        if(self.data[i] == c) return i;
    }
    return -1;
}

int c11_sv__rindex(c11_sv self, char c) {
    for(int i = self.size - 1; i >= 0; i--) {
        if(self.data[i] == c) return i;
    }
    return -1;
}

int c11_sv__index2(c11_sv self, c11_sv sub, int start) {
    if(sub.size == 0) return start;
    int max_end = self.size - sub.size;
    for(int i = start; i <= max_end; i++) {
        int res = memcmp(self.data + i, sub.data, sub.size);
        if(res == 0) return i;
    }
    return -1;
}

int c11_sv__count(c11_sv self, c11_sv sub) {
    if(sub.size == 0) return self.size + 1;
    int cnt = 0;
    int start = 0;
    while(true) {
        int i = c11_sv__index2(self, sub, start);
        if(i == -1) break;
        cnt++;
        start = i + sub.size;
    }
    return cnt;
}

bool c11_sv__startswith(c11_sv self, c11_sv prefix) {
    if(prefix.size > self.size) return false;
    return memcmp(self.data, prefix.data, prefix.size) == 0;
}

bool c11_sv__endswith(c11_sv self, c11_sv suffix) {
    if(suffix.size > self.size) return false;
    return memcmp(self.data + self.size - suffix.size, suffix.data, suffix.size) == 0;
}

uint64_t c11_sv__hash(c11_sv self) {
    uint64_t hash = 5381;
    for(int i = 0; i < self.size; i++) {
        // hash * 33 + c
        hash = ((hash << 5) + hash) + (unsigned char)self.data[i];
    }
    return hash;
}

c11_vector /* T=c11_sv */ c11_sv__splitwhitespace(c11_sv self) {
    c11_vector retval;
    c11_vector__ctor(&retval, sizeof(c11_sv));
    const char* data = self.data;
    int i = 0;
    for(int j = 0; j < self.size; j++) {
        if(isspace(data[j])) {
            assert(j >= i);
            c11_sv tmp = {data + i, j - i};
            c11_vector__push(c11_sv, &retval, tmp);
            i = j + 1;
        }
    }
    if(i <= self.size) {
        c11_sv tmp = {data + i, self.size - i};
        c11_vector__push(c11_sv, &retval, tmp);
    }
    return retval;
}

c11_vector /* T=c11_sv */ c11_sv__split(c11_sv self, char sep) {
    c11_vector retval;
    c11_vector__ctor(&retval, sizeof(c11_sv));
    const char* data = self.data;
    int i = 0;
    for(int j = 0; j < self.size; j++) {
        if(data[j] == sep) {
            assert(j >= i);
            c11_sv tmp = {data + i, j - i};
            c11_vector__push(c11_sv, &retval, tmp);
            i = j + 1;
        }
    }
    if(i <= self.size) {
        c11_sv tmp = {data + i, self.size - i};
        c11_vector__push(c11_sv, &retval, tmp);
    }
    return retval;
}

c11_vector /* T=c11_sv */ c11_sv__split2(c11_sv self, c11_sv sep) {
    if(sep.size == 1) return c11_sv__split(self, sep.data[0]);
    c11_vector retval;
    c11_vector__ctor(&retval, sizeof(c11_sv));
    int start = 0;
    const char* data = self.data;
    while(true) {
        int i = c11_sv__index2(self, sep, start);
        if(i == -1) break;
        c11_sv tmp = {data + start, i - start};
        c11_vector__push(c11_sv, &retval, tmp);
        start = i + sep.size;
    }
    c11_sv tmp = {data + start, self.size - start};
    c11_vector__push(c11_sv, &retval, tmp);
    return retval;
}

int c11__unicode_index_to_byte(const char* data, int i) {
    int j = 0;
    while(i > 0) {
        j += c11__u8_header(data[j], false);
        i--;
    }
    return j;
}

int c11__byte_index_to_unicode(const char* data, int n) {
    int cnt = 0;
    for(int i = 0; i < n; i++) {
        if((data[i] & 0xC0) != 0x80) cnt++;
    }
    return cnt;
}

//////////////
bool c11_bytes__eq(c11_bytes* self, c11_bytes* other) {
    if(self->size != other->size) return false;
    return memcmp(self->data, other->data, self->size) == 0;
}

int c11_sv__cmp(c11_sv self, c11_sv other) {
    int res = strncmp(self.data, other.data, c11__min(self.size, other.size));
    if(res != 0) return res;
    return self.size - other.size;
}

int c11_sv__cmp2(c11_sv self, const char* other) {
    int size = strlen(other);
    int res = strncmp(self.data, other, c11__min(self.size, size));
    if(res != 0) return res;
    return self.size - size;
}

bool c11__streq(const char* a, const char* b) { return strcmp(a, b) == 0; }

bool c11__sveq(c11_sv a, c11_sv b) {
    if(a.size != b.size) return false;
    return memcmp(a.data, b.data, a.size) == 0;
}

bool c11__sveq2(c11_sv a, const char* b) {
    int size = strlen(b);
    if(a.size != size) return false;
    return memcmp(a.data, b, size) == 0;
}

int c11__u8_header(unsigned char c, bool suppress) {
    if((c & 0b10000000) == 0) return 1;
    if((c & 0b11100000) == 0b11000000) return 2;
    if((c & 0b11110000) == 0b11100000) return 3;
    if((c & 0b11111000) == 0b11110000) return 4;
    if((c & 0b11111100) == 0b11111000) return 5;
    if((c & 0b11111110) == 0b11111100) return 6;
    if(!suppress) c11__abort("invalid utf8 char");
    return 0;
}

int c11__u8_value(int u8bytes, const char* data) {
    assert(u8bytes != 0);
    if(u8bytes == 1) return (int)data[0];
    uint32_t value = 0;
    for(int k = 0; k < u8bytes; k++) {
        uint8_t b = data[k];
        if(k == 0) {
            if(u8bytes == 2)
                value = (b & 0b00011111) << 6;
            else if(u8bytes == 3)
                value = (b & 0b00001111) << 12;
            else if(u8bytes == 4)
                value = (b & 0b00000111) << 18;
        } else {
            value |= (b & 0b00111111) << (6 * (u8bytes - k - 1));
        }
    }
    return (int)value;
}

int c11__u32_to_u8(uint32_t utf32_char, char utf8_output[4]) {
    int length = 0;

    if(utf32_char <= 0x7F) {
        // 1-byte UTF-8
        utf8_output[0] = (char)utf32_char;
        length = 1;
    } else if(utf32_char <= 0x7FF) {
        // 2-byte UTF-8
        utf8_output[0] = (char)(0xC0 | ((utf32_char >> 6) & 0x1F));
        utf8_output[1] = (char)(0x80 | (utf32_char & 0x3F));
        length = 2;
    } else if(utf32_char <= 0xFFFF) {
        // 3-byte UTF-8
        utf8_output[0] = (char)(0xE0 | ((utf32_char >> 12) & 0x0F));
        utf8_output[1] = (char)(0x80 | ((utf32_char >> 6) & 0x3F));
        utf8_output[2] = (char)(0x80 | (utf32_char & 0x3F));
        length = 3;
    } else if(utf32_char <= 0x10FFFF) {
        // 4-byte UTF-8
        utf8_output[0] = (char)(0xF0 | ((utf32_char >> 18) & 0x07));
        utf8_output[1] = (char)(0x80 | ((utf32_char >> 12) & 0x3F));
        utf8_output[2] = (char)(0x80 | ((utf32_char >> 6) & 0x3F));
        utf8_output[3] = (char)(0x80 | (utf32_char & 0x3F));
        length = 4;
    } else {
        // Invalid UTF-32 character
        return -1;
    }
    return length;
}

char* c11_strdup(const char* str) {
    int len = strlen(str);
    char* dst = PK_MALLOC(len + 1);
    memcpy(dst, str, len);
    dst[len] = '\0';
    return dst;
}

unsigned char* c11_memdup(const unsigned char* src, int size) {
    unsigned char* dst = PK_MALLOC(size);
    memcpy(dst, src, size);
    return dst;
}

IntParsingResult c11__parse_uint(c11_sv text, int64_t* out, int base) {
    *out = 0;

    c11_sv prefix = {.data = text.data, .size = c11__min(2, text.size)};
    if(base == -1) {
        if(c11__sveq2(prefix, "0b"))
            base = 2;
        else if(c11__sveq2(prefix, "0o"))
            base = 8;
        else if(c11__sveq2(prefix, "0x"))
            base = 16;
        else
            base = 10;
    }

    if(base == 10) {
        // 10-base  12334
        if(text.size == 0) return IntParsing_FAILURE;
        for(int i = 0; i < text.size; i++) {
            char c = text.data[i];
            if(c >= '0' && c <= '9') {
                *out = (*out * 10) + (c - '0');
            } else {
                return IntParsing_FAILURE;
            }
        }
        // "9223372036854775807".__len__() == 19
        if(text.size > 19) return IntParsing_OVERFLOW;
        return IntParsing_SUCCESS;
    } else if(base == 2) {
        // 2-base   0b101010
        if(c11__sveq2(prefix, "0b")) {
            // text.remove_prefix(2);
            text = (c11_sv){text.data + 2, text.size - 2};
        }
        if(text.size == 0) return IntParsing_FAILURE;
        for(int i = 0; i < text.size; i++) {
            char c = text.data[i];
            if(c == '0' || c == '1') {
                *out = (*out << 1) | (c - '0');
            } else {
                return IntParsing_FAILURE;
            }
        }
        // "111111111111111111111111111111111111111111111111111111111111111".__len__() == 63
        if(text.size > 63) return IntParsing_OVERFLOW;
        return IntParsing_SUCCESS;
    } else if(base == 8) {
        // 8-base   0o123
        if(c11__sveq2(prefix, "0o")) {
            // text.remove_prefix(2);
            text = (c11_sv){text.data + 2, text.size - 2};
        }
        if(text.size == 0) return IntParsing_FAILURE;
        for(int i = 0; i < text.size; i++) {
            char c = text.data[i];
            if(c >= '0' && c <= '7') {
                *out = (*out << 3) | (c - '0');
            } else {
                return IntParsing_FAILURE;
            }
        }
        // "777777777777777777777".__len__() == 21
        if(text.size > 21) return IntParsing_OVERFLOW;
        return IntParsing_SUCCESS;
    } else if(base == 16) {
        // 16-base  0x123
        if(c11__sveq2(prefix, "0x")) {
            // text.remove_prefix(2);
            text = (c11_sv){text.data + 2, text.size - 2};
        }
        if(text.size == 0) return IntParsing_FAILURE;
        for(int i = 0; i < text.size; i++) {
            char c = text.data[i];
            if(c >= '0' && c <= '9') {
                *out = (*out << 4) | (c - '0');
            } else if(c >= 'a' && c <= 'f') {
                *out = (*out << 4) | (c - 'a' + 10);
            } else if(c >= 'A' && c <= 'F') {
                *out = (*out << 4) | (c - 'A' + 10);
            } else {
                return IntParsing_FAILURE;
            }
        }
        // "7fffffffffffffff".__len__() == 16
        if(text.size > 16) return IntParsing_OVERFLOW;
        return IntParsing_SUCCESS;
    }
    return IntParsing_FAILURE;
}

const char* c11__search_u32_ranges(int c, const c11_u32_range* p, int n_ranges) {
    int lbound = 0;
    int ubound = n_ranges - 1;

    if(c < p[0].start || c > p[ubound].end) return NULL;
    while(ubound >= lbound) {
        int mid = (lbound + ubound) / 2;
        if(c > p[mid].end) {
            lbound = mid + 1;
        } else if(c < p[mid].start) {
            ubound = mid - 1;
        } else {
            return p[mid].data;
        }
    }
    return NULL;
}

const static c11_u32_range kLoRanges[] = {
    {170,    170   },
    {186,    186   },
    {443,    443   },
    {448,    451   },
    {660,    660   },
    {1488,   1514  },
    {1519,   1522  },
    {1568,   1599  },
    {1601,   1610  },
    {1646,   1647  },
    {1649,   1747  },
    {1749,   1749  },
    {1774,   1775  },
    {1786,   1788  },
    {1791,   1791  },
    {1808,   1808  },
    {1810,   1839  },
    {1869,   1957  },
    {1969,   1969  },
    {1994,   2026  },
    {2048,   2069  },
    {2112,   2136  },
    {2144,   2154  },
    {2160,   2183  },
    {2185,   2190  },
    {2208,   2248  },
    {2308,   2361  },
    {2365,   2365  },
    {2384,   2384  },
    {2392,   2401  },
    {2418,   2432  },
    {2437,   2444  },
    {2447,   2448  },
    {2451,   2472  },
    {2474,   2480  },
    {2482,   2482  },
    {2486,   2489  },
    {2493,   2493  },
    {2510,   2510  },
    {2524,   2525  },
    {2527,   2529  },
    {2544,   2545  },
    {2556,   2556  },
    {2565,   2570  },
    {2575,   2576  },
    {2579,   2600  },
    {2602,   2608  },
    {2610,   2611  },
    {2613,   2614  },
    {2616,   2617  },
    {2649,   2652  },
    {2654,   2654  },
    {2674,   2676  },
    {2693,   2701  },
    {2703,   2705  },
    {2707,   2728  },
    {2730,   2736  },
    {2738,   2739  },
    {2741,   2745  },
    {2749,   2749  },
    {2768,   2768  },
    {2784,   2785  },
    {2809,   2809  },
    {2821,   2828  },
    {2831,   2832  },
    {2835,   2856  },
    {2858,   2864  },
    {2866,   2867  },
    {2869,   2873  },
    {2877,   2877  },
    {2908,   2909  },
    {2911,   2913  },
    {2929,   2929  },
    {2947,   2947  },
    {2949,   2954  },
    {2958,   2960  },
    {2962,   2965  },
    {2969,   2970  },
    {2972,   2972  },
    {2974,   2975  },
    {2979,   2980  },
    {2984,   2986  },
    {2990,   3001  },
    {3024,   3024  },
    {3077,   3084  },
    {3086,   3088  },
    {3090,   3112  },
    {3114,   3129  },
    {3133,   3133  },
    {3160,   3162  },
    {3165,   3165  },
    {3168,   3169  },
    {3200,   3200  },
    {3205,   3212  },
    {3214,   3216  },
    {3218,   3240  },
    {3242,   3251  },
    {3253,   3257  },
    {3261,   3261  },
    {3293,   3294  },
    {3296,   3297  },
    {3313,   3314  },
    {3332,   3340  },
    {3342,   3344  },
    {3346,   3386  },
    {3389,   3389  },
    {3406,   3406  },
    {3412,   3414  },
    {3423,   3425  },
    {3450,   3455  },
    {3461,   3478  },
    {3482,   3505  },
    {3507,   3515  },
    {3517,   3517  },
    {3520,   3526  },
    {3585,   3632  },
    {3634,   3635  },
    {3648,   3653  },
    {3713,   3714  },
    {3716,   3716  },
    {3718,   3722  },
    {3724,   3747  },
    {3749,   3749  },
    {3751,   3760  },
    {3762,   3763  },
    {3773,   3773  },
    {3776,   3780  },
    {3804,   3807  },
    {3840,   3840  },
    {3904,   3911  },
    {3913,   3948  },
    {3976,   3980  },
    {4096,   4138  },
    {4159,   4159  },
    {4176,   4181  },
    {4186,   4189  },
    {4193,   4193  },
    {4197,   4198  },
    {4206,   4208  },
    {4213,   4225  },
    {4238,   4238  },
    {4352,   4680  },
    {4682,   4685  },
    {4688,   4694  },
    {4696,   4696  },
    {4698,   4701  },
    {4704,   4744  },
    {4746,   4749  },
    {4752,   4784  },
    {4786,   4789  },
    {4792,   4798  },
    {4800,   4800  },
    {4802,   4805  },
    {4808,   4822  },
    {4824,   4880  },
    {4882,   4885  },
    {4888,   4954  },
    {4992,   5007  },
    {5121,   5740  },
    {5743,   5759  },
    {5761,   5786  },
    {5792,   5866  },
    {5873,   5880  },
    {5888,   5905  },
    {5919,   5937  },
    {5952,   5969  },
    {5984,   5996  },
    {5998,   6000  },
    {6016,   6067  },
    {6108,   6108  },
    {6176,   6210  },
    {6212,   6264  },
    {6272,   6276  },
    {6279,   6312  },
    {6314,   6314  },
    {6320,   6389  },
    {6400,   6430  },
    {6480,   6509  },
    {6512,   6516  },
    {6528,   6571  },
    {6576,   6601  },
    {6656,   6678  },
    {6688,   6740  },
    {6917,   6963  },
    {6981,   6988  },
    {7043,   7072  },
    {7086,   7087  },
    {7098,   7141  },
    {7168,   7203  },
    {7245,   7247  },
    {7258,   7287  },
    {7401,   7404  },
    {7406,   7411  },
    {7413,   7414  },
    {7418,   7418  },
    {8501,   8504  },
    {11568,  11623 },
    {11648,  11670 },
    {11680,  11686 },
    {11688,  11694 },
    {11696,  11702 },
    {11704,  11710 },
    {11712,  11718 },
    {11720,  11726 },
    {11728,  11734 },
    {11736,  11742 },
    {12294,  12294 },
    {12348,  12348 },
    {12353,  12438 },
    {12447,  12447 },
    {12449,  12538 },
    {12543,  12543 },
    {12549,  12591 },
    {12593,  12686 },
    {12704,  12735 },
    {12784,  12799 },
    {13312,  19903 },
    {19968,  40980 },
    {40982,  42124 },
    {42192,  42231 },
    {42240,  42507 },
    {42512,  42527 },
    {42538,  42539 },
    {42606,  42606 },
    {42656,  42725 },
    {42895,  42895 },
    {42999,  42999 },
    {43003,  43009 },
    {43011,  43013 },
    {43015,  43018 },
    {43020,  43042 },
    {43072,  43123 },
    {43138,  43187 },
    {43250,  43255 },
    {43259,  43259 },
    {43261,  43262 },
    {43274,  43301 },
    {43312,  43334 },
    {43360,  43388 },
    {43396,  43442 },
    {43488,  43492 },
    {43495,  43503 },
    {43514,  43518 },
    {43520,  43560 },
    {43584,  43586 },
    {43588,  43595 },
    {43616,  43631 },
    {43633,  43638 },
    {43642,  43642 },
    {43646,  43695 },
    {43697,  43697 },
    {43701,  43702 },
    {43705,  43709 },
    {43712,  43712 },
    {43714,  43714 },
    {43739,  43740 },
    {43744,  43754 },
    {43762,  43762 },
    {43777,  43782 },
    {43785,  43790 },
    {43793,  43798 },
    {43808,  43814 },
    {43816,  43822 },
    {43968,  44002 },
    {44032,  55203 },
    {55216,  55238 },
    {55243,  55291 },
    {63744,  64109 },
    {64112,  64217 },
    {64285,  64285 },
    {64287,  64296 },
    {64298,  64310 },
    {64312,  64316 },
    {64318,  64318 },
    {64320,  64321 },
    {64323,  64324 },
    {64326,  64433 },
    {64467,  64829 },
    {64848,  64911 },
    {64914,  64967 },
    {65008,  65019 },
    {65136,  65140 },
    {65142,  65276 },
    {65382,  65391 },
    {65393,  65437 },
    {65440,  65470 },
    {65474,  65479 },
    {65482,  65487 },
    {65490,  65495 },
    {65498,  65500 },
    {65536,  65547 },
    {65549,  65574 },
    {65576,  65594 },
    {65596,  65597 },
    {65599,  65613 },
    {65616,  65629 },
    {65664,  65786 },
    {66176,  66204 },
    {66208,  66256 },
    {66304,  66335 },
    {66349,  66368 },
    {66370,  66377 },
    {66384,  66421 },
    {66432,  66461 },
    {66464,  66499 },
    {66504,  66511 },
    {66640,  66717 },
    {66816,  66855 },
    {66864,  66915 },
    {67072,  67382 },
    {67392,  67413 },
    {67424,  67431 },
    {67584,  67589 },
    {67592,  67592 },
    {67594,  67637 },
    {67639,  67640 },
    {67644,  67644 },
    {67647,  67669 },
    {67680,  67702 },
    {67712,  67742 },
    {67808,  67826 },
    {67828,  67829 },
    {67840,  67861 },
    {67872,  67897 },
    {67968,  68023 },
    {68030,  68031 },
    {68096,  68096 },
    {68112,  68115 },
    {68117,  68119 },
    {68121,  68149 },
    {68192,  68220 },
    {68224,  68252 },
    {68288,  68295 },
    {68297,  68324 },
    {68352,  68405 },
    {68416,  68437 },
    {68448,  68466 },
    {68480,  68497 },
    {68608,  68680 },
    {68864,  68899 },
    {69248,  69289 },
    {69296,  69297 },
    {69376,  69404 },
    {69415,  69415 },
    {69424,  69445 },
    {69488,  69505 },
    {69552,  69572 },
    {69600,  69622 },
    {69635,  69687 },
    {69745,  69746 },
    {69749,  69749 },
    {69763,  69807 },
    {69840,  69864 },
    {69891,  69926 },
    {69956,  69956 },
    {69959,  69959 },
    {69968,  70002 },
    {70006,  70006 },
    {70019,  70066 },
    {70081,  70084 },
    {70106,  70106 },
    {70108,  70108 },
    {70144,  70161 },
    {70163,  70187 },
    {70272,  70278 },
    {70280,  70280 },
    {70282,  70285 },
    {70287,  70301 },
    {70303,  70312 },
    {70320,  70366 },
    {70405,  70412 },
    {70415,  70416 },
    {70419,  70440 },
    {70442,  70448 },
    {70450,  70451 },
    {70453,  70457 },
    {70461,  70461 },
    {70480,  70480 },
    {70493,  70497 },
    {70656,  70708 },
    {70727,  70730 },
    {70751,  70753 },
    {70784,  70831 },
    {70852,  70853 },
    {70855,  70855 },
    {71040,  71086 },
    {71128,  71131 },
    {71168,  71215 },
    {71236,  71236 },
    {71296,  71338 },
    {71352,  71352 },
    {71424,  71450 },
    {71488,  71494 },
    {71680,  71723 },
    {71935,  71942 },
    {71945,  71945 },
    {71948,  71955 },
    {71957,  71958 },
    {71960,  71983 },
    {71999,  71999 },
    {72001,  72001 },
    {72096,  72103 },
    {72106,  72144 },
    {72161,  72161 },
    {72163,  72163 },
    {72192,  72192 },
    {72203,  72242 },
    {72250,  72250 },
    {72272,  72272 },
    {72284,  72329 },
    {72349,  72349 },
    {72368,  72440 },
    {72704,  72712 },
    {72714,  72750 },
    {72768,  72768 },
    {72818,  72847 },
    {72960,  72966 },
    {72968,  72969 },
    {72971,  73008 },
    {73030,  73030 },
    {73056,  73061 },
    {73063,  73064 },
    {73066,  73097 },
    {73112,  73112 },
    {73440,  73458 },
    {73648,  73648 },
    {73728,  74649 },
    {74880,  75075 },
    {77712,  77808 },
    {77824,  78894 },
    {82944,  83526 },
    {92160,  92728 },
    {92736,  92766 },
    {92784,  92862 },
    {92880,  92909 },
    {92928,  92975 },
    {93027,  93047 },
    {93053,  93071 },
    {93952,  94026 },
    {94032,  94032 },
    {94208,  100343},
    {100352, 101589},
    {101632, 101640},
    {110592, 110882},
    {110928, 110930},
    {110948, 110951},
    {110960, 111355},
    {113664, 113770},
    {113776, 113788},
    {113792, 113800},
    {113808, 113817},
    {122634, 122634},
    {123136, 123180},
    {123214, 123214},
    {123536, 123565},
    {123584, 123627},
    {124896, 124902},
    {124904, 124907},
    {124909, 124910},
    {124912, 124926},
    {124928, 125124},
    {126464, 126467},
    {126469, 126495},
    {126497, 126498},
    {126500, 126500},
    {126503, 126503},
    {126505, 126514},
    {126516, 126519},
    {126521, 126521},
    {126523, 126523},
    {126530, 126530},
    {126535, 126535},
    {126537, 126537},
    {126539, 126539},
    {126541, 126543},
    {126545, 126546},
    {126548, 126548},
    {126551, 126551},
    {126553, 126553},
    {126555, 126555},
    {126557, 126557},
    {126559, 126559},
    {126561, 126562},
    {126564, 126564},
    {126567, 126570},
    {126572, 126578},
    {126580, 126583},
    {126585, 126588},
    {126590, 126590},
    {126592, 126601},
    {126603, 126619},
    {126625, 126627},
    {126629, 126633},
    {126635, 126651},
    {131072, 173791},
    {173824, 177976},
    {177984, 178205},
    {178208, 183969},
    {183984, 191456},
    {194560, 195101},
    {196608, 201546},
};

bool c11__is_unicode_Lo_char(int c) {
    if(c == 0x1f955) return true;
    const char* data =
        c11__search_u32_ranges(c, kLoRanges, sizeof(kLoRanges) / sizeof(c11_u32_range));
    return data != NULL;
}
