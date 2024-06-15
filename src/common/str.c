#include "pocketpy/common/str.h"
#include "pocketpy/common/utils.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

void pkpy_Str__ctor(pkpy_Str *self, const char *data){
    pkpy_Str__ctor2(self, data, strlen(data));
}

void pkpy_Str__ctor2(pkpy_Str *self, const char *data, int size){
    self->size = size;
    self->is_ascii = c11__isascii(data, size);
    self->is_sso = size < sizeof(self->_inlined);
    char* p;
    if(self->is_sso){
        p = self->_inlined;
    }else{
        self->_ptr = (char*)malloc(size + 1);
        p = self->_ptr;
    }
    memcpy(p, data, size);
    p[size] = '\0';
}

void pkpy_Str__dtor(pkpy_Str *self){
    if(!self->is_sso){
        free(self->_ptr);
        self->is_sso = true;
        self->size = 0;
    }
}

pkpy_Str pkpy_Str__copy(const pkpy_Str *self){
    pkpy_Str retval = *self;
    if(!self->is_sso){
        retval._ptr = (char*)malloc(self->size + 1);
        // '\0' is copied
        memcpy(retval._ptr, self->_ptr, self->size + 1);
    }
    return retval;
}

pkpy_Str pkpy_Str__concat(const pkpy_Str *self, const pkpy_Str *other){
    pkpy_Str retval = {
        .size = self->size + other->size,
        .is_ascii = self->is_ascii && other->is_ascii,
        .is_sso = self->size + other->size < sizeof(retval._inlined),
    };
    char* p;
    if(retval.is_sso){
        p = retval._inlined;
    }else{
        retval._ptr = (char*)malloc(retval.size + 1);
        p = retval._ptr;
    }
    memcpy(p, pkpy_Str__data(self), self->size);
    memcpy(p + self->size, pkpy_Str__data(other), other->size);
    p[retval.size] = '\0';
    return retval;
}

pkpy_Str pkpy_Str__concat2(const pkpy_Str *self, const char *other, int size){
    pkpy_Str tmp;
    pkpy_Str__ctor2(&tmp, other, size);
    pkpy_Str retval = pkpy_Str__concat(self, &tmp);
    pkpy_Str__dtor(&tmp);
    return retval;
}

pkpy_Str pkpy_Str__slice(const pkpy_Str *self, int start){
    return pkpy_Str__slice2(self, start, self->size);
}

pkpy_Str pkpy_Str__slice2(const pkpy_Str *self, int start, int stop){
    pkpy_Str retval;
    if(stop < start) stop = start;
    pkpy_Str__ctor2(&retval, pkpy_Str__data(self) + start, stop - start);
    return retval;
}

pkpy_Str pkpy_Str__lower(const pkpy_Str *self){
    pkpy_Str retval = pkpy_Str__copy(self);
    char* p = (char*)pkpy_Str__data(&retval);
    for(int i = 0; i < retval.size; i++){
        if('A' <= p[i] && p[i] <= 'Z') p[i] += 32;
    }
    return retval;
}

pkpy_Str pkpy_Str__upper(const pkpy_Str *self){
    pkpy_Str retval = pkpy_Str__copy(self);
    char* p = (char*)pkpy_Str__data(&retval);
    for(int i = 0; i < retval.size; i++){
        if('a' <= p[i] && p[i] <= 'z') p[i] -= 32;
    }
    return retval;
}

pkpy_Str pkpy_Str__escape(const pkpy_Str* self, char quote){
    assert(quote == '"' || quote == '\'');
    c11_vector buffer;
    c11_vector__ctor(&buffer, sizeof(char));
    c11_vector__reserve(&buffer, self->size);
    c11_vector__push(char, &buffer, quote);
    const char* data = pkpy_Str__data(self);
    for(int i = 0; i < self->size; i++) {
        char c = data[i];
        switch(c) {
            case '"': case '\'':
                if(c == quote) c11_vector__push(char, &buffer, '\\');
                c11_vector__push(char, &buffer, c);
                break;
            case '\\':
                c11_vector__push(char, &buffer, '\\');
                c11_vector__push(char, &buffer, '\\');
                break;
            case '\n':
                c11_vector__push(char, &buffer, '\\');
                c11_vector__push(char, &buffer, 'n');
                break;
            case '\r':
                c11_vector__push(char, &buffer, '\\');
                c11_vector__push(char, &buffer, 'r');
                break;
            case '\t':
                c11_vector__push(char, &buffer, '\\');
                c11_vector__push(char, &buffer, 't');
                break;
            case '\b':
                c11_vector__push(char, &buffer, '\\');
                c11_vector__push(char, &buffer, 'b');
                break;
            default:
                if('\x00' <= c && c <= '\x1f') {
                    c11_vector__push(char, &buffer, '\\');
                    c11_vector__push(char, &buffer, 'x');
                    c11_vector__push(char, &buffer, PK_HEX_TABLE[c >> 4]);
                    c11_vector__push(char, &buffer, PK_HEX_TABLE[c & 0xf]);
                } else {
                    c11_vector__push(char, &buffer, c);
                }
        }
    }
    c11_vector__push(char, &buffer, quote);
    c11_vector__push(char, &buffer, '\0');
    pkpy_Str retval = {
        .size = buffer.count - 1,
        .is_ascii = self->is_ascii,
        .is_sso = false,
        ._ptr = (char*)buffer.data,
    };
    return retval;
}

pkpy_Str pkpy_Str__strip(const pkpy_Str *self, bool left, bool right){
    const char* data = pkpy_Str__data(self);
    if(self->is_ascii) {
        int L = 0;
        int R = self->size;
        if(left) {
            while(L < R && (data[L] == ' ' || data[L] == '\t' || data[L] == '\n' || data[L] == '\r'))
                L++;
        }
        if(right) {
            while(L < R && (data[R - 1] == ' ' || data[R - 1] == '\t' || data[R - 1] == '\n' || data[R - 1] == '\r'))
                R--;
        }
        return pkpy_Str__slice2(self, L, R);
    } else {
        pkpy_Str tmp;
        pkpy_Str__ctor(&tmp, " \t\n\r");
        pkpy_Str retval = pkpy_Str__strip2(self, left, right, &tmp);
        pkpy_Str__dtor(&tmp);
        return retval;
    }
}

pkpy_Str pkpy_Str__strip2(const pkpy_Str *self, bool left, bool right, const pkpy_Str *chars){
    int L = 0;
    int R = pkpy_Str__u8_length(self);
    if(left) {
        while(L < R){
            pkpy_Str tmp = pkpy_Str__u8_getitem(self, L);
            bool found = pkpy_Str__index(chars, &tmp, 0) != -1;
            pkpy_Str__dtor(&tmp);
            if(!found) break;
            L++;
        }
    }
    if(right) {
        while(L < R){
            pkpy_Str tmp = pkpy_Str__u8_getitem(self, R - 1);
            bool found = pkpy_Str__index(chars, &tmp, 0) != -1;
            pkpy_Str__dtor(&tmp);
            if(!found) break;
            R--;
        }
    }
    return pkpy_Str__u8_slice(self, L, R, 1);
}

pkpy_Str pkpy_Str__replace(const pkpy_Str *self, char old, char new_){
    pkpy_Str retval = pkpy_Str__copy(self);
    char* p = (char*)pkpy_Str__data(&retval);
    for(int i = 0; i < retval.size; i++){
        if(p[i] == old) p[i] = new_;
    }
    return retval;
}

pkpy_Str pkpy_Str__replace2(const pkpy_Str *self, const pkpy_Str *old, const pkpy_Str *new_){
    c11_vector buffer;
    c11_vector__ctor(&buffer, sizeof(char));
    int start = 0;
    while(true) {
        int i = pkpy_Str__index(self, old, start);
        if(i == -1) break;
        pkpy_Str tmp = pkpy_Str__slice2(self, start, i);
        c11_vector__extend(char, &buffer, pkpy_Str__data(&tmp), tmp.size);
        pkpy_Str__dtor(&tmp);
        c11_vector__extend(char, &buffer, pkpy_Str__data(new_), new_->size);
        start = i + old->size;
    }
    pkpy_Str tmp = pkpy_Str__slice2(self, start, self->size);
    c11_vector__extend(char, &buffer, pkpy_Str__data(&tmp), tmp.size);
    pkpy_Str__dtor(&tmp);
    c11_vector__push(char, &buffer, '\0');
    pkpy_Str retval = {
        .size = buffer.count - 1,
        .is_ascii = self->is_ascii && old->is_ascii && new_->is_ascii,
        .is_sso = false,
        ._ptr = (char*)buffer.data,
    };
    return retval;
}

int pkpy_Str__cmp(const pkpy_Str *self, const pkpy_Str *other){
    return pkpy_Str__cmp2(self, pkpy_Str__data(other), other->size);
}

int pkpy_Str__cmp2(const pkpy_Str *self, const char *other, int size){
    int res = strncmp(pkpy_Str__data(self), other, PK_MIN(self->size, size));
    if(res != 0) return res;
    return self->size - size;
}

pkpy_Str pkpy_Str__u8_getitem(const pkpy_Str *self, int i){
    i = pkpy_Str__unicode_index_to_byte(self, i);
    int size = c11__u8_header(pkpy_Str__data(self)[i], false);
    return pkpy_Str__slice2(self, i, i + size);
}

pkpy_Str pkpy_Str__u8_slice(const pkpy_Str *self, int start, int stop, int step){
    c11_vector buffer;
    c11_vector__ctor(&buffer, sizeof(char));
    assert(step != 0);
    if(self->is_ascii){
        const char* p = pkpy_Str__data(self);
        for (int i=start; step>0 ? i<stop : i>stop; i+=step) {
            c11_vector__push(char, &buffer, p[i]);
        }
    }else{
        for (int i=start; step>0 ? i<stop : i>stop; i+=step) {
            pkpy_Str unicode = pkpy_Str__u8_getitem(self, i);
            const char* p = pkpy_Str__data(&unicode);
            for(int j = 0; j < unicode.size; j++){
                c11_vector__push(char, &buffer, p[j]);
            }
            pkpy_Str__dtor(&unicode);
        }
    }
    c11_vector__push(char, &buffer, '\0');
    pkpy_Str retval = {
        .size = buffer.count - 1,
        .is_ascii = self->is_ascii,
        .is_sso = false,
        ._ptr = (char*)buffer.data,
    };
    return retval;
}

int pkpy_Str__u8_length(const pkpy_Str *self){
    return pkpy_Str__byte_index_to_unicode(self, self->size);
}

int pkpy_Str__unicode_index_to_byte(const pkpy_Str* self, int i) {
    if(self->is_ascii) return i;
    const char* p = pkpy_Str__data(self);
    int j = 0;
    while(i > 0) {
        j += c11__u8_header(p[j], false);
        i--;
    }
    return j;
}

int pkpy_Str__byte_index_to_unicode(const pkpy_Str* self, int n) {
    if(self->is_ascii) return n;
    const char* p = pkpy_Str__data(self);
    int cnt = 0;
    for(int i = 0; i < n; i++) {
        if((p[i] & 0xC0) != 0x80) cnt++;
    }
    return cnt;
}

int pkpy_Str__index(const pkpy_Str *self, const pkpy_Str *sub, int start){
    if(sub->size == 0) return start;
    int max_end = self->size - sub->size;
    const char* self_data = pkpy_Str__data(self);
    const char* sub_data = pkpy_Str__data(sub);
    for(int i=start; i<=max_end; i++){
        int res = memcmp(self_data + i, sub_data, sub->size);
        if(res == 0) return i;
    }
    return -1;
}

int pkpy_Str__count(const pkpy_Str *self, const pkpy_Str *sub){
    if(sub->size == 0) return self->size + 1;
    int cnt = 0;
    int start = 0;
    while(true) {
        int i = pkpy_Str__index(self, sub, start);
        if(i == -1) break;
        cnt++;
        start = i + sub->size;
    }
    return cnt;
}

c11_vector/* T=c11_string */ pkpy_Str__split(const pkpy_Str *self, char sep){
    c11_vector retval;
    c11_vector__ctor(&retval, sizeof(c11_string));
    const char* data = pkpy_Str__data(self);
    int i = 0;
    for(int j = 0; j < self->size; j++) {
        if(data[j] == sep) {
            if(j > i){
                c11_string tmp = {data + i, j - i};
                c11_vector__push(c11_string, &retval, tmp);
            }
            i = j + 1;
            continue;
        }
    }
    if(self->size > i){
        c11_string tmp = {data + i, self->size - i};
        c11_vector__push(c11_string, &retval, tmp);
    }
    return retval;
}

c11_vector/* T=c11_string */ pkpy_Str__split2(const pkpy_Str *self, const pkpy_Str *sep){
    c11_vector retval;
    c11_vector__ctor(&retval, sizeof(c11_string));
    int start = 0;
    const char* data = pkpy_Str__data(self);
    while(true) {
        int i = pkpy_Str__index(self, sep, start);
        if(i == -1) break;
        c11_string tmp = {data + start, i - start};
        if(tmp.size != 0) c11_vector__push(c11_string, &retval, tmp);
        start = i + sep->size;
    }
    c11_string tmp = {data + start, self->size - start};
    if(tmp.size != 0) c11_vector__push(c11_string, &retval, tmp);
    return retval;
}

bool c11__isascii(const char* p, int size){
    for(int i = 0; i < size; i++)
        if((unsigned char)p[i] > 127)
            return false;
    return true;
}

// clang-format off
static const int kLoRangeA[] = {170,186,443,448,660,1488,1519,1568,1601,1646,1649,1749,1774,1786,1791,1808,1810,1869,1969,1994,2048,2112,2144,2208,2230,2308,2365,2384,2392,2418,2437,2447,2451,2474,2482,2486,2493,2510,2524,2527,2544,2556,2565,2575,2579,2602,2610,2613,2616,2649,2654,2674,2693,2703,2707,2730,2738,2741,2749,2768,2784,2809,2821,2831,2835,2858,2866,2869,2877,2908,2911,2929,2947,2949,2958,2962,2969,2972,2974,2979,2984,2990,3024,3077,3086,3090,3114,3133,3160,3168,3200,3205,3214,3218,3242,3253,3261,3294,3296,3313,3333,3342,3346,3389,3406,3412,3423,3450,3461,3482,3507,3517,3520,3585,3634,3648,3713,3716,3718,3724,3749,3751,3762,3773,3776,3804,3840,3904,3913,3976,4096,4159,4176,4186,4193,4197,4206,4213,4238,4352,4682,4688,4696,4698,4704,4746,4752,4786,4792,4800,4802,4808,4824,4882,4888,4992,5121,5743,5761,5792,5873,5888,5902,5920,5952,5984,5998,6016,6108,6176,6212,6272,6279,6314,6320,6400,6480,6512,6528,6576,6656,6688,6917,6981,7043,7086,7098,7168,7245,7258,7401,7406,7413,7418,8501,11568,11648,11680,11688,11696,11704,11712,11720,11728,11736,12294,12348,12353,12447,12449,12543,12549,12593,12704,12784,13312,19968,40960,40982,42192,42240,42512,42538,42606,42656,42895,42999,43003,43011,43015,43020,43072,43138,43250,43259,43261,43274,43312,43360,43396,43488,43495,43514,43520,43584,43588,43616,43633,43642,43646,43697,43701,43705,43712,43714,43739,43744,43762,43777,43785,43793,43808,43816,43968,44032,55216,55243,63744,64112,64285,64287,64298,64312,64318,64320,64323,64326,64467,64848,64914,65008,65136,65142,65382,65393,65440,65474,65482,65490,65498,65536,65549,65576,65596,65599,65616,65664,66176,66208,66304,66349,66370,66384,66432,66464,66504,66640,66816,66864,67072,67392,67424,67584,67592,67594,67639,67644,67647,67680,67712,67808,67828,67840,67872,67968,68030,68096,68112,68117,68121,68192,68224,68288,68297,68352,68416,68448,68480,68608,68864,69376,69415,69424,69600,69635,69763,69840,69891,69956,69968,70006,70019,70081,70106,70108,70144,70163,70272,70280,70282,70287,70303,70320,70405,70415,70419,70442,70450,70453,70461,70480,70493,70656,70727,70751,70784,70852,70855,71040,71128,71168,71236,71296,71352,71424,71680,71935,72096,72106,72161,72163,72192,72203,72250,72272,72284,72349,72384,72704,72714,72768,72818,72960,72968,72971,73030,73056,73063,73066,73112,73440,73728,74880,77824,82944,92160,92736,92880,92928,93027,93053,93952,94032,94208,100352,110592,110928,110948,110960,113664,113776,113792,113808,123136,123214,123584,124928,126464,126469,126497,126500,126503,126505,126516,126521,126523,126530,126535,126537,126539,126541,126545,126548,126551,126553,126555,126557,126559,126561,126564,126567,126572,126580,126585,126590,126592,126603,126625,126629,126635,131072,173824,177984,178208,183984,194560};
static const int kLoRangeB[] = {170,186,443,451,660,1514,1522,1599,1610,1647,1747,1749,1775,1788,1791,1808,1839,1957,1969,2026,2069,2136,2154,2228,2237,2361,2365,2384,2401,2432,2444,2448,2472,2480,2482,2489,2493,2510,2525,2529,2545,2556,2570,2576,2600,2608,2611,2614,2617,2652,2654,2676,2701,2705,2728,2736,2739,2745,2749,2768,2785,2809,2828,2832,2856,2864,2867,2873,2877,2909,2913,2929,2947,2954,2960,2965,2970,2972,2975,2980,2986,3001,3024,3084,3088,3112,3129,3133,3162,3169,3200,3212,3216,3240,3251,3257,3261,3294,3297,3314,3340,3344,3386,3389,3406,3414,3425,3455,3478,3505,3515,3517,3526,3632,3635,3653,3714,3716,3722,3747,3749,3760,3763,3773,3780,3807,3840,3911,3948,3980,4138,4159,4181,4189,4193,4198,4208,4225,4238,4680,4685,4694,4696,4701,4744,4749,4784,4789,4798,4800,4805,4822,4880,4885,4954,5007,5740,5759,5786,5866,5880,5900,5905,5937,5969,5996,6000,6067,6108,6210,6264,6276,6312,6314,6389,6430,6509,6516,6571,6601,6678,6740,6963,6987,7072,7087,7141,7203,7247,7287,7404,7411,7414,7418,8504,11623,11670,11686,11694,11702,11710,11718,11726,11734,11742,12294,12348,12438,12447,12538,12543,12591,12686,12730,12799,19893,40943,40980,42124,42231,42507,42527,42539,42606,42725,42895,42999,43009,43013,43018,43042,43123,43187,43255,43259,43262,43301,43334,43388,43442,43492,43503,43518,43560,43586,43595,43631,43638,43642,43695,43697,43702,43709,43712,43714,43740,43754,43762,43782,43790,43798,43814,43822,44002,55203,55238,55291,64109,64217,64285,64296,64310,64316,64318,64321,64324,64433,64829,64911,64967,65019,65140,65276,65391,65437,65470,65479,65487,65495,65500,65547,65574,65594,65597,65613,65629,65786,66204,66256,66335,66368,66377,66421,66461,66499,66511,66717,66855,66915,67382,67413,67431,67589,67592,67637,67640,67644,67669,67702,67742,67826,67829,67861,67897,68023,68031,68096,68115,68119,68149,68220,68252,68295,68324,68405,68437,68466,68497,68680,68899,69404,69415,69445,69622,69687,69807,69864,69926,69956,70002,70006,70066,70084,70106,70108,70161,70187,70278,70280,70285,70301,70312,70366,70412,70416,70440,70448,70451,70457,70461,70480,70497,70708,70730,70751,70831,70853,70855,71086,71131,71215,71236,71338,71352,71450,71723,71935,72103,72144,72161,72163,72192,72242,72250,72272,72329,72349,72440,72712,72750,72768,72847,72966,72969,73008,73030,73061,73064,73097,73112,73458,74649,75075,78894,83526,92728,92766,92909,92975,93047,93071,94026,94032,100343,101106,110878,110930,110951,111355,113770,113788,113800,113817,123180,123214,123627,125124,126467,126495,126498,126500,126503,126514,126519,126521,126523,126530,126535,126537,126539,126543,126546,126548,126551,126553,126555,126557,126559,126562,126564,126570,126578,126583,126588,126590,126601,126619,126627,126633,126651,173782,177972,178205,183969,191456,195101};
// clang-format on

bool c11__is_unicode_Lo_char(int c){
    if(c == 0x1f955) return true;
    const int* p;
    c11__lower_bound(int, kLoRangeA, 476, c, c11__less, &p);
    int index = p - kLoRangeA;
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
    if(!suppress) PK_FATAL_ERROR("invalid utf8 char\n")
    return 0;
}