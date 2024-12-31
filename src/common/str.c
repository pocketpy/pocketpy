#include "pocketpy/common/str.h"
#include "pocketpy/common/sstream.h"
#include "pocketpy/common/utils.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

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

// clang-format off
static const int kLoRangeA[] = {170,186,443,448,660,1488,1519,1568,1601,1646,1649,1749,1774,1786,1791,1808,1810,1869,1969,1994,2048,2112,2144,2208,2230,2308,2365,2384,2392,2418,2437,2447,2451,2474,2482,2486,2493,2510,2524,2527,2544,2556,2565,2575,2579,2602,2610,2613,2616,2649,2654,2674,2693,2703,2707,2730,2738,2741,2749,2768,2784,2809,2821,2831,2835,2858,2866,2869,2877,2908,2911,2929,2947,2949,2958,2962,2969,2972,2974,2979,2984,2990,3024,3077,3086,3090,3114,3133,3160,3168,3200,3205,3214,3218,3242,3253,3261,3294,3296,3313,3333,3342,3346,3389,3406,3412,3423,3450,3461,3482,3507,3517,3520,3585,3634,3648,3713,3716,3718,3724,3749,3751,3762,3773,3776,3804,3840,3904,3913,3976,4096,4159,4176,4186,4193,4197,4206,4213,4238,4352,4682,4688,4696,4698,4704,4746,4752,4786,4792,4800,4802,4808,4824,4882,4888,4992,5121,5743,5761,5792,5873,5888,5902,5920,5952,5984,5998,6016,6108,6176,6212,6272,6279,6314,6320,6400,6480,6512,6528,6576,6656,6688,6917,6981,7043,7086,7098,7168,7245,7258,7401,7406,7413,7418,8501,11568,11648,11680,11688,11696,11704,11712,11720,11728,11736,12294,12348,12353,12447,12449,12543,12549,12593,12704,12784,13312,19968,40960,40982,42192,42240,42512,42538,42606,42656,42895,42999,43003,43011,43015,43020,43072,43138,43250,43259,43261,43274,43312,43360,43396,43488,43495,43514,43520,43584,43588,43616,43633,43642,43646,43697,43701,43705,43712,43714,43739,43744,43762,43777,43785,43793,43808,43816,43968,44032,55216,55243,63744,64112,64285,64287,64298,64312,64318,64320,64323,64326,64467,64848,64914,65008,65136,65142,65382,65393,65440,65474,65482,65490,65498,65536,65549,65576,65596,65599,65616,65664,66176,66208,66304,66349,66370,66384,66432,66464,66504,66640,66816,66864,67072,67392,67424,67584,67592,67594,67639,67644,67647,67680,67712,67808,67828,67840,67872,67968,68030,68096,68112,68117,68121,68192,68224,68288,68297,68352,68416,68448,68480,68608,68864,69376,69415,69424,69600,69635,69763,69840,69891,69956,69968,70006,70019,70081,70106,70108,70144,70163,70272,70280,70282,70287,70303,70320,70405,70415,70419,70442,70450,70453,70461,70480,70493,70656,70727,70751,70784,70852,70855,71040,71128,71168,71236,71296,71352,71424,71680,71935,72096,72106,72161,72163,72192,72203,72250,72272,72284,72349,72384,72704,72714,72768,72818,72960,72968,72971,73030,73056,73063,73066,73112,73440,73728,74880,77824,82944,92160,92736,92880,92928,93027,93053,93952,94032,94208,100352,110592,110928,110948,110960,113664,113776,113792,113808,123136,123214,123584,124928,126464,126469,126497,126500,126503,126505,126516,126521,126523,126530,126535,126537,126539,126541,126545,126548,126551,126553,126555,126557,126559,126561,126564,126567,126572,126580,126585,126590,126592,126603,126625,126629,126635,131072,173824,177984,178208,183984,194560};
static const int kLoRangeB[] = {170,186,443,451,660,1514,1522,1599,1610,1647,1747,1749,1775,1788,1791,1808,1839,1957,1969,2026,2069,2136,2154,2228,2237,2361,2365,2384,2401,2432,2444,2448,2472,2480,2482,2489,2493,2510,2525,2529,2545,2556,2570,2576,2600,2608,2611,2614,2617,2652,2654,2676,2701,2705,2728,2736,2739,2745,2749,2768,2785,2809,2828,2832,2856,2864,2867,2873,2877,2909,2913,2929,2947,2954,2960,2965,2970,2972,2975,2980,2986,3001,3024,3084,3088,3112,3129,3133,3162,3169,3200,3212,3216,3240,3251,3257,3261,3294,3297,3314,3340,3344,3386,3389,3406,3414,3425,3455,3478,3505,3515,3517,3526,3632,3635,3653,3714,3716,3722,3747,3749,3760,3763,3773,3780,3807,3840,3911,3948,3980,4138,4159,4181,4189,4193,4198,4208,4225,4238,4680,4685,4694,4696,4701,4744,4749,4784,4789,4798,4800,4805,4822,4880,4885,4954,5007,5740,5759,5786,5866,5880,5900,5905,5937,5969,5996,6000,6067,6108,6210,6264,6276,6312,6314,6389,6430,6509,6516,6571,6601,6678,6740,6963,6987,7072,7087,7141,7203,7247,7287,7404,7411,7414,7418,8504,11623,11670,11686,11694,11702,11710,11718,11726,11734,11742,12294,12348,12438,12447,12538,12543,12591,12686,12730,12799,19893,40943,40980,42124,42231,42507,42527,42539,42606,42725,42895,42999,43009,43013,43018,43042,43123,43187,43255,43259,43262,43301,43334,43388,43442,43492,43503,43518,43560,43586,43595,43631,43638,43642,43695,43697,43702,43709,43712,43714,43740,43754,43762,43782,43790,43798,43814,43822,44002,55203,55238,55291,64109,64217,64285,64296,64310,64316,64318,64321,64324,64433,64829,64911,64967,65019,65140,65276,65391,65437,65470,65479,65487,65495,65500,65547,65574,65594,65597,65613,65629,65786,66204,66256,66335,66368,66377,66421,66461,66499,66511,66717,66855,66915,67382,67413,67431,67589,67592,67637,67640,67644,67669,67702,67742,67826,67829,67861,67897,68023,68031,68096,68115,68119,68149,68220,68252,68295,68324,68405,68437,68466,68497,68680,68899,69404,69415,69445,69622,69687,69807,69864,69926,69956,70002,70006,70066,70084,70106,70108,70161,70187,70278,70280,70285,70301,70312,70366,70412,70416,70440,70448,70451,70457,70461,70480,70497,70708,70730,70751,70831,70853,70855,71086,71131,71215,71236,71338,71352,71450,71723,71935,72103,72144,72161,72163,72192,72242,72250,72272,72329,72349,72440,72712,72750,72768,72847,72966,72969,73008,73030,73061,73064,73097,73112,73458,74649,75075,78894,83526,92728,92766,92909,92975,93047,93071,94026,94032,100343,101106,110878,110930,110951,111355,113770,113788,113800,113817,123180,123214,123627,125124,126467,126495,126498,126500,126503,126514,126519,126521,126523,126530,126535,126537,126539,126543,126546,126548,126551,126553,126555,126557,126559,126562,126564,126570,126578,126583,126588,126590,126601,126619,126627,126633,126651,173782,177972,178205,183969,191456,195101};

// clang-format on

bool c11__is_unicode_Lo_char(int c) {
    if(c == 0x1f955) return true;
    int index;
    c11__lower_bound(const int, kLoRangeA, 476, c, c11__less, &index);
    if(index == 476) return false;
    if(c == kLoRangeA[index]) return true;
    index -= 1;
    if(index < 0) return false;
    return c >= kLoRangeA[index] && c <= kLoRangeB[index];
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