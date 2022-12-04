#pragma once

#include "__stl__.h"

typedef std::stringstream _StrStream;

class _StrMemory : public std::string {
    mutable std::vector<uint16_t>* _u8_index = nullptr;

    mutable bool hash_initialized = false;
    mutable size_t _hash;

    void utf8_lazy_init() const{
        if(_u8_index != nullptr) return;
        _u8_index = new std::vector<uint16_t>();
        _u8_index->reserve(size());
        if(size() > 65535) throw std::runtime_error("String has more than 65535 bytes.");
        for(uint16_t i = 0; i < size(); i++){
            // https://stackoverflow.com/questions/3911536/utf-8-unicode-whats-with-0xc0-and-0x80
            if((at(i) & 0xC0) != 0x80)
                _u8_index->push_back(i);
        }
    }
public:
    size_t hash() const{
        if(!hash_initialized){
            _hash = std::hash<std::string>()(*this);
            hash_initialized = true;
        }
        return _hash;
    }

    int u8_length() const {
        utf8_lazy_init();
        return _u8_index->size();
    }

    std::string u8_getitem(int i) const{
        return u8_substr(i, i+1);
    }

    std::string u8_substr(int start, int end) const{
        utf8_lazy_init();
        if(start >= end) return std::string();
        int c_end = end >= _u8_index->size() ? size() : _u8_index->at(end);
        return substr(_u8_index->at(start), c_end - _u8_index->at(start));
    }

    _StrMemory(const std::string& s) : std::string(s) {}
    _StrMemory(std::string&& s) : std::string(std::move(s)) {}

    ~_StrMemory(){
        if(_u8_index != nullptr) delete _u8_index;
    }
};


std::map<std::string, pkpy::shared_ptr<_StrMemory>, std::less<>> _strIntern;


class _StrLiteral : public std::string_view {
public:
    constexpr _StrLiteral(const char* str, size_t len) : std::string_view(str, len) {}
};

inline constexpr _StrLiteral operator "" _c(const char* str, size_t len){
    return _StrLiteral(str, len);
}

class _Str {
private:
    pkpy::shared_ptr<_StrMemory> _s;
    bool interned = false;
public:
    _Str(const _StrLiteral& s){
        construct(s);
        intern();
    }
    _Str(const char* s){
        construct(s);
    }
    _Str(const char* s, size_t len){
        construct(std::string_view(s, len));
    }
    _Str(){
        construct("");
    }
    _Str(const std::string& s){
        construct(s);
    }
    _Str(const _Str& s) : _s(s._s), interned(s.interned) {}

    // for move constructor, we do not check if the string is interned!!
    _Str(std::string&& s){
        this->_s = pkpy::make_shared<_StrMemory>(std::move(s));
    }

    void construct(const std::string_view& sv){
        auto it = _strIntern.find(sv);
        if(it != _strIntern.end()){
            this->_s = it->second;
            interned = true;
        }else{
            this->_s = pkpy::make_shared<_StrMemory>(std::string(sv));
        }
    }

    // force the string to be interned
    void intern(){
        if(interned) return;
        auto it = _strIntern.find(*this->_s);
        if(it == _strIntern.end()) _strIntern[*this->_s] = this->_s;
        else this->_s = it->second;
        interned = true;
    }

    inline int u8_length() const {
        return this->_s->u8_length();
    }

    inline _Str u8_getitem(int i) const{
        return _Str(this->_s->u8_getitem(i));
    }

    inline _Str u8_substr(int start, int end) const{
        return _Str(this->_s->u8_substr(start, end));
    }

    inline size_t hash() const{
        return _s->hash();
    }

    inline int size() const {
        return _s->size();
    }

    inline bool empty() const {
        return _s->empty();
    }

    bool operator==(const _Str& other) const {
        if(interned && other.interned) return _s == other._s;
        return *_s == *other._s;
    }

    bool operator!=(const _Str& other) const {
        if(interned && other.interned) return _s != other._s;
        return *_s != *other._s;
    }

    bool operator<(const _Str& other) const {
        return *_s < *other._s;
    }

    bool operator>(const _Str& other) const {
        return *_s > *other._s;
    }

    char operator[](int i) const {
        return _s->at(i);
    }

    friend std::ostream& operator<<(std::ostream& os, const _Str& s) {
        os << *s._s;
        return os;
    }

    _Str operator+(const _Str& other) const {
        return _Str(*_s + *other._s);
    }

    _Str operator+(const char* other) const {
        return _Str(*_s + other);
    }

    _Str operator+(const std::string& other) const {
        return _Str(*_s + other);
    }

    friend _Str operator+(const char* other, const _Str& s){
        return _Str(other + *s._s);
    }

    friend _Str operator+(const std::string& other, const _Str& s){
        return _Str(other + *s._s);
    }

    const std::string& str() const {
        return *_s;
    }

    const char* c_str() const {
        return _s->c_str();
    }

    static const std::size_t npos = std::string::npos;

    _Str __lstrip() const {
        std::string copy(*_s);
        copy.erase(copy.begin(), std::find_if(copy.begin(), copy.end(), [](char c) {
            return !std::isspace(c);
        }));
        return _Str(copy);
    }

    _Str __escape(bool single_quote) const {
        _StrStream ss;
        ss << (single_quote ? '\'' : '"');
        for (auto c = _s->cbegin(); c != _s->cend(); c++) {
            switch (*c) {
                case '"':
                    if(!single_quote) ss << '\\';
                    ss << '"';
                    break;
                case '\'':
                    if(single_quote) ss << '\\';
                    ss << '\'';
                    break;
                case '\\': ss << '\\' << '\\'; break;
                case '\n': ss << "\\n"; break;
                case '\r': ss << "\\r"; break;
                case '\t': ss << "\\t"; break;
                default:
                    if ('\x00' <= *c && *c <= '\x1f') {
                        ss << "\\u"
                        << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(*c);
                    } else {
                        ss << *c;
                    }
            }
        }
        ss << (single_quote ? '\'' : '"');
        return ss.str();
    }
};

namespace std {
    template<>
    struct hash<_Str> {
        std::size_t operator()(const _Str& s) const {
            return s.hash();
        }
    };
}

const _Str& __class__ = _Str("__class__"_c);
const _Str& __base__ = _Str("__base__"_c);
const _Str& __new__ = _Str("__new__"_c);
const _Str& __iter__ = _Str("__iter__"_c);
const _Str& __str__ = _Str("__str__"_c);
const _Str& __repr__ = _Str("__repr__"_c);
const _Str& __module__ = _Str("__module__"_c);
const _Str& __getitem__ = _Str("__getitem__"_c);
const _Str& __setitem__ = _Str("__setitem__"_c);
const _Str& __delitem__ = _Str("__delitem__"_c);
const _Str& __contains__ = _Str("__contains__"_c);
const _Str& __init__ = _Str("__init__"_c);
const _Str& __json__ = _Str("__json__"_c);
const _Str& __name__ = _Str("__name__"_c);
const _Str& __len__ = _Str("__len__"_c);

const _Str CMP_SPECIAL_METHODS[] = {
    "__lt__"_c, "__le__"_c, "__eq__"_c, "__ne__"_c, "__gt__"_c, "__ge__"_c
};  // __ne__ should not be used

const _Str BINARY_SPECIAL_METHODS[] = {
    "__add__"_c, "__sub__"_c, "__mul__"_c, "__truediv__"_c, "__floordiv__"_c, "__mod__"_c, "__pow__"_c
};

const _Str BITWISE_SPECIAL_METHODS[] = {
    "__lshift__"_c, "__rshift__"_c, "__and__"_c, "__or__"_c, "__xor__"_c
};

const uint32_t __LoRangeA[] = {170,186,443,448,660,1488,1519,1568,1601,1646,1649,1749,1774,1786,1791,1808,1810,1869,1969,1994,2048,2112,2144,2208,2230,2308,2365,2384,2392,2418,2437,2447,2451,2474,2482,2486,2493,2510,2524,2527,2544,2556,2565,2575,2579,2602,2610,2613,2616,2649,2654,2674,2693,2703,2707,2730,2738,2741,2749,2768,2784,2809,2821,2831,2835,2858,2866,2869,2877,2908,2911,2929,2947,2949,2958,2962,2969,2972,2974,2979,2984,2990,3024,3077,3086,3090,3114,3133,3160,3168,3200,3205,3214,3218,3242,3253,3261,3294,3296,3313,3333,3342,3346,3389,3406,3412,3423,3450,3461,3482,3507,3517,3520,3585,3634,3648,3713,3716,3718,3724,3749,3751,3762,3773,3776,3804,3840,3904,3913,3976,4096,4159,4176,4186,4193,4197,4206,4213,4238,4352,4682,4688,4696,4698,4704,4746,4752,4786,4792,4800,4802,4808,4824,4882,4888,4992,5121,5743,5761,5792,5873,5888,5902,5920,5952,5984,5998,6016,6108,6176,6212,6272,6279,6314,6320,6400,6480,6512,6528,6576,6656,6688,6917,6981,7043,7086,7098,7168,7245,7258,7401,7406,7413,7418,8501,11568,11648,11680,11688,11696,11704,11712,11720,11728,11736,12294,12348,12353,12447,12449,12543,12549,12593,12704,12784,13312,19968,40960,40982,42192,42240,42512,42538,42606,42656,42895,42999,43003,43011,43015,43020,43072,43138,43250,43259,43261,43274,43312,43360,43396,43488,43495,43514,43520,43584,43588,43616,43633,43642,43646,43697,43701,43705,43712,43714,43739,43744,43762,43777,43785,43793,43808,43816,43968,44032,55216,55243,63744,64112,64285,64287,64298,64312,64318,64320,64323,64326,64467,64848,64914,65008,65136,65142,65382,65393,65440,65474,65482,65490,65498,65536,65549,65576,65596,65599,65616,65664,66176,66208,66304,66349,66370,66384,66432,66464,66504,66640,66816,66864,67072,67392,67424,67584,67592,67594,67639,67644,67647,67680,67712,67808,67828,67840,67872,67968,68030,68096,68112,68117,68121,68192,68224,68288,68297,68352,68416,68448,68480,68608,68864,69376,69415,69424,69600,69635,69763,69840,69891,69956,69968,70006,70019,70081,70106,70108,70144,70163,70272,70280,70282,70287,70303,70320,70405,70415,70419,70442,70450,70453,70461,70480,70493,70656,70727,70751,70784,70852,70855,71040,71128,71168,71236,71296,71352,71424,71680,71935,72096,72106,72161,72163,72192,72203,72250,72272,72284,72349,72384,72704,72714,72768,72818,72960,72968,72971,73030,73056,73063,73066,73112,73440,73728,74880,77824,82944,92160,92736,92880,92928,93027,93053,93952,94032,94208,100352,110592,110928,110948,110960,113664,113776,113792,113808,123136,123214,123584,124928,126464,126469,126497,126500,126503,126505,126516,126521,126523,126530,126535,126537,126539,126541,126545,126548,126551,126553,126555,126557,126559,126561,126564,126567,126572,126580,126585,126590,126592,126603,126625,126629,126635,131072,173824,177984,178208,183984,194560};
const uint32_t __LoRangeB[] = {170,186,443,451,660,1514,1522,1599,1610,1647,1747,1749,1775,1788,1791,1808,1839,1957,1969,2026,2069,2136,2154,2228,2237,2361,2365,2384,2401,2432,2444,2448,2472,2480,2482,2489,2493,2510,2525,2529,2545,2556,2570,2576,2600,2608,2611,2614,2617,2652,2654,2676,2701,2705,2728,2736,2739,2745,2749,2768,2785,2809,2828,2832,2856,2864,2867,2873,2877,2909,2913,2929,2947,2954,2960,2965,2970,2972,2975,2980,2986,3001,3024,3084,3088,3112,3129,3133,3162,3169,3200,3212,3216,3240,3251,3257,3261,3294,3297,3314,3340,3344,3386,3389,3406,3414,3425,3455,3478,3505,3515,3517,3526,3632,3635,3653,3714,3716,3722,3747,3749,3760,3763,3773,3780,3807,3840,3911,3948,3980,4138,4159,4181,4189,4193,4198,4208,4225,4238,4680,4685,4694,4696,4701,4744,4749,4784,4789,4798,4800,4805,4822,4880,4885,4954,5007,5740,5759,5786,5866,5880,5900,5905,5937,5969,5996,6000,6067,6108,6210,6264,6276,6312,6314,6389,6430,6509,6516,6571,6601,6678,6740,6963,6987,7072,7087,7141,7203,7247,7287,7404,7411,7414,7418,8504,11623,11670,11686,11694,11702,11710,11718,11726,11734,11742,12294,12348,12438,12447,12538,12543,12591,12686,12730,12799,19893,40943,40980,42124,42231,42507,42527,42539,42606,42725,42895,42999,43009,43013,43018,43042,43123,43187,43255,43259,43262,43301,43334,43388,43442,43492,43503,43518,43560,43586,43595,43631,43638,43642,43695,43697,43702,43709,43712,43714,43740,43754,43762,43782,43790,43798,43814,43822,44002,55203,55238,55291,64109,64217,64285,64296,64310,64316,64318,64321,64324,64433,64829,64911,64967,65019,65140,65276,65391,65437,65470,65479,65487,65495,65500,65547,65574,65594,65597,65613,65629,65786,66204,66256,66335,66368,66377,66421,66461,66499,66511,66717,66855,66915,67382,67413,67431,67589,67592,67637,67640,67644,67669,67702,67742,67826,67829,67861,67897,68023,68031,68096,68115,68119,68149,68220,68252,68295,68324,68405,68437,68466,68497,68680,68899,69404,69415,69445,69622,69687,69807,69864,69926,69956,70002,70006,70066,70084,70106,70108,70161,70187,70278,70280,70285,70301,70312,70366,70412,70416,70440,70448,70451,70457,70461,70480,70497,70708,70730,70751,70831,70853,70855,71086,71131,71215,71236,71338,71352,71450,71723,71935,72103,72144,72161,72163,72192,72242,72250,72272,72329,72349,72440,72712,72750,72768,72847,72966,72969,73008,73030,73061,73064,73097,73112,73458,74649,75075,78894,83526,92728,92766,92909,92975,93047,93071,94026,94032,100343,101106,110878,110930,110951,111355,113770,113788,113800,113817,123180,123214,123627,125124,126467,126495,126498,126500,126503,126514,126519,126521,126523,126530,126535,126537,126539,126543,126546,126548,126551,126553,126555,126557,126559,126562,126564,126570,126578,126583,126588,126590,126601,126619,126627,126633,126651,173782,177972,178205,183969,191456,195101};

bool __isLoChar(uint32_t c) {
    auto index = std::lower_bound(__LoRangeA, __LoRangeA + 476, c) - __LoRangeA;
    if(c == __LoRangeA[index]) return true;
    index -= 1;
    if(index < 0) return false;
    return c >= __LoRangeA[index] && c <= __LoRangeB[index];
}