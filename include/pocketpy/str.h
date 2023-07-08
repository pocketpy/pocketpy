#pragma once

#include "common.h"
#include "memory.h"
#include "vector.h"

namespace pkpy {

inline int utf8len(unsigned char c, bool suppress=false){
    if((c & 0b10000000) == 0) return 1;
    if((c & 0b11100000) == 0b11000000) return 2;
    if((c & 0b11110000) == 0b11100000) return 3;
    if((c & 0b11111000) == 0b11110000) return 4;
    if((c & 0b11111100) == 0b11111000) return 5;
    if((c & 0b11111110) == 0b11111100) return 6;
    if(!suppress) throw std::runtime_error("invalid utf8 char: " + std::to_string(c));
    return 0;
}

struct Str{
    int size;
    bool is_ascii;
    char* data;
    char _inlined[16];

    mutable const char* _cached_c_str = nullptr;

    bool is_inlined() const { return data == _inlined; }

    Str(): size(0), is_ascii(true), data(_inlined) {}

    void _alloc(){
        if(size <= 16){
            this->data = _inlined;
        }else{
            this->data = (char*)pool64.alloc(size);
        }
    }

    Str(int size, bool is_ascii): size(size), is_ascii(is_ascii) {
        _alloc();
    }

#define STR_INIT()                                  \
        _alloc();                                   \
        for(int i=0; i<size; i++){                  \
            data[i] = s[i];                         \
            if(!isascii(s[i])) is_ascii = false;    \
        }

    Str(const std::string& s): size(s.size()), is_ascii(true) {
        STR_INIT()
    }

    Str(std::string_view s): size(s.size()), is_ascii(true) {
        STR_INIT()
    }

    Str(const char* s): size(strlen(s)), is_ascii(true) {
        STR_INIT()
    }

    Str(const char* s, int len): size(len), is_ascii(true) {
        STR_INIT()
    }

#undef STR_INIT

    Str(const Str& other): size(other.size), is_ascii(other.is_ascii) {
        _alloc();
        memcpy(data, other.data, size);
    }

    Str(Str&& other): size(other.size), is_ascii(other.is_ascii) {
        if(other.is_inlined()){
            data = _inlined;
            for(int i=0; i<size; i++) _inlined[i] = other._inlined[i];
        }else{
            data = other.data;
            other.data = other._inlined;
            other.size = 0;
        }
    }

    const char* begin() const { return data; }
    const char* end() const { return data + size; }
    char operator[](int idx) const { return data[idx]; }
    int length() const { return size; }
    bool empty() const { return size == 0; }
    size_t hash() const{ return std::hash<std::string_view>()(sv()); }

    Str& operator=(const Str& other);
    Str operator+(const Str& other) const;
    Str operator+(const char* p) const;

    bool operator==(const Str& other) const;
    bool operator!=(const Str& other) const;
    bool operator==(const std::string_view other) const;
    bool operator!=(const std::string_view other) const;
    bool operator==(const char* p) const;
    bool operator!=(const char* p) const;
    bool operator<(const Str& other) const;
    bool operator>(const Str& other) const;
    bool operator<=(const Str& other) const;
    bool operator>=(const Str& other) const;
    bool operator<(const std::string_view other) const;

    ~Str();

    friend Str operator+(const char* p, const Str& str){
        Str other(p);
        return other + str;
    }

    friend std::ostream& operator<<(std::ostream& os, const Str& str){
        os.write(str.data, str.size);
        return os;
    }

    friend bool operator<(const std::string_view other, const Str& str){
        return str > other;
    }

    Str substr(int start, int len) const;
    Str substr(int start) const;
    char* c_str_dup() const;
    const char* c_str() const;
    std::string_view sv() const;
    std::string str() const;
    Str lstrip() const;
    Str strip() const;
    Str lower() const;
    Str upper() const;
    Str escape(bool single_quote=true) const;
    int index(const Str& sub, int start=0) const;
    Str replace(const Str& old, const Str& new_, int count=-1) const;

    /*************unicode*************/
    int _unicode_index_to_byte(int i) const;
    int _byte_index_to_unicode(int n) const;
    Str u8_getitem(int i) const;
    Str u8_slice(int start, int stop, int step) const;
    int u8_length() const;
};

template<typename... Args>
inline std::string fmt(Args&&... args) {
    std::stringstream ss;
    (ss << ... << args);
    return ss.str();
}

const uint32_t kLoRangeA[] = {170,186,443,448,660,1488,1519,1568,1601,1646,1649,1749,1774,1786,1791,1808,1810,1869,1969,1994,2048,2112,2144,2208,2230,2308,2365,2384,2392,2418,2437,2447,2451,2474,2482,2486,2493,2510,2524,2527,2544,2556,2565,2575,2579,2602,2610,2613,2616,2649,2654,2674,2693,2703,2707,2730,2738,2741,2749,2768,2784,2809,2821,2831,2835,2858,2866,2869,2877,2908,2911,2929,2947,2949,2958,2962,2969,2972,2974,2979,2984,2990,3024,3077,3086,3090,3114,3133,3160,3168,3200,3205,3214,3218,3242,3253,3261,3294,3296,3313,3333,3342,3346,3389,3406,3412,3423,3450,3461,3482,3507,3517,3520,3585,3634,3648,3713,3716,3718,3724,3749,3751,3762,3773,3776,3804,3840,3904,3913,3976,4096,4159,4176,4186,4193,4197,4206,4213,4238,4352,4682,4688,4696,4698,4704,4746,4752,4786,4792,4800,4802,4808,4824,4882,4888,4992,5121,5743,5761,5792,5873,5888,5902,5920,5952,5984,5998,6016,6108,6176,6212,6272,6279,6314,6320,6400,6480,6512,6528,6576,6656,6688,6917,6981,7043,7086,7098,7168,7245,7258,7401,7406,7413,7418,8501,11568,11648,11680,11688,11696,11704,11712,11720,11728,11736,12294,12348,12353,12447,12449,12543,12549,12593,12704,12784,13312,19968,40960,40982,42192,42240,42512,42538,42606,42656,42895,42999,43003,43011,43015,43020,43072,43138,43250,43259,43261,43274,43312,43360,43396,43488,43495,43514,43520,43584,43588,43616,43633,43642,43646,43697,43701,43705,43712,43714,43739,43744,43762,43777,43785,43793,43808,43816,43968,44032,55216,55243,63744,64112,64285,64287,64298,64312,64318,64320,64323,64326,64467,64848,64914,65008,65136,65142,65382,65393,65440,65474,65482,65490,65498,65536,65549,65576,65596,65599,65616,65664,66176,66208,66304,66349,66370,66384,66432,66464,66504,66640,66816,66864,67072,67392,67424,67584,67592,67594,67639,67644,67647,67680,67712,67808,67828,67840,67872,67968,68030,68096,68112,68117,68121,68192,68224,68288,68297,68352,68416,68448,68480,68608,68864,69376,69415,69424,69600,69635,69763,69840,69891,69956,69968,70006,70019,70081,70106,70108,70144,70163,70272,70280,70282,70287,70303,70320,70405,70415,70419,70442,70450,70453,70461,70480,70493,70656,70727,70751,70784,70852,70855,71040,71128,71168,71236,71296,71352,71424,71680,71935,72096,72106,72161,72163,72192,72203,72250,72272,72284,72349,72384,72704,72714,72768,72818,72960,72968,72971,73030,73056,73063,73066,73112,73440,73728,74880,77824,82944,92160,92736,92880,92928,93027,93053,93952,94032,94208,100352,110592,110928,110948,110960,113664,113776,113792,113808,123136,123214,123584,124928,126464,126469,126497,126500,126503,126505,126516,126521,126523,126530,126535,126537,126539,126541,126545,126548,126551,126553,126555,126557,126559,126561,126564,126567,126572,126580,126585,126590,126592,126603,126625,126629,126635,131072,173824,177984,178208,183984,194560};
const uint32_t kLoRangeB[] = {170,186,443,451,660,1514,1522,1599,1610,1647,1747,1749,1775,1788,1791,1808,1839,1957,1969,2026,2069,2136,2154,2228,2237,2361,2365,2384,2401,2432,2444,2448,2472,2480,2482,2489,2493,2510,2525,2529,2545,2556,2570,2576,2600,2608,2611,2614,2617,2652,2654,2676,2701,2705,2728,2736,2739,2745,2749,2768,2785,2809,2828,2832,2856,2864,2867,2873,2877,2909,2913,2929,2947,2954,2960,2965,2970,2972,2975,2980,2986,3001,3024,3084,3088,3112,3129,3133,3162,3169,3200,3212,3216,3240,3251,3257,3261,3294,3297,3314,3340,3344,3386,3389,3406,3414,3425,3455,3478,3505,3515,3517,3526,3632,3635,3653,3714,3716,3722,3747,3749,3760,3763,3773,3780,3807,3840,3911,3948,3980,4138,4159,4181,4189,4193,4198,4208,4225,4238,4680,4685,4694,4696,4701,4744,4749,4784,4789,4798,4800,4805,4822,4880,4885,4954,5007,5740,5759,5786,5866,5880,5900,5905,5937,5969,5996,6000,6067,6108,6210,6264,6276,6312,6314,6389,6430,6509,6516,6571,6601,6678,6740,6963,6987,7072,7087,7141,7203,7247,7287,7404,7411,7414,7418,8504,11623,11670,11686,11694,11702,11710,11718,11726,11734,11742,12294,12348,12438,12447,12538,12543,12591,12686,12730,12799,19893,40943,40980,42124,42231,42507,42527,42539,42606,42725,42895,42999,43009,43013,43018,43042,43123,43187,43255,43259,43262,43301,43334,43388,43442,43492,43503,43518,43560,43586,43595,43631,43638,43642,43695,43697,43702,43709,43712,43714,43740,43754,43762,43782,43790,43798,43814,43822,44002,55203,55238,55291,64109,64217,64285,64296,64310,64316,64318,64321,64324,64433,64829,64911,64967,65019,65140,65276,65391,65437,65470,65479,65487,65495,65500,65547,65574,65594,65597,65613,65629,65786,66204,66256,66335,66368,66377,66421,66461,66499,66511,66717,66855,66915,67382,67413,67431,67589,67592,67637,67640,67644,67669,67702,67742,67826,67829,67861,67897,68023,68031,68096,68115,68119,68149,68220,68252,68295,68324,68405,68437,68466,68497,68680,68899,69404,69415,69445,69622,69687,69807,69864,69926,69956,70002,70006,70066,70084,70106,70108,70161,70187,70278,70280,70285,70301,70312,70366,70412,70416,70440,70448,70451,70457,70461,70480,70497,70708,70730,70751,70831,70853,70855,71086,71131,71215,71236,71338,71352,71450,71723,71935,72103,72144,72161,72163,72192,72242,72250,72272,72329,72349,72440,72712,72750,72768,72847,72966,72969,73008,73030,73061,73064,73097,73112,73458,74649,75075,78894,83526,92728,92766,92909,92975,93047,93071,94026,94032,100343,101106,110878,110930,110951,111355,113770,113788,113800,113817,123180,123214,123627,125124,126467,126495,126498,126500,126503,126514,126519,126521,126523,126530,126535,126537,126539,126543,126546,126548,126551,126553,126555,126557,126559,126562,126564,126570,126578,126583,126588,126590,126601,126619,126627,126633,126651,173782,177972,178205,183969,191456,195101};

inline bool is_unicode_Lo_char(uint32_t c) {
    auto index = std::lower_bound(kLoRangeA, kLoRangeA + 476, c) - kLoRangeA;
    if(c == kLoRangeA[index]) return true;
    index -= 1;
    if(index < 0) return false;
    return c >= kLoRangeA[index] && c <= kLoRangeB[index];
}

struct StrName {
    uint16_t index;
    StrName();
    explicit StrName(uint16_t index);
    StrName(const char* s);
    StrName(const Str& s);
    std::string_view sv() const;
    bool empty() const { return index == 0; }

    friend std::ostream& operator<<(std::ostream& os, const StrName& sn){
        return os << sn.sv();
    }

    Str escape() const;

    bool operator==(const StrName& other) const noexcept {
        return this->index == other.index;
    }

    bool operator!=(const StrName& other) const noexcept {
        return this->index != other.index;
    }

    bool operator<(const StrName& other) const noexcept {
        return this->index < other.index;
    }

    bool operator>(const StrName& other) const noexcept {
        return this->index > other.index;
    }

    inline static std::map<Str, uint16_t, std::less<>> _interned;
    inline static std::vector<Str> _r_interned;

    static bool is_valid(int index);
    static StrName get(std::string_view s);
};

struct FastStrStream{
    pod_vector<const Str*> parts;

    FastStrStream& operator<<(const Str& s){
        parts.push_back(&s);
        return *this;
    }

    bool empty() const { return parts.empty(); }

    Str str() const{
        int len = 0;
        bool is_ascii = true;
        for(auto& s: parts){
            len += s->length();
            is_ascii &= s->is_ascii;
        }
        Str result(len, is_ascii);
        char* p = result.data;
        for(auto& s: parts){
            memcpy(p, s->data, s->length());
            p += s->length();
        }
        return result;
    }
};

struct CString{
	const char* ptr;
	CString(const char* ptr): ptr(ptr) {}
    operator const char*() const { return ptr; }
};

// unary operators
const StrName __repr__ = StrName::get("__repr__");
const StrName __str__ = StrName::get("__str__");
const StrName __hash__ = StrName::get("__hash__");      // unused
const StrName __len__ = StrName::get("__len__");
const StrName __iter__ = StrName::get("__iter__");
const StrName __next__ = StrName::get("__next__");      // unused
const StrName __json__ = StrName::get("__json__");
const StrName __neg__ = StrName::get("__neg__");        // unused
const StrName __bool__ = StrName::get("__bool__");      // unused
// logical operators
const StrName __eq__ = StrName::get("__eq__");
const StrName __lt__ = StrName::get("__lt__");
const StrName __le__ = StrName::get("__le__");
const StrName __gt__ = StrName::get("__gt__");
const StrName __ge__ = StrName::get("__ge__");
const StrName __contains__ = StrName::get("__contains__");
// binary operators
const StrName __add__ = StrName::get("__add__");
const StrName __radd__ = StrName::get("__radd__");
const StrName __sub__ = StrName::get("__sub__");
const StrName __rsub__ = StrName::get("__rsub__");
const StrName __mul__ = StrName::get("__mul__");
const StrName __rmul__ = StrName::get("__rmul__");
const StrName __truediv__ = StrName::get("__truediv__");
const StrName __floordiv__ = StrName::get("__floordiv__");
const StrName __mod__ = StrName::get("__mod__");
const StrName __pow__ = StrName::get("__pow__");
const StrName __matmul__ = StrName::get("__matmul__");
const StrName __lshift__ = StrName::get("__lshift__");
const StrName __rshift__ = StrName::get("__rshift__");
const StrName __and__ = StrName::get("__and__");
const StrName __or__ = StrName::get("__or__");
const StrName __xor__ = StrName::get("__xor__");
const StrName __invert__ = StrName::get("__invert__");
// indexer
const StrName __getitem__ = StrName::get("__getitem__");
const StrName __setitem__ = StrName::get("__setitem__");
const StrName __delitem__ = StrName::get("__delitem__");

#define DEF_SNAME(name) const static StrName name(#name)

} // namespace pkpy