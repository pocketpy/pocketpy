/*
 *  Copyright (c) 2022 blueloveTH
 *  Distributed Under The LGPLv3 License
 */

#ifndef POCKETPY_H
#define POCKETPY_H


#ifdef _MSC_VER
#pragma warning (disable:4267)
#pragma warning (disable:4101)
#define _CRT_NONSTDC_NO_DEPRECATE
#endif

#include <sstream>
#include <regex>
#include <stack>
#include <cmath>
#include <stdexcept>
#include <vector>
#include <string>
#include <cstring>
#include <chrono>
#include <string_view>
#include <queue>
#include <iomanip>
#include <map>

#include <thread>
#include <atomic>
#include <iostream>

#ifdef POCKETPY_H
#define UNREACHABLE() throw std::runtime_error( "L" + std::to_string(__LINE__) + " UNREACHABLE()! This should be a bug, please report it");
#else
#define UNREACHABLE() throw std::runtime_error( __FILE__ + std::string(":") + std::to_string(__LINE__) + " UNREACHABLE()!");
#endif

#define PK_VERSION "0.5.0"

//#define PKPY_NO_TYPE_CHECK
//#define PKPY_NO_INDEX_CHECK


namespace pkpy{
    template <typename T>
    class shared_ptr {
        int* counter = nullptr;

#define _t() ((T*)(counter + 1))
#define _inc_counter() if(counter) ++(*counter)
#define _dec_counter() if(counter && --(*counter) == 0){ _t()->~T(); free(counter); }

    public:
        shared_ptr() {}
        shared_ptr(int* block) : counter(block) {}
        shared_ptr(const shared_ptr& other) : counter(other.counter) {
            _inc_counter();
        }
        shared_ptr(shared_ptr&& other) noexcept : counter(other.counter) {
            other.counter = nullptr;
        }
        ~shared_ptr() {
            _dec_counter();
        }

        bool operator==(const shared_ptr& other) const {
            return counter == other.counter;
        }

        bool operator!=(const shared_ptr& other) const {
            return counter != other.counter;
        }

        bool operator==(std::nullptr_t) const {
            return counter == nullptr;
        }

        bool operator!=(std::nullptr_t) const {
            return counter != nullptr;
        }

        shared_ptr& operator=(const shared_ptr& other) {
            if (this != &other) {
                _dec_counter();
                counter = other.counter;
                _inc_counter();
            }
            return *this;
        }

        shared_ptr& operator=(shared_ptr&& other) noexcept {
            if (this != &other) {
                _dec_counter();
                counter = other.counter;
                other.counter = nullptr;
            }
            return *this;
        }

        T& operator*() const {
            return *_t();
        }
        T* operator->() const {
            return _t();
        }
        T* get() const {
            return _t();
        }
        int use_count() const {
            return counter ? *counter : 0;
        }
        void reset(){
            _dec_counter();
            counter = nullptr;
        }
    };

#undef _t
#undef _inc_counter
#undef _dec_counter

    template <typename T, typename U, typename... Args>
    shared_ptr<T> make_shared(Args&&... args) {
        static_assert(std::is_base_of<T, U>::value, "U must be derived from T");
        int* p = (int*)malloc(sizeof(int) + sizeof(U));
        *p = 1;
        new(p+1) U(std::forward<Args>(args)...);
        return shared_ptr<T>(p);
    }

    template <typename T, typename... Args>
    shared_ptr<T> make_shared(Args&&... args) {
        int* p = (int*)malloc(sizeof(int) + sizeof(T));
        *p = 1;
        new(p+1) T(std::forward<Args>(args)...);
        return shared_ptr<T>(p);
    }

    template <typename T>
    class unique_ptr {
        T* ptr;

    public:
        unique_ptr() : ptr(nullptr) {}
        unique_ptr(T* ptr) : ptr(ptr) {}
        unique_ptr(const unique_ptr& other) = delete;
        unique_ptr(unique_ptr&& other) : ptr(other.ptr) {
            other.ptr = nullptr;
        }
        ~unique_ptr() {
            delete ptr;
        }

        bool operator==(const unique_ptr& other) const {
            return ptr == other.ptr;
        }

        bool operator!=(const unique_ptr& other) const {
            return ptr != other.ptr;
        }

        bool operator==(std::nullptr_t) const {
            return ptr == nullptr;
        }

        bool operator!=(std::nullptr_t) const {
            return ptr != nullptr;
        }

        unique_ptr& operator=(const unique_ptr& other) = delete;

        unique_ptr& operator=(unique_ptr&& other) {
            if (this != &other) {
                delete ptr;
                ptr = other.ptr;
                other.ptr = nullptr;
            }
            return *this;
        }

        T& operator*() const {
            return *ptr;
        }
        T* operator->() const {
            return ptr;
        }
        T* get() const {
            return ptr;
        }

        void reset(){
            delete ptr;
            ptr = nullptr;
        }
    };

    template <typename T, typename... Args>
    unique_ptr<T> make_unique(Args&&... args) {
        return unique_ptr<T>(new T(std::forward<Args>(args)...));
    }
};


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

typedef std::string_view _StrLiteral;

inline constexpr _StrLiteral operator "" _c(const char* str, size_t len){
    return _StrLiteral(str, len);
}

class _Str {
private:
    pkpy::shared_ptr<_StrMemory> _s;
public:
    _Str(_StrLiteral s){
        _s = pkpy::make_shared<_StrMemory>(std::string(s));
    }
    _Str(const char* s){
        _s = pkpy::make_shared<_StrMemory>(std::string(s));
    }
    _Str(const char* s, size_t len){
        _s = pkpy::make_shared<_StrMemory>(std::string(s, len));
    }
    _Str(){
        _s = pkpy::make_shared<_StrMemory>(std::string());
    }
    _Str(const std::string& s){
        _s = pkpy::make_shared<_StrMemory>(s);
    }
    _Str(const _Str& s) : _s(s._s) {}

    _Str(std::string&& s){
        this->_s = pkpy::make_shared<_StrMemory>(std::move(s));
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
        return *_s == *other._s;
    }

    bool operator!=(const _Str& other) const {
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
            // std::isspace(c) does not working on windows (Debug)
            return c != ' ' && c != '\t' && c != '\r' && c != '\n';
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

// const _Str& __class__ = _Str("__class__"_c);
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

const _Str& m_append = _Str("append"_c);
const _Str& m_eval = _Str("eval"_c);
const _Str& __enter__ = _Str("__enter__"_c);
const _Str& __exit__ = _Str("__exit__"_c);

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
// emhash8::HashMap for C++11/14/17
// version 1.6.3
//
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// SPDX-License-Identifier: MIT
// Copyright (c) 2019-2022 Huang Yuanbing & bailuzhou AT 163.com
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE



#include <cstring>
#include <string>
#include <cstdlib>
#include <type_traits>
#include <cassert>
#include <utility>
#include <cstdint>
#include <functional>
#include <iterator>
#include <algorithm>

#ifdef EMH_KEY
    #undef  EMH_KEY
    #undef  EMH_VAL
    #undef  EMH_KV
    #undef  EMH_BUCKET
    #undef  EMH_NEW
    #undef  EMH_EMPTY
    #undef  EMH_PREVET
    #undef  EMH_LIKELY
    #undef  EMH_UNLIKELY
#endif

// likely/unlikely
#if defined(__GNUC__) || defined(__INTEL_COMPILER) || defined(__clang__)
#    define EMH_LIKELY(condition)   __builtin_expect(condition, 1)
#    define EMH_UNLIKELY(condition) __builtin_expect(condition, 0)
#else
#    define EMH_LIKELY(condition)   (condition)
#    define EMH_UNLIKELY(condition) (condition)
#endif

#define EMH_KEY(p, n)     p[n].first
#define EMH_VAL(p, n)     p[n].second
#define EMH_KV(p, n)      p[n]

#define EMH_INDEX(i, n)   i[n]
#define EMH_BUCKET(i, n)  i[n].bucket
#define EMH_HSLOT(i, n)   i[n].slot
#define EMH_SLOT(i, n)    (i[n].slot & _mask)
#define EMH_PREVET(i, n)  i[n].slot

#define EMH_KEYMASK(key, mask)  ((size_type)(key) & ~mask)
#define EMH_EQHASH(n, key_hash) (EMH_KEYMASK(key_hash, _mask) == (_index[n].slot & ~_mask))
#define EMH_NEW(key, val, bucket, key_hash) \
    new(_pairs + _num_filled) value_type(key, val); \
    _etail = bucket; \
    _index[bucket] = {bucket, _num_filled++ | EMH_KEYMASK(key_hash, _mask)}

#define EMH_EMPTY(i, n) (0 > (int)i[n].bucket)

namespace emhash8 {

#ifndef EMH_DEFAULT_LOAD_FACTOR
    constexpr static float EMH_DEFAULT_LOAD_FACTOR = 0.80f;
    constexpr static float EMH_MIN_LOAD_FACTOR     = 0.25f; //< 0.5
#endif
#if EMH_CACHE_LINE_SIZE < 32
    constexpr static uint32_t EMH_CACHE_LINE_SIZE  = 64;
#endif

template <typename KeyT, typename ValueT, typename HashT = std::hash<KeyT>, typename EqT = std::equal_to<KeyT>>
class HashMap
{
public:
    using htype = HashMap<KeyT, ValueT, HashT, EqT>;
    using value_type = std::pair<KeyT, ValueT>;
    using key_type = KeyT;
    using mapped_type = ValueT;

#ifdef EMH_SMALL_TYPE
    using size_type = uint16_t;
#elif EMH_SIZE_TYPE == 0
    using size_type = uint32_t;
#else
    using size_type = size_t;
#endif

    using hasher = HashT;
    using key_equal = EqT;

    constexpr static size_type INACTIVE = 0-1u;
    //constexpr uint32_t END      = 0-0x1u;
    constexpr static size_type EAD      = 2;

    struct Index
    {
        size_type bucket;
        size_type slot;
    };

    class const_iterator;
    class iterator
    {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type      = typename htype::value_type;
        using pointer         = value_type*;
        using const_pointer   = const value_type* ;
        using reference       = value_type&;
        using const_reference = const value_type&;

        iterator() : kv_(nullptr) {}
        iterator(const_iterator& cit) {
            kv_ = cit.kv_;
        }

        iterator(const htype* hash_map, size_type bucket) {
            kv_ = hash_map->_pairs + (int)bucket;
        }

        iterator& operator++()
        {
            kv_ ++;
            return *this;
        }

        iterator operator++(int)
        {
            auto cur = *this; kv_ ++;
            return cur;
        }

        iterator& operator--()
        {
            kv_ --;
            return *this;
        }

        iterator operator--(int)
        {
            auto cur = *this; kv_ --;
            return cur;
        }

        reference operator*() const { return *kv_; }
        pointer operator->() const { return kv_; }

        bool operator == (const iterator& rhs) const { return kv_ == rhs.kv_; }
        bool operator != (const iterator& rhs) const { return kv_ != rhs.kv_; }
        bool operator == (const const_iterator& rhs) const { return kv_ == rhs.kv_; }
        bool operator != (const const_iterator& rhs) const { return kv_ != rhs.kv_; }

    public:
        value_type* kv_;
    };

    class const_iterator
    {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type        = typename htype::value_type;
        using difference_type   = std::ptrdiff_t;
        using pointer           = value_type*;
        using const_pointer     = const value_type*;
        using reference         = value_type&;
        using const_reference   = const value_type&;

        const_iterator(const iterator& it) {
            kv_ = it.kv_;
        }

        const_iterator (const htype* hash_map, size_type bucket) {
            kv_ = hash_map->_pairs + (int)bucket;
        }

        const_iterator& operator++()
        {
            kv_ ++;
            return *this;
        }

        const_iterator operator++(int)
        {
            auto cur = *this; kv_ ++;
            return cur;
        }

        const_iterator& operator--()
        {
            kv_ --;
            return *this;
        }

        const_iterator operator--(int)
        {
            auto cur = *this; kv_ --;
            return cur;
        }

        const_reference operator*() const { return *kv_; }
        const_pointer operator->() const { return kv_; }

        bool operator == (const iterator& rhs) const { return kv_ == rhs.kv_; }
        bool operator != (const iterator& rhs) const { return kv_ != rhs.kv_; }
        bool operator == (const const_iterator& rhs) const { return kv_ == rhs.kv_; }
        bool operator != (const const_iterator& rhs) const { return kv_ != rhs.kv_; }
    public:
        const value_type* kv_;
    };

    void init(size_type bucket, float mlf = EMH_DEFAULT_LOAD_FACTOR)
    {
        _pairs = nullptr;
        _index = nullptr;
        _mask  = _num_buckets = 0;
        _num_filled = 0;
        max_load_factor(mlf);
        rehash(bucket);
    }

    HashMap(size_type bucket = 2, float mlf = EMH_DEFAULT_LOAD_FACTOR)
    {
        init(bucket, mlf);
    }

    HashMap(const HashMap& rhs)
    {
        if (rhs.load_factor() > EMH_MIN_LOAD_FACTOR) {
            _pairs = alloc_bucket((size_type)(rhs._num_buckets * rhs.max_load_factor()) + 4);
            _index = alloc_index(rhs._num_buckets);
            clone(rhs);
        } else {
            init(rhs._num_filled + 2, EMH_DEFAULT_LOAD_FACTOR);
            for (auto it = rhs.begin(); it != rhs.end(); ++it)
                insert_unique(it->first, it->second);
        }
    }

    HashMap(HashMap&& rhs) noexcept
    {
        init(0);
        *this = std::move(rhs);
    }

    HashMap(std::initializer_list<value_type> ilist)
    {
        init((size_type)ilist.size());
        for (auto it = ilist.begin(); it != ilist.end(); ++it)
            do_insert(*it);
    }

    template<class InputIt>
    HashMap(InputIt first, InputIt last, size_type bucket_count=4)
    {
        init(std::distance(first, last) + bucket_count);
        for (; first != last; ++first)
            emplace(*first);
    }

    HashMap& operator=(const HashMap& rhs)
    {
        if (this == &rhs)
            return *this;

        if (rhs.load_factor() < EMH_MIN_LOAD_FACTOR) {
            clear(); free(_pairs); _pairs = nullptr;
            rehash(rhs._num_filled + 2);
            for (auto it = rhs.begin(); it != rhs.end(); ++it)
                insert_unique(it->first, it->second);
            return *this;
        }

        clearkv();

        if (_num_buckets != rhs._num_buckets) {
            free(_pairs); free(_index);
            _index = alloc_index(rhs._num_buckets);
            _pairs = alloc_bucket((size_type)(rhs._num_buckets * rhs.max_load_factor()) + 4);
        }

        clone(rhs);
        return *this;
    }

    HashMap& operator=(HashMap&& rhs) noexcept
    {
        if (this != &rhs) {
            swap(rhs);
            rhs.clear();
        }
        return *this;
    }

    template<typename Con>
    bool operator == (const Con& rhs) const
    {
        if (size() != rhs.size())
            return false;

        for (auto it = begin(), last = end(); it != last; ++it) {
            auto oi = rhs.find(it->first);
            if (oi == rhs.end() || it->second != oi->second)
                return false;
        }
        return true;
    }

    template<typename Con>
    bool operator != (const Con& rhs) const { return !(*this == rhs); }

    ~HashMap() noexcept
    {
        clearkv();
        free(_pairs);
        free(_index);
    }

    void clone(const HashMap& rhs)
    {
        _hasher      = rhs._hasher;
//        _eq          = rhs._eq;
        _num_buckets = rhs._num_buckets;
        _num_filled  = rhs._num_filled;
        _mlf         = rhs._mlf;
        _last        = rhs._last;
        _mask        = rhs._mask;
#if EMH_HIGH_LOAD
        _ehead       = rhs._ehead;
#endif
        _etail       = rhs._etail;

        auto opairs  = rhs._pairs;
        memcpy((char*)_index, (char*)rhs._index, (_num_buckets + EAD) * sizeof(Index));

        if (is_copy_trivially()) {
            if (opairs)
                memcpy((char*)_pairs, (char*)opairs, _num_filled * sizeof(value_type));
        } else {
            for (size_type slot = 0; slot < _num_filled; slot++)
                new(_pairs + slot) value_type(opairs[slot]);
        }
    }

    void swap(HashMap& rhs)
    {
        //      std::swap(_eq, rhs._eq);
        std::swap(_hasher, rhs._hasher);
        std::swap(_pairs, rhs._pairs);
        std::swap(_index, rhs._index);
        std::swap(_num_buckets, rhs._num_buckets);
        std::swap(_num_filled, rhs._num_filled);
        std::swap(_mask, rhs._mask);
        std::swap(_mlf, rhs._mlf);
        std::swap(_last, rhs._last);
#if EMH_HIGH_LOAD
        std::swap(_ehead, rhs._ehead);
#endif
        std::swap(_etail, rhs._etail);
    }

    // -------------------------------------------------------------
    inline iterator first() const { return {this, 0}; }
    inline iterator last() const { return {this, _num_filled - 1}; }

    inline iterator begin() { return first(); }
    inline const_iterator cbegin() const { return first(); }
    inline const_iterator begin() const { return first(); }

    inline iterator end() { return {this, _num_filled}; }
    inline const_iterator cend() const { return {this, _num_filled}; }
    inline const_iterator end() const { return cend(); }

    inline const value_type* values() const { return _pairs; }
    inline const Index* index() const { return _index; }

    inline size_type size() const { return _num_filled; }
    inline bool empty() const { return _num_filled == 0; }
    inline size_type bucket_count() const { return _num_buckets; }

    /// Returns average number of elements per bucket.
    inline float load_factor() const { return static_cast<float>(_num_filled) / (_mask + 1); }

    inline HashT& hash_function() const { return _hasher; }
    inline EqT& key_eq() const { return _eq; }

    void max_load_factor(float mlf)
    {
        if (mlf < 0.991 && mlf > EMH_MIN_LOAD_FACTOR) {
            _mlf = (uint32_t)((1 << 27) / mlf);
            if (_num_buckets > 0) rehash(_num_buckets);
        }
    }

    inline constexpr float max_load_factor() const { return (1 << 27) / (float)_mlf; }
    inline constexpr size_type max_size() const { return (1ull << (sizeof(size_type) * 8 - 1)); }
    inline constexpr size_type max_bucket_count() const { return max_size(); }

#if EMH_STATIS
    //Returns the bucket number where the element with key k is located.
    size_type bucket(const KeyT& key) const
    {
        const auto bucket = hash_bucket(key);
        const auto next_bucket = EMH_BUCKET(_index, bucket);
        if ((int)next_bucket < 0)
            return 0;
        else if (bucket == next_bucket)
            return bucket + 1;

        return hash_main(bucket) + 1;
    }

    //Returns the number of elements in bucket n.
    size_type bucket_size(const size_type bucket) const
    {
        auto next_bucket = EMH_BUCKET(_index, bucket);
        if ((int)next_bucket < 0)
            return 0;

        next_bucket = hash_main(bucket);
        size_type ibucket_size = 1;

        //iterator each item in current main bucket
        while (true) {
            const auto nbucket = EMH_BUCKET(_index, next_bucket);
            if (nbucket == next_bucket) {
                break;
            }
            ibucket_size ++;
            next_bucket = nbucket;
        }
        return ibucket_size;
    }

    size_type get_main_bucket(const size_type bucket) const
    {
        auto next_bucket = EMH_BUCKET(_index, bucket);
        if ((int)next_bucket < 0)
            return INACTIVE;

        return hash_main(bucket);
    }

    size_type get_diss(size_type bucket, size_type next_bucket, const size_type slots) const
    {
        auto pbucket = reinterpret_cast<uint64_t>(&_pairs[bucket]);
        auto pnext   = reinterpret_cast<uint64_t>(&_pairs[next_bucket]);
        if (pbucket / EMH_CACHE_LINE_SIZE == pnext / EMH_CACHE_LINE_SIZE)
            return 0;
        size_type diff = pbucket > pnext ? (pbucket - pnext) : (pnext - pbucket);
        if (diff / EMH_CACHE_LINE_SIZE < slots - 1)
            return diff / EMH_CACHE_LINE_SIZE + 1;
        return slots - 1;
    }

    int get_bucket_info(const size_type bucket, size_type steps[], const size_type slots) const
    {
        auto next_bucket = EMH_BUCKET(_index, bucket);
        if ((int)next_bucket < 0)
            return -1;

        const auto main_bucket = hash_main(bucket);
        if (next_bucket == main_bucket)
            return 1;
        else if (main_bucket != bucket)
            return 0;

        steps[get_diss(bucket, next_bucket, slots)] ++;
        size_type ibucket_size = 2;
        //find a empty and linked it to tail
        while (true) {
            const auto nbucket = EMH_BUCKET(_index, next_bucket);
            if (nbucket == next_bucket)
                break;

            steps[get_diss(nbucket, next_bucket, slots)] ++;
            ibucket_size ++;
            next_bucket = nbucket;
        }
        return (int)ibucket_size;
    }

    void dump_statics() const
    {
        const size_type slots = 128;
        size_type buckets[slots + 1] = {0};
        size_type steps[slots + 1]   = {0};
        for (size_type bucket = 0; bucket < _num_buckets; ++bucket) {
            auto bsize = get_bucket_info(bucket, steps, slots);
            if (bsize > 0)
                buckets[bsize] ++;
        }

        size_type sumb = 0, collision = 0, sumc = 0, finds = 0, sumn = 0;
        puts("============== buckets size ration =========");
        for (size_type i = 0; i < sizeof(buckets) / sizeof(buckets[0]); i++) {
            const auto bucketsi = buckets[i];
            if (bucketsi == 0)
                continue;
            sumb += bucketsi;
            sumn += bucketsi * i;
            collision += bucketsi * (i - 1);
            finds += bucketsi * i * (i + 1) / 2;
            printf("  %2u  %8u  %2.2lf|  %.2lf\n", i, bucketsi, bucketsi * 100.0 * i / _num_filled, sumn * 100.0 / _num_filled);
        }

        puts("========== collision miss ration ===========");
        for (size_type i = 0; i < sizeof(steps) / sizeof(steps[0]); i++) {
            sumc += steps[i];
            if (steps[i] <= 2)
                continue;
            printf("  %2u  %8u  %.2lf  %.2lf\n", i, steps[i], steps[i] * 100.0 / collision, sumc * 100.0 / collision);
        }

        if (sumb == 0)  return;
        printf("    _num_filled/bucket_size/packed collision/cache_miss/hit_find = %u/%.2lf/%zd/ %.2lf%%/%.2lf%%/%.2lf\n",
                _num_filled, _num_filled * 1.0 / sumb, sizeof(value_type), (collision * 100.0 / _num_filled), (collision - steps[0]) * 100.0 / _num_filled, finds * 1.0 / _num_filled);
        assert(sumn == _num_filled);
        assert(sumc == collision);
        puts("============== buckets size end =============");
    }
#endif

    // ------------------------------------------------------------
    template<typename K=KeyT>
    inline iterator find(const K& key) noexcept
    {
        return {this, find_filled_slot(key)};
    }

    template<typename K=KeyT>
    inline const_iterator find(const K& key) const noexcept
    {
        return {this, find_filled_slot(key)};
    }

    template<typename K=KeyT>
    ValueT& at(const K& key)
    {
        const auto slot = find_filled_slot(key);
        //throw
        return EMH_VAL(_pairs, slot);
    }

    template<typename K=KeyT>
    const ValueT& at(const K& key) const
    {
        const auto slot = find_filled_slot(key);
        //throw
        return EMH_VAL(_pairs, slot);
    }

    template<typename K=KeyT>
    inline bool contains(const K& key) const noexcept
    {
        return find_filled_slot(key) != _num_filled;
    }

    template<typename K=KeyT>
    inline size_type count(const K& key) const noexcept
    {
        return find_filled_slot(key) == _num_filled ? 0 : 1;
        //return find_sorted_bucket(key) == END ? 0 : 1;
        //return find_hash_bucket(key) == END ? 0 : 1;
    }

    template<typename K=KeyT>
    std::pair<iterator, iterator> equal_range(const K& key)
    {
        const auto found = find(key);
        if (found.second == _num_filled)
            return { found, found };
        else
            return { found, std::next(found) };
    }

    void merge(HashMap& rhs)
    {
        if (empty()) {
            *this = std::move(rhs);
            return;
        }

        for (auto rit = rhs.begin(); rit != rhs.end(); ) {
            auto fit = find(rit->first);
            if (fit == end()) {
                insert_unique(rit->first, std::move(rit->second));
                rit = rhs.erase(rit);
            } else {
                ++rit;
            }
        }
    }

    /// Returns the matching ValueT or nullptr if k isn't found.
    bool try_get(const KeyT& key, ValueT& val) const noexcept
    {
        const auto slot = find_filled_slot(key);
        const auto found = slot != _num_filled;
        if (found) {
            val = EMH_VAL(_pairs, slot);
        }
        return found;
    }

    /// Returns the matching ValueT or nullptr if k isn't found.
    ValueT* try_get(const KeyT& key) noexcept
    {
        const auto slot = find_filled_slot(key);
        return slot != _num_filled ? &EMH_VAL(_pairs, slot) : nullptr;
    }

    /// Const version of the above
    ValueT* try_get(const KeyT& key) const noexcept
    {
        const auto slot = find_filled_slot(key);
        return slot != _num_filled ? &EMH_VAL(_pairs, slot) : nullptr;
    }

    /// set value if key exist
    bool try_set(const KeyT& key, const ValueT& val) noexcept
    {
        const auto slot = find_filled_slot(key);
        if (slot == _num_filled)
            return false;

        EMH_VAL(_pairs, slot) = val;
        return true;
    }

    /// set value if key exist
    bool try_set(const KeyT& key, ValueT&& val) noexcept
    {
        const auto slot = find_filled_slot(key);
        if (slot == _num_filled)
            return false;

        EMH_VAL(_pairs, slot) = std::move(val);
        return true;
    }

    /// Convenience function.
    ValueT get_or_return_default(const KeyT& key) const noexcept
    {
        const auto slot = find_filled_slot(key);
        return slot == _num_filled ? ValueT() : EMH_VAL(_pairs, slot);
    }

    // -----------------------------------------------------
    std::pair<iterator, bool> do_insert(const value_type& value) noexcept
    {
        const auto key_hash = hash_key(value.first);
        const auto bucket = find_or_allocate(value.first, key_hash);
        const auto bempty  = EMH_EMPTY(_index, bucket);
        if (bempty) {
            EMH_NEW(value.first, value.second, bucket, key_hash);
        }

        const auto slot = EMH_SLOT(_index, bucket);
        return { {this, slot}, bempty };
    }

    std::pair<iterator, bool> do_insert(value_type&& value) noexcept
    {
        const auto key_hash = hash_key(value.first);
        const auto bucket = find_or_allocate(value.first, key_hash);
        const auto bempty  = EMH_EMPTY(_index, bucket);
        if (bempty) {
            EMH_NEW(std::move(value.first), std::move(value.second), bucket, key_hash);
        }

        const auto slot = EMH_SLOT(_index, bucket);
        return { {this, slot}, bempty };
    }

    template<typename K, typename V>
    std::pair<iterator, bool> do_insert(K&& key, V&& val) noexcept
    {
        const auto key_hash = hash_key(key);
        const auto bucket = find_or_allocate(key, key_hash);
        const auto bempty = EMH_EMPTY(_index, bucket);
        if (bempty) {
            EMH_NEW(std::forward<K>(key), std::forward<V>(val), bucket, key_hash);
        }

        const auto slot = EMH_SLOT(_index, bucket);
        return { {this, slot}, bempty };
    }

    template<typename K, typename V>
    std::pair<iterator, bool> do_assign(K&& key, V&& val) noexcept
    {
        check_expand_need();
        const auto key_hash = hash_key(key);
        const auto bucket = find_or_allocate(key, key_hash);
        const auto bempty = EMH_EMPTY(_index, bucket);
        if (bempty) {
            EMH_NEW(std::forward<K>(key), std::forward<V>(val), bucket, key_hash);
        } else {
            EMH_VAL(_pairs, EMH_SLOT(_index, bucket)) = std::move(val);
        }

        const auto slot = EMH_SLOT(_index, bucket);
        return { {this, slot}, bempty };
    }

    std::pair<iterator, bool> insert(const value_type& p)
    {
        check_expand_need();
        return do_insert(p);
    }

    std::pair<iterator, bool> insert(value_type && p)
    {
        check_expand_need();
        return do_insert(std::move(p));
    }

    void insert(std::initializer_list<value_type> ilist)
    {
        reserve(ilist.size() + _num_filled, false);
        for (auto it = ilist.begin(); it != ilist.end(); ++it)
            do_insert(*it);
    }

    template <typename Iter>
    void insert(Iter first, Iter last)
    {
        reserve(std::distance(first, last) + _num_filled, false);
        for (; first != last; ++first)
            do_insert(first->first, first->second);
    }

#if 0
    template <typename Iter>
    void insert_unique(Iter begin, Iter end)
    {
        reserve(std::distance(begin, end) + _num_filled, false);
        for (; begin != end; ++begin) {
            insert_unique(*begin);
        }
    }
#endif

    template<typename K, typename V>
    size_type insert_unique(K&& key, V&& val)
    {
        check_expand_need();
        const auto key_hash = hash_key(key);
        auto bucket = find_unique_bucket(key_hash);
        EMH_NEW(std::forward<K>(key), std::forward<V>(val), bucket, key_hash);
        return bucket;
    }

    size_type insert_unique(value_type&& value)
    {
        return insert_unique(std::move(value.first), std::move(value.second));
    }

    inline size_type insert_unique(const value_type& value)
    {
        return insert_unique(value.first, value.second);
    }

    template <class... Args>
    inline std::pair<iterator, bool> emplace(Args&&... args) noexcept
    {
        check_expand_need();
        return do_insert(std::forward<Args>(args)...);
    }

    //no any optimize for position
    template <class... Args>
    iterator emplace_hint(const_iterator hint, Args&&... args)
    {
        (void)hint;
        check_expand_need();
        return do_insert(std::forward<Args>(args)...).first;
    }

    template<class... Args>
    std::pair<iterator, bool> try_emplace(const KeyT& k, Args&&... args)
    {
        check_expand_need();
        return do_insert(k, std::forward<Args>(args)...);
    }

    template<class... Args>
    std::pair<iterator, bool> try_emplace(KeyT&& k, Args&&... args)
    {
        check_expand_need();
        return do_insert(std::move(k), std::forward<Args>(args)...);
    }

    template <class... Args>
    inline size_type emplace_unique(Args&&... args)
    {
        return insert_unique(std::forward<Args>(args)...);
    }

    std::pair<iterator, bool> insert_or_assign(const KeyT& key, ValueT&& val) { return do_assign(key, std::forward<ValueT>(val)); }
    std::pair<iterator, bool> insert_or_assign(KeyT&& key, ValueT&& val) { return do_assign(std::move(key), std::forward<ValueT>(val)); }

    /// Return the old value or ValueT() if it didn't exist.
    ValueT set_get(const KeyT& key, const ValueT& val)
    {
        check_expand_need();
        const auto key_hash = hash_key(key);
        const auto bucket = find_or_allocate(key, key_hash);
        if (EMH_EMPTY(_index, bucket)) {
            EMH_NEW(key, val, bucket, key_hash);
            return ValueT();
        } else {
            const auto slot = EMH_SLOT(_index, bucket);
            ValueT old_value(val);
            std::swap(EMH_VAL(_pairs, slot), old_value);
            return old_value;
        }
    }

    /// Like std::map<KeyT, ValueT>::operator[].
    ValueT& operator[](const KeyT& key) noexcept
    {
        check_expand_need();
        const auto key_hash = hash_key(key);
        const auto bucket = find_or_allocate(key, key_hash);
        if (EMH_EMPTY(_index, bucket)) {
            /* Check if inserting a value rather than overwriting an old entry */
            EMH_NEW(key, std::move(ValueT()), bucket, key_hash);
        }

        const auto slot = EMH_SLOT(_index, bucket);
        return EMH_VAL(_pairs, slot);
    }

    ValueT& operator[](KeyT&& key) noexcept
    {
        check_expand_need();
        const auto key_hash = hash_key(key);
        const auto bucket = find_or_allocate(key, key_hash);
        if (EMH_EMPTY(_index, bucket)) {
            EMH_NEW(std::move(key), std::move(ValueT()), bucket, key_hash);
        }

        const auto slot = EMH_SLOT(_index, bucket);
        return EMH_VAL(_pairs, slot);
    }

    /// Erase an element from the hash table.
    /// return 0 if element was not found
    size_type erase(const KeyT& key) noexcept
    {
        const auto key_hash = hash_key(key);
        const auto sbucket = find_filled_bucket(key, key_hash);
        if (sbucket == INACTIVE)
            return 0;

        const auto main_bucket = key_hash & _mask;
        erase_slot(sbucket, (size_type)main_bucket);
        return 1;
    }

    //iterator erase(const_iterator begin_it, const_iterator end_it)
    iterator erase(const const_iterator& cit) noexcept
    {
        const auto slot = (size_type)(cit.kv_ - _pairs);
        size_type main_bucket;
        const auto sbucket = find_slot_bucket(slot, main_bucket); //TODO
        erase_slot(sbucket, main_bucket);
        return {this, slot};
    }

    //only last >= first
    iterator erase(const_iterator first, const_iterator last) noexcept
    {
        auto esize = long(last.kv_ - first.kv_);
        auto tsize = long((_pairs + _num_filled) - last.kv_); //last to tail size
        auto next = first;
        while (tsize -- > 0) {
            if (esize-- <= 0)
                break;
            next = ++erase(next);
        }

        //fast erase from last
        next = this->last();
        while (esize -- > 0)
            next = --erase(next);

        return {this, size_type(next.kv_ - _pairs)};
    }

    template<typename Pred>
    size_type erase_if(Pred pred)
    {
        auto old_size = size();
        for (auto it = begin(); it != end();) {
            if (pred(*it))
                it = erase(it);
            else
                ++it;
        }
        return old_size - size();
    }

    static constexpr bool is_triviall_destructable()
    {
#if __cplusplus >= 201402L || _MSC_VER > 1600
        return !(std::is_trivially_destructible<KeyT>::value && std::is_trivially_destructible<ValueT>::value);
#else
        return !(std::is_pod<KeyT>::value && std::is_pod<ValueT>::value);
#endif
    }

    static constexpr bool is_copy_trivially()
    {
#if __cplusplus >= 201103L || _MSC_VER > 1600
        return (std::is_trivially_copyable<KeyT>::value && std::is_trivially_copyable<ValueT>::value);
#else
        return (std::is_pod<KeyT>::value && std::is_pod<ValueT>::value);
#endif
    }

    void clearkv()
    {
        if (is_triviall_destructable()) {
            while (_num_filled --)
                _pairs[_num_filled].~value_type();
        }
    }

    /// Remove all elements, keeping full capacity.
    void clear() noexcept
    {
        clearkv();

        if (_num_filled > 0)
            memset((char*)_index, INACTIVE, sizeof(_index[0]) * _num_buckets);

        _last = _num_filled = 0;
        _etail = INACTIVE;

#if EMH_HIGH_LOAD
        _ehead = 0;
#endif
    }

    void shrink_to_fit(const float min_factor = EMH_DEFAULT_LOAD_FACTOR / 4)
    {
        if (load_factor() < min_factor && bucket_count() > 10) //safe guard
            rehash(_num_filled + 1);
    }

#if EMH_HIGH_LOAD
    void set_empty()
    {
        auto prev = 0;
        for (int32_t bucket = 1; bucket < _num_buckets; ++bucket) {
            if (EMH_EMPTY(_index, bucket)) {
                if (prev != 0) {
                    EMH_PREVET(_index, bucket) = prev;
                    EMH_BUCKET(_index, prev) = -bucket;
                }
                else
                    _ehead = bucket;
                prev = bucket;
            }
        }

        EMH_PREVET(_index, _ehead) = prev;
        EMH_BUCKET(_index, prev) = 0-_ehead;
        _ehead = 0-EMH_BUCKET(_index, _ehead);
    }

    void clear_empty()
    {
        auto prev = EMH_PREVET(_index, _ehead);
        while (prev != _ehead) {
            EMH_BUCKET(_index, prev) = INACTIVE;
            prev = EMH_PREVET(_index, prev);
        }
        EMH_BUCKET(_index, _ehead) = INACTIVE;
        _ehead = 0;
    }

    //prev-ehead->next
    size_type pop_empty(const size_type bucket)
    {
        const auto prev_bucket = EMH_PREVET(_index, bucket);
        const int next_bucket = 0-EMH_BUCKET(_index, bucket);

        EMH_PREVET(_index, next_bucket) = prev_bucket;
        EMH_BUCKET(_index, prev_bucket) = -next_bucket;

        _ehead = next_bucket;
        return bucket;
    }

    //ehead->bucket->next
    void push_empty(const int32_t bucket)
    {
        const int next_bucket = 0-EMH_BUCKET(_index, _ehead);
        assert(next_bucket > 0);

        EMH_PREVET(_index, bucket) = _ehead;
        EMH_BUCKET(_index, bucket) = -next_bucket;

        EMH_PREVET(_index, next_bucket) = bucket;
        EMH_BUCKET(_index, _ehead) = -bucket;
        //        _ehead = bucket;
    }
#endif

    /// Make room for this many elements
    bool reserve(uint64_t num_elems, bool force)
    {
        (void)force;
#if EMH_HIGH_LOAD == 0
        const auto required_buckets = num_elems * _mlf >> 27;
        if (EMH_LIKELY(required_buckets < _mask)) // && !force
            return false;

#elif EMH_HIGH_LOAD
        const auto required_buckets = num_elems + num_elems * 1 / 9;
        if (EMH_LIKELY(required_buckets < _mask))
            return false;

        else if (_num_buckets < 16 && _num_filled < _num_buckets)
            return false;

        else if (_num_buckets > EMH_HIGH_LOAD) {
            if (_ehead == 0) {
                set_empty();
                return false;
            } else if (/*_num_filled + 100 < _num_buckets && */EMH_BUCKET(_index, _ehead) != 0-_ehead) {
                return false;
            }
        }
#endif
#if EMH_STATIS
        if (_num_filled > EMH_STATIS) dump_statics();
#endif

        //assert(required_buckets < max_size());
        rehash(required_buckets + 2);
        return true;
    }

    static value_type* alloc_bucket(size_type num_buckets)
    {
        auto new_pairs = (char*)malloc((uint64_t)num_buckets * sizeof(value_type));
        return (value_type *)(new_pairs);
    }

    static Index* alloc_index(size_type num_buckets)
    {
        auto new_index = (char*)malloc((uint64_t)(EAD + num_buckets) * sizeof(Index));
        return (Index *)(new_index);
    }

    bool reserve(size_type required_buckets) noexcept
    {
        if (_num_filled != required_buckets)
            return reserve(required_buckets, true);

        _last = 0;
#if EMH_HIGH_LOAD
        _ehead = 0;
#endif

#if EMH_SORT
        std::sort(_pairs, _pairs + _num_filled, [this](const value_type & l, const value_type & r) {
            const auto hashl = (size_type)hash_key(l.first) & _mask, hashr = (size_type)hash_key(r.first) & _mask;
            return hashl < hashr;
            //return l.first < r.first;
        });
#endif

        memset((char*)_index, INACTIVE, sizeof(_index[0]) * _num_buckets);
        for (size_type slot = 0; slot < _num_filled; slot++) {
            const auto& key = EMH_KEY(_pairs, slot);
            const auto key_hash = hash_key(key);
            const auto bucket = size_type(key_hash & _mask);
            auto& next_bucket = EMH_BUCKET(_index, bucket);
            if ((int)next_bucket < 0)
                EMH_INDEX(_index, bucket) = {1, slot | EMH_KEYMASK(key_hash, _mask)};
            else {
                EMH_HSLOT(_index, bucket) |= EMH_KEYMASK(key_hash, _mask);
                next_bucket ++;
            }
        }
        return true;
    }

    void rebuild(size_type num_buckets) noexcept
    {
        free(_index);
        auto new_pairs = (value_type*)alloc_bucket((size_type)(num_buckets * max_load_factor()) + 4);
        if (is_copy_trivially()) {
            memcpy((char*)new_pairs, (char*)_pairs, _num_filled * sizeof(value_type));
        } else {
            for (size_type slot = 0; slot < _num_filled; slot++) {
                new(new_pairs + slot) value_type(std::move(_pairs[slot]));
                if (is_triviall_destructable())
                    _pairs[slot].~value_type();
            }
        }
        free(_pairs);
        _pairs = new_pairs;
        _index = (Index*)alloc_index (num_buckets);

        memset((char*)_index, INACTIVE, sizeof(_index[0]) * num_buckets);
        memset((char*)(_index + num_buckets), 0, sizeof(_index[0]) * EAD);
    }

    void rehash(uint64_t required_buckets)
    {
        if (required_buckets < _num_filled)
            return;

        assert(required_buckets < max_size());
        auto num_buckets = _num_filled > (1u << 16) ? (1u << 16) : 4u;
        while (num_buckets < required_buckets) { num_buckets *= 2; }

#if EMH_REHASH_LOG
        auto last = _last;
        size_type collision = 0;
#endif

#if EMH_HIGH_LOAD
        _ehead = 0;
#endif
        _last = _mask / 4;

        _mask        = num_buckets - 1;
#if EMH_PACK_TAIL > 1
        _last = _mask;
        num_buckets += num_buckets * EMH_PACK_TAIL / 100; //add more 5-10%
#endif
        _num_buckets = num_buckets;

        rebuild(num_buckets);

#ifdef EMH_SORT
        std::sort(_pairs, _pairs + _num_filled, [this](const value_type & l, const value_type & r) {
            const auto hashl = hash_key(l.first), hashr = hash_key(r.first);
            auto diff = int64_t((hashl & _mask) - (hashr & _mask));
            if (diff != 0)
                return diff < 0;
            return hashl < hashr;
//          return l.first < r.first;
        });
#endif

        _etail = INACTIVE;
        for (size_type slot = 0; slot < _num_filled; ++slot) {
            const auto& key = EMH_KEY(_pairs, slot);
            const auto key_hash = hash_key(key);
            const auto bucket = find_unique_bucket(key_hash);
            EMH_INDEX(_index, bucket) = {bucket, slot | EMH_KEYMASK(key_hash, _mask)};

#if EMH_REHASH_LOG
            if (bucket != hash_main(bucket))
                collision ++;
#endif
        }

#if EMH_REHASH_LOG
        if (_num_filled > EMH_REHASH_LOG) {
            auto mbucket = _num_filled - collision;
            char buff[255] = {0};
            sprintf(buff, "    _num_filled/aver_size/K.V/pack/collision|last = %u/%.2lf/%s.%s/%zd|%.2lf%%,%.2lf%%",
                    _num_filled, double (_num_filled) / mbucket, typeid(KeyT).name(), typeid(ValueT).name(), sizeof(_pairs[0]), collision * 100.0 / _num_filled, last * 100.0 / _num_buckets);
#ifdef EMH_LOG
            static uint32_t ihashs = 0; EMH_LOG() << "hash_nums = " << ihashs ++ << "|" <<__FUNCTION__ << "|" << buff << endl;
#else
            puts(buff);
#endif
        }
#endif
    }

private:
    // Can we fit another element?
    inline bool check_expand_need()
    {
        return reserve(_num_filled, false);
    }

    size_type slot_to_bucket(const size_type slot) const noexcept
    {
        size_type main_bucket;
        return find_slot_bucket(slot, main_bucket); //TODO
    }

    //very slow
    void erase_slot(const size_type sbucket, const size_type main_bucket) noexcept
    {
        const auto slot = EMH_SLOT(_index, sbucket);
        const auto ebucket = erase_bucket(sbucket, main_bucket);
        const auto last_slot = --_num_filled;
        if (EMH_LIKELY(slot != last_slot)) {
            const auto last_bucket = (_etail == INACTIVE || ebucket == _etail)
                ? slot_to_bucket(last_slot) : _etail;

            EMH_KV(_pairs, slot) = std::move(EMH_KV(_pairs, last_slot));
            EMH_HSLOT(_index, last_bucket) = slot | (EMH_HSLOT(_index, last_bucket) & ~_mask);
        }

        if (is_triviall_destructable())
            _pairs[last_slot].~value_type();

        _etail = INACTIVE;
        EMH_INDEX(_index, ebucket) = {INACTIVE, 0};
#if EMH_HIGH_LOAD
        if (_ehead) {
            if (10 * _num_filled < 8 * _num_buckets)
                clear_empty();
            else if (ebucket)
                push_empty(ebucket);
        }
#endif
    }

    size_type erase_bucket(const size_type bucket, const size_type main_bucket) noexcept
    {
        const auto next_bucket = EMH_BUCKET(_index, bucket);
        if (bucket == main_bucket) {
            if (main_bucket != next_bucket) {
                const auto nbucket = EMH_BUCKET(_index, next_bucket);
                EMH_INDEX(_index, main_bucket) = {
                    (nbucket == next_bucket) ? main_bucket : nbucket,
                    EMH_HSLOT(_index, next_bucket)
                };
            }
            return next_bucket;
        }

        const auto prev_bucket = find_prev_bucket(main_bucket, bucket);
        EMH_BUCKET(_index, prev_bucket) = (bucket == next_bucket) ? prev_bucket : next_bucket;
        return bucket;
    }

    // Find the slot with this key, or return bucket size
    size_type find_slot_bucket(const size_type slot, size_type& main_bucket) const
    {
        const auto key_hash = hash_key(EMH_KEY(_pairs, slot));
        const auto bucket = main_bucket = size_type(key_hash & _mask);
        if (slot == EMH_SLOT(_index, bucket))
            return bucket;

        auto next_bucket = EMH_BUCKET(_index, bucket);
        while (true) {
            if (EMH_LIKELY(slot == EMH_SLOT(_index, next_bucket)))
                return next_bucket;
            next_bucket = EMH_BUCKET(_index, next_bucket);
        }

        return INACTIVE;
    }

    // Find the slot with this key, or return bucket size
    size_type find_filled_bucket(const KeyT& key, uint64_t key_hash) const noexcept
    {
        const auto bucket = size_type(key_hash & _mask);
        auto next_bucket  = EMH_BUCKET(_index, bucket);
        if (EMH_UNLIKELY((int)next_bucket < 0))
            return INACTIVE;

        if (EMH_EQHASH(bucket, key_hash)) {
            const auto slot = EMH_SLOT(_index, bucket);
            if (EMH_LIKELY(_eq(key, EMH_KEY(_pairs, slot))))
                return bucket;
        }
        if (next_bucket == bucket)
            return INACTIVE;

        while (true) {
            if (EMH_EQHASH(next_bucket, key_hash)) {
                const auto slot = EMH_SLOT(_index, next_bucket);
                if (EMH_LIKELY(_eq(key, EMH_KEY(_pairs, slot))))
                    return next_bucket;
            }

            const auto nbucket = EMH_BUCKET(_index, next_bucket);
            if (nbucket == next_bucket)
                return INACTIVE;
            next_bucket = nbucket;
        }

        return INACTIVE;
    }

    // Find the slot with this key, or return bucket size
    template<typename K=KeyT>
    size_type find_filled_slot(const K& key) const noexcept
    {
        const auto key_hash = hash_key(key);
        const auto bucket = size_type(key_hash & _mask);
        auto next_bucket = EMH_BUCKET(_index, bucket);
        if ((int)next_bucket < 0)
            return _num_filled;

        if (EMH_EQHASH(bucket, key_hash)) {
            const auto slot = EMH_SLOT(_index, bucket);
            if (EMH_LIKELY(_eq(key, EMH_KEY(_pairs, slot))))
                return slot;
        }
        if (next_bucket == bucket)
            return _num_filled;

        while (true) {
            if (EMH_EQHASH(next_bucket, key_hash)) {
                const auto slot = EMH_SLOT(_index, next_bucket);
                if (EMH_LIKELY(_eq(key, EMH_KEY(_pairs, slot))))
                    return slot;
            }

            const auto nbucket = EMH_BUCKET(_index, next_bucket);
            if (nbucket == next_bucket)
                return _num_filled;
            next_bucket = nbucket;
        }

        return _num_filled;
    }

#if EMH_SORT
    size_type find_hash_bucket(const KeyT& key) const noexcept
    {
        const auto key_hash = hash_key(key);
        const auto bucket = size_type(key_hash & _mask);
        const auto next_bucket = EMH_BUCKET(_index, bucket);
        if ((int)next_bucket < 0)
            return END;

        auto slot = EMH_SLOT(_index, bucket);
        if (_eq(key, EMH_KEY(_pairs, slot++)))
            return slot;
        else if (next_bucket == bucket)
            return END;

        while (true) {
            const auto& okey = EMH_KEY(_pairs, slot++);
            if (_eq(key, okey))
                return slot;

            const auto hasho = hash_key(okey);
            if ((hasho & _mask) != bucket)
                break;
            else if (hasho > key_hash)
                break;
            else if (EMH_UNLIKELY(slot >= _num_filled))
                break;
        }

        return END;
    }

    //only for find/can not insert
    size_type find_sorted_bucket(const KeyT& key) const noexcept
    {
        const auto key_hash = hash_key(key);
        const auto bucket = size_type(key_hash & _mask);
        const auto slots = (int)(EMH_BUCKET(_index, bucket)); //TODO
        if (slots < 0 /**|| key < EMH_KEY(_pairs, slot)*/)
            return END;

        const auto slot = EMH_SLOT(_index, bucket);
        auto ormask = _index[bucket].slot & ~_mask;
        auto hmask  = EMH_KEYMASK(key_hash, _mask);
        if ((hmask | ormask) != ormask)
            return END;

        if (_eq(key, EMH_KEY(_pairs, slot)))
            return slot;
        else if (slots == 1 || key < EMH_KEY(_pairs, slot))
            return END;

#if EMH_SORT
        if (key < EMH_KEY(_pairs, slot) || key > EMH_KEY(_pairs, slots + slot - 1))
            return END;
#endif

        for (size_type i = 1; i < slots; ++i) {
            const auto& okey = EMH_KEY(_pairs, slot + i);
            if (_eq(key, okey))
                return slot + i;
//            else if (okey > key)
//                return END;
        }

        return END;
    }
#endif

    //kick out bucket and find empty to occpuy
    //it will break the orgin link and relnik again.
    //before: main_bucket-->prev_bucket --> bucket   --> next_bucket
    //atfer : main_bucket-->prev_bucket --> (removed)--> new_bucket--> next_bucket
    size_type kickout_bucket(const size_type kmain, const size_type bucket) noexcept
    {
        const auto next_bucket = EMH_BUCKET(_index, bucket);
        const auto new_bucket  = find_empty_bucket(next_bucket, 2);
        const auto prev_bucket = find_prev_bucket(kmain, bucket);

        const auto last = next_bucket == bucket ? new_bucket : next_bucket;
        EMH_INDEX(_index, new_bucket) = {last, EMH_HSLOT(_index, bucket)};

        EMH_BUCKET(_index, prev_bucket) = new_bucket;
        EMH_BUCKET(_index, bucket) = INACTIVE;

        return bucket;
    }

/*
** inserts a new key into a hash table; first, check whether key's main
** bucket/position is free. If not, check whether colliding node/bucket is in its main
** position or not: if it is not, move colliding bucket to an empty place and
** put new key in its main position; otherwise (colliding bucket is in its main
** position), new key goes to an empty position.
*/
    template<typename K=KeyT>
    size_type find_or_allocate(const K& key, uint64_t key_hash) noexcept
    {
        const auto bucket = size_type(key_hash & _mask);
        auto next_bucket = EMH_BUCKET(_index, bucket);
        if ((int)next_bucket < 0) {
#if EMH_HIGH_LOAD
            if (next_bucket != INACTIVE)
                pop_empty(bucket);
#endif
            return bucket;
        }

        const auto slot = EMH_SLOT(_index, bucket);
        if (EMH_EQHASH(bucket, key_hash))
            if (EMH_LIKELY(_eq(key, EMH_KEY(_pairs, slot))))
            return bucket;

        //check current bucket_key is in main bucket or not
        const auto kmain = hash_bucket(EMH_KEY(_pairs, slot));
        if (kmain != bucket)
            return kickout_bucket(kmain, bucket);
        else if (next_bucket == bucket)
            return EMH_BUCKET(_index, next_bucket) = find_empty_bucket(next_bucket, 1);

        uint32_t csize = 1;
        //find next linked bucket and check key
        while (true) {
            const auto eslot = EMH_SLOT(_index, next_bucket);
            if (EMH_EQHASH(next_bucket, key_hash)) {
                if (EMH_LIKELY(_eq(key, EMH_KEY(_pairs, eslot))))
                return next_bucket;
            }

            csize += 1;
            const auto nbucket = EMH_BUCKET(_index, next_bucket);
            if (nbucket == next_bucket)
                break;
            next_bucket = nbucket;
        }

        //find a empty and link it to tail
        const auto new_bucket = find_empty_bucket(next_bucket, csize);
        return EMH_BUCKET(_index, next_bucket) = new_bucket;
    }

    size_type find_unique_bucket(uint64_t key_hash) noexcept
    {
        const auto bucket = size_type(key_hash & _mask);
        auto next_bucket = EMH_BUCKET(_index, bucket);
        if ((int)next_bucket < 0) {
#if EMH_HIGH_LOAD
            if (next_bucket != INACTIVE)
                pop_empty(bucket);
#endif
            return bucket;
        }

        //check current bucket_key is in main bucket or not
        const auto kmain = hash_main(bucket);
        if (EMH_UNLIKELY(kmain != bucket))
            return kickout_bucket(kmain, bucket);
        else if (EMH_UNLIKELY(next_bucket != bucket))
            next_bucket = find_last_bucket(next_bucket);

        return EMH_BUCKET(_index, next_bucket) = find_empty_bucket(next_bucket, 2);
    }

/***
  Different probing techniques usually provide a trade-off between memory locality and avoidance of clustering.
Since Robin Hood hashing is relatively resilient to clustering (both primary and secondary), linear probing is the most cache friendly alternativeis typically used.

    It's the core algorithm of this hash map with highly optimization/benchmark.
normaly linear probing is inefficient with high load factor, it use a new 3-way linear
probing strategy to search empty slot. from benchmark even the load factor > 0.9, it's more 2-3 timer fast than
one-way search strategy.

1. linear or quadratic probing a few cache line for less cache miss from input slot "bucket_from".
2. the first  search  slot from member variant "_last", init with 0
3. the second search slot from calculated pos "(_num_filled + _last) & _mask", it's like a rand value
*/
    // key is not in this mavalue. Find a place to put it.
    size_type find_empty_bucket(const size_type bucket_from, uint32_t csize) noexcept
    {
#if EMH_HIGH_LOAD
        if (_ehead)
            return pop_empty(_ehead);
#endif

        auto bucket = bucket_from;
        if (EMH_EMPTY(_index, ++bucket) || EMH_EMPTY(_index, ++bucket))
            return bucket;

#ifdef EMH_QUADRATIC
        constexpr size_type linear_probe_length = 2 * EMH_CACHE_LINE_SIZE / sizeof(Index);//16
        for (size_type offset = csize + 2, step = 4; offset <= linear_probe_length; ) {
            bucket = (bucket_from + offset) & _mask;
            if (EMH_EMPTY(_index, bucket) || EMH_EMPTY(_index, ++bucket))
                return bucket;
            offset += step; //7/8. 12. 16
        }
#else
        constexpr size_type quadratic_probe_length = 6u;
        for (size_type offset = 4u, step = 3u; step < quadratic_probe_length; ) {
            bucket = (bucket_from + offset) & _mask;
            if (EMH_EMPTY(_index, bucket) || EMH_EMPTY(_index, ++bucket))
                return bucket;
            offset += step++;//3.4.5
        }
#endif

#if EMH_PREFETCH
        __builtin_prefetch(static_cast<const void*>(_index + _last + 1), 0, EMH_PREFETCH);
#endif

        for (;;) {
#if EMH_PACK_TAIL
            //find empty bucket and skip next
            if (EMH_EMPTY(_index, _last++))// || EMH_EMPTY(_index, _last++))
                return _last++ - 1;

            if (EMH_UNLIKELY(_last >= _num_buckets))
                _last = 0;

            auto medium = (_mask / 4 + _last++) & _mask;
            if (EMH_EMPTY(_index, medium))
                return medium;
#else
            if (EMH_EMPTY(_index, ++_last))// || EMH_EMPTY(_index, ++_last))
                return _last++;

            _last &= _mask;
            auto medium = (_num_buckets / 2 + _last) & _mask;
            if (EMH_EMPTY(_index, medium))// || EMH_EMPTY(_index, ++medium))
                return _last = medium;
#endif
        }

        return 0;
    }

    size_type find_last_bucket(size_type main_bucket) const
    {
        auto next_bucket = EMH_BUCKET(_index, main_bucket);
        if (next_bucket == main_bucket)
            return main_bucket;

        while (true) {
            const auto nbucket = EMH_BUCKET(_index, next_bucket);
            if (nbucket == next_bucket)
                return next_bucket;
            next_bucket = nbucket;
        }
    }

    size_type find_prev_bucket(const size_type main_bucket, const size_type bucket) const
    {
        auto next_bucket = EMH_BUCKET(_index, main_bucket);
        if (next_bucket == bucket)
            return main_bucket;

        while (true) {
            const auto nbucket = EMH_BUCKET(_index, next_bucket);
            if (nbucket == bucket)
                return next_bucket;
            next_bucket = nbucket;
        }
    }

    inline size_type hash_bucket(const KeyT& key) const noexcept
    {
        return (size_type)hash_key(key) & _mask;
    }

    inline size_type hash_main(const size_type bucket) const noexcept
    {
        const auto slot = EMH_SLOT(_index, bucket);
        return (size_type)hash_key(EMH_KEY(_pairs, slot)) & _mask;
    }

#if EMH_INT_HASH
    static constexpr uint64_t KC = UINT64_C(11400714819323198485);
    static uint64_t hash64(uint64_t key)
    {
#if __SIZEOF_INT128__ && EMH_INT_HASH == 1
        __uint128_t r = key; r *= KC;
        return (uint64_t)(r >> 64) + (uint64_t)r;
#elif EMH_INT_HASH == 2
        //MurmurHash3Mixer
        uint64_t h = key;
        h ^= h >> 33;
        h *= 0xff51afd7ed558ccd;
        h ^= h >> 33;
        h *= 0xc4ceb9fe1a85ec53;
        h ^= h >> 33;
        return h;
#elif _WIN64 && EMH_INT_HASH == 1
        uint64_t high;
        return _umul128(key, KC, &high) + high;
#elif EMH_INT_HASH == 3
        auto ror  = (key >> 32) | (key << 32);
        auto low  = key * 0xA24BAED4963EE407ull;
        auto high = ror * 0x9FB21C651E98DF25ull;
        auto mix  = low + high;
        return mix;
#elif EMH_INT_HASH == 1
        uint64_t r = key * UINT64_C(0xca4bcaa75ec3f625);
        return (r >> 32) + r;
#elif EMH_WYHASH64
        return wyhash64(key, KC);
#else
        uint64_t x = key;
        x = (x ^ (x >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
        x = (x ^ (x >> 27)) * UINT64_C(0x94d049bb133111eb);
        x = x ^ (x >> 31);
        return x;
#endif
    }
#endif

#if EMH_WYHASH_HASH
    //#define WYHASH_CONDOM 1
    inline uint64_t wymix(uint64_t A, uint64_t B)
    {
#if defined(__SIZEOF_INT128__)
        __uint128_t r = A; r *= B;
#if WYHASH_CONDOM2
        A ^= (uint64_t)r; B ^= (uint64_t)(r >> 64);
#else
        A = (uint64_t)r; B = (uint64_t)(r >> 64);
#endif

#elif defined(_MSC_VER) && defined(_M_X64)
#if WYHASH_CONDOM2
        uint64_t a, b;
        a = _umul128(A, B, &b);
        A ^= a; B ^= b;
#else
        A = _umul128(A, B, &B);
#endif
#else
        uint64_t ha = A >> 32, hb = B >> 32, la = (uint32_t)A, lb = (uint32_t)B, hi, lo;
        uint64_t rh = ha * hb, rm0 = ha * lb, rm1 = hb * la, rl = la * lb, t = rl + (rm0 << 32), c = t < rl;
        lo = t + (rm1 << 32); c += lo < t; hi = rh + (rm0 >> 32) + (rm1 >> 32) + c;
#if WYHASH_CONDOM2
        A ^= lo; B ^= hi;
#else
        A = lo; B = hi;
#endif
#endif
        return A ^ B;
    }

    //multiply and xor mix function, aka MUM
    static inline uint64_t wyr8(const uint8_t *p) { uint64_t v; memcpy(&v, p, 8); return v; }
    static inline uint64_t wyr4(const uint8_t *p) { uint32_t v; memcpy(&v, p, 4); return v; }
    static inline uint64_t wyr3(const uint8_t *p, size_t k) {
        return (((uint64_t)p[0]) << 16) | (((uint64_t)p[k >> 1]) << 8) | p[k - 1];
    }

    static constexpr uint64_t secret[4] = {
        0xa0761d6478bd642full, 0xe7037ed1a0b428dbull,
        0x8ebc6af09c88c6e3ull, 0x589965cc75374cc3ull};

public:
    //wyhash main function https://github.com/wangyi-fudan/wyhash
    static uint64_t wyhashstr(const char *key, const size_t len)
    {
        uint64_t a = 0, b = 0, seed = secret[0];
        const uint8_t *p = (const uint8_t*)key;
        if (EMH_LIKELY(len <= 16)) {
            if (EMH_LIKELY(len >= 4)) {
                const auto half = (len >> 3) << 2;
                a = (wyr4(p) << 32U) | wyr4(p + half); p += len - 4;
                b = (wyr4(p) << 32U) | wyr4(p - half);
            } else if (len) {
                a = wyr3(p, len);
            }
        } else {
            size_t i = len;
            if (EMH_UNLIKELY(i > 48)) {
                uint64_t see1 = seed, see2 = seed;
                do {
                    seed = wymix(wyr8(p +  0) ^ secret[1], wyr8(p +  8) ^ seed);
                    see1 = wymix(wyr8(p + 16) ^ secret[2], wyr8(p + 24) ^ see1);
                    see2 = wymix(wyr8(p + 32) ^ secret[3], wyr8(p + 40) ^ see2);
                    p += 48; i -= 48;
                } while (EMH_LIKELY(i > 48));
                seed ^= see1 ^ see2;
            }
            while (i > 16) {
                seed = wymix(wyr8(p) ^ secret[1], wyr8(p + 8) ^ seed);
                i -= 16; p += 16;
            }
            a = wyr8(p + i - 16);
            b = wyr8(p + i - 8);
        }

        return wymix(secret[1] ^ len, wymix(a ^ secret[1], b ^ seed));
    }
#endif

private:
    template<typename UType, typename std::enable_if<std::is_integral<UType>::value, uint32_t>::type = 0>
    inline uint64_t hash_key(const UType key) const
    {
#if EMH_INT_HASH
        return hash64(key);
#elif EMH_IDENTITY_HASH
        return key + (key >> 24);
#else
        return _hasher(key);
#endif
    }

    template<typename UType, typename std::enable_if<std::is_same<UType, std::string>::value, uint32_t>::type = 0>
    inline uint64_t hash_key(const UType& key) const
    {
#if EMH_WYHASH_HASH
        return wyhashstr(key.data(), key.size());
#else
        return _hasher(key);
#endif
    }

    template<typename UType, typename std::enable_if<!std::is_integral<UType>::value && !std::is_same<UType, std::string>::value, uint32_t>::type = 0>
    inline uint64_t hash_key(const UType& key) const
    {
        return _hasher(key);
    }

private:
    Index*    _index;
    value_type*_pairs;

    HashT     _hasher;
    EqT       _eq;
    uint32_t  _mlf;
    size_type _mask;
    size_type _num_buckets;
    size_type _num_filled;
    size_type _last;
#if EMH_HIGH_LOAD
    size_type _ehead;
#endif
    size_type _etail;
};
} // namespace emhash




struct PyObject;
typedef pkpy::shared_ptr<PyObject> PyVar;
typedef PyVar PyVarOrNull;
typedef PyVar PyVarRef;

class PyVarList: public std::vector<PyVar> {
    PyVar& at(size_t) = delete;

    inline void __checkIndex(size_t i) const {
#ifndef PKPY_NO_INDEX_CHECK
        if (i >= size()){
            auto msg = "std::vector index out of range, " + std::to_string(i) + " not in [0, " + std::to_string(size()) + ")";
            throw std::out_of_range(msg);
        }
#endif
    }
public:
    PyVar& operator[](size_t i) {
        __checkIndex(i);
        return std::vector<PyVar>::operator[](i);
    }

    const PyVar& operator[](size_t i) const {
        __checkIndex(i);
        return std::vector<PyVar>::operator[](i);
    }

    // define constructors the same as std::vector
    using std::vector<PyVar>::vector;
};


class PyVarDict: public emhash8::HashMap<_Str, PyVar> {
    PyVar& at(const _Str&) = delete;

public:

#ifndef PKPY_NO_INDEX_CHECK
    PyVar& operator[](const _Str& key) {
        return emhash8::HashMap<_Str, PyVar>::operator[](key);
    }

    const PyVar& operator[](const _Str& key) const {
        auto it = find(key);
        if (it == end()){
            auto msg = "map key not found, '" + key.str() + "'";
            throw std::out_of_range(msg);
        }
        return it->second;
    }
#endif

    using emhash8::HashMap<_Str, PyVar>::HashMap;
};


namespace pkpy {
    const uint8_t MAX_POOLING_N = 10;
    static std::vector<PyVar*>* _poolArgList = new std::vector<PyVar*>[MAX_POOLING_N];

    class ArgList {
        PyVar* _args = nullptr;
        uint8_t _size = 0;

        inline void __checkIndex(uint8_t i) const {
#ifndef PKPY_NO_INDEX_CHECK
            if (i >= _size){
                auto msg = "pkpy:ArgList index out of range, " + std::to_string(i) + " not in [0, " + std::to_string(size()) + ")";
                throw std::out_of_range(msg);
            }
#endif
        }

        void __tryAlloc(uint8_t n){
            if(n > 255) UNREACHABLE();
            if(n >= MAX_POOLING_N || _poolArgList[n].empty()){
                this->_size = n;
                this->_args = new PyVar[n];
            }else{
                this->_args = _poolArgList[n].back();
                this->_size = n;
                _poolArgList[n].pop_back();
            }
        }

        void __tryRelease(){
            if(_size == 0 || _args == nullptr) return;
            if(_size >= MAX_POOLING_N || _poolArgList[_size].size() > 32){
                delete[] _args;
            }else{
                for(uint8_t i = 0; i < _size; i++) _args[i].reset();
                _poolArgList[_size].push_back(_args);
            }
        }

    public:
        ArgList(uint8_t n){
            if(n != 0) __tryAlloc(n);
        }

        ArgList(const ArgList& other){
            __tryAlloc(other._size);
            for(uint8_t i=0; i<_size; i++){
                _args[i] = other._args[i];
            }
        }

        ArgList(ArgList&& other) noexcept {
            this->_args = other._args;
            this->_size = other._size;
            other._args = nullptr;
            other._size = 0;
        }

        ArgList(PyVarList&& other) noexcept {
            __tryAlloc(other.size());
            for(uint8_t i=0; i<_size; i++){
                _args[i] = std::move(other[i]);
            }
            other.clear();
        }

        PyVar& operator[](uint8_t i){
            __checkIndex(i);
            return _args[i];
        }

        const PyVar& operator[](uint8_t i) const {
            __checkIndex(i);
            return _args[i];
        }

        inline PyVar& _index(uint8_t i){
            return _args[i];
        }

        inline const PyVar& _index(uint8_t i) const {
            return _args[i];
        }

        // overload = for &&
        ArgList& operator=(ArgList&& other) noexcept {
            if(this != &other){
                __tryRelease();
                this->_args = other._args;
                this->_size = other._size;
                other._args = nullptr;
                other._size = 0;
            }
            return *this;
        }

        inline uint8_t size() const {
            return _size;
        }

        ArgList subList(uint8_t start) const {
            if(start >= _size) return ArgList(0);
            ArgList ret(_size - start);
            for(uint8_t i=start; i<_size; i++){
                ret[i-start] = _args[i];
            }
            return ret;
        }

        PyVarList toList() const {
            PyVarList ret(_size);
            for(uint8_t i=0; i<_size; i++){
                ret[i] = _args[i];
            }
            return ret;
        }

        ~ArgList(){
            __tryRelease();
        }
    };

    const ArgList& noArg(){
        static ArgList ret(0);
        return ret;
    }

    ArgList oneArg(PyVar&& a) {
        ArgList ret(1);
        ret[0] = std::move(a);
        return ret;
    }

    ArgList oneArg(const PyVar& a) {
        ArgList ret(1);
        ret[0] = a;
        return ret;
    }

    ArgList twoArgs(PyVar&& a, PyVar&& b) {
        ArgList ret(2);
        ret[0] = std::move(a);
        ret[1] = std::move(b);
        return ret;
    }

    ArgList twoArgs(const PyVar& a, const PyVar& b) {
        ArgList ret(2);
        ret[0] = a;
        ret[1] = b;
        return ret;
    }
}


const char* __BUILTINS_CODE = R"(
def len(x):
    return x.__len__()

def print(*args, sep=' ', end='\n'):
    s = sep.join([str(i) for i in args])
    __sys_stdout_write(s + end)


str.__mul__ = lambda self, n: ''.join([self for _ in range(n)])

def __str4split(self, sep):
    if sep == "":
        return list(self)
    res = []
    i = 0
    while i < len(self):
        if self[i:i+len(sep)] == sep:
            res.append(self[:i])
            self = self[i+len(sep):]
            i = 0
        else:
            i += 1
    res.append(self)
    return res
str.split = __str4split
del __str4split

def __str4index(self, sub):
    for i in range(len(self) - len(sub) + 1):
        if self[i:i+len(sub)] == sub:
            return i
    return -1
str.index = __str4index
del __str4index

list.__repr__ = lambda self: '[' + ', '.join([repr(i) for i in self]) + ']'
tuple.__repr__ = lambda self: '(' + ', '.join([repr(i) for i in self]) + ')'
list.__json__ = lambda self: '[' + ', '.join([i.__json__() for i in self]) + ']'
tuple.__json__ = lambda self: '[' + ', '.join([i.__json__() for i in self]) + ']'

def __list4extend(self, other):
    for i in other:
        self.append(i)
list.extend = __list4extend
del __list4extend

def __list4remove(self, value):
    for i in range(len(self)):
        if self[i] == value:
            del self[i]
            return True
    return False
list.remove = __list4remove
del __list4remove

def __list4index(self, value):
    for i in range(len(self)):
        if self[i] == value:
            return i
    return -1
list.index = __list4index
del __list4index

def __list4pop(self, i=-1):
    res = self[i]
    del self[i]
    return res
list.pop = __list4pop
del __list4pop

def __list4__mul__(self, n):
    a = []
    for i in range(n):
        a.extend(self)
    return a
list.__mul__ = __list4__mul__
del __list4__mul__

def __iterable4__eq__(self, other):
    if len(self) != len(other):
        return False
    for i in range(len(self)):
        if self[i] != other[i]:
            return False
    return True
list.__eq__ = __iterable4__eq__
tuple.__eq__ = __iterable4__eq__
del __iterable4__eq__

def __iterable4count(self, x):
    res = 0
    for i in self:
        if i == x:
            res += 1
    return res
list.count = __iterable4count
tuple.count = __iterable4count
del __iterable4count

def __iterable4__contains__(self, item):
    for i in self:
        if i == item:
            return True
    return False
list.__contains__ = __iterable4__contains__
tuple.__contains__ = __iterable4__contains__
del __iterable4__contains__

list.__new__ = lambda obj: [i for i in obj]

# https://github.com/python/cpython/blob/main/Objects/dictobject.c
class dict:
    def __init__(self, capacity=16):
        self._capacity = capacity
        self._a = [None] * self._capacity
        self._len = 0
        
    def __len__(self):
        return self._len

    def __probe(self, key):
        i = hash(key) % self._capacity
        while self._a[i] is not None:
            if self._a[i][0] == key:
                return True, i
            i = (i + 1) % self._capacity
        return False, i

    def __getitem__(self, key):
        ok, i = self.__probe(key)
        if not ok:
            raise KeyError(key)
        return self._a[i][1]

    def __contains__(self, key):
        ok, i = self.__probe(key)
        return ok

    def __setitem__(self, key, value):
        ok, i = self.__probe(key)
        if ok:
            self._a[i][1] = value
        else:
            self._a[i] = [key, value]
            self._len += 1
            if self._len > self._capacity * 0.8:
                self._capacity *= 2
                self.__rehash()

    def __delitem__(self, key):
        ok, i = self.__probe(key)
        if not ok:
            raise KeyError(key)
        self._a[i] = None
        self._len -= 1

    def __rehash(self):
        old_a = self._a
        self._a = [None] * self._capacity
        self._len = 0
        for kv in old_a:
            if kv is not None:
                self[kv[0]] = kv[1]

    def keys(self):
        return [kv[0] for kv in self._a if kv is not None]

    def values(self):
        return [kv[1] for kv in self._a if kv is not None]

    def items(self):
        return [kv for kv in self._a if kv is not None]

    def clear(self):
        self._a = [None] * self._capacity
        self._len = 0

    def update(self, other):
        for k, v in other.items():
            self[k] = v

    def copy(self):
        d = dict()
        for kv in self._a:
            if kv is not None:
                d[kv[0]] = kv[1]
        return d

    def __repr__(self):
        a = [repr(k)+': '+repr(v) for k,v in self.items()]
        return '{'+ ', '.join(a) + '}'

    def __json__(self):
        a = []
        for k,v in self.items():
            if type(k) is not str:
                raise TypeError('json keys must be strings, got ' + repr(k) )
            a.append(k.__json__()+': '+v.__json__())
        return '{'+ ', '.join(a) + '}'

def round(x):
    if x >= 0:
        return int(x + 0.5)
    else:
        return int(x - 0.5)

def max(a, b):
    if a > b:
        return a
    return b

def min(a, b):
    if a < b:
        return a
    return b

def sum(iterable):
    res = 0
    for i in iterable:
        res += i
    return res

def map(f, iterable):
    return [f(i) for i in iterable]

def zip(a, b):
    return [(a[i], b[i]) for i in range(min(len(a), len(b)))]

def reversed(iterable):
    a = list(iterable)
    return [a[i] for i in range(len(a)-1, -1, -1)]

def sorted(iterable, key=None, reverse=False):
    if key is None:
        key = lambda x: x
    a = [key(i) for i in iterable]
    b = list(iterable)
    for i in range(len(a)):
        for j in range(i+1, len(a)):
            if (a[i] > a[j]) ^ reverse:
                a[i], a[j] = a[j], a[i]
                b[i], b[j] = b[j], b[i]
    return b

import json as _json

def jsonrpc(method, params, raw=False):
  assert type(method) is str
  assert type(params) is list or type(params) is tuple
  data = {
    'jsonrpc': '2.0',
    'method': method,
    'params': params,
  }
  ret = __string_channel_call(_json.dumps(data))
  ret = _json.loads(ret)
  if raw:
    return ret
  assert type(ret) is dict
  if 'result' in ret:
    return ret['result']
  raise JsonRpcError(ret['error']['message'])

def input(prompt=None):
  return jsonrpc('input', [prompt])
  
class FileIO:
  def __init__(self, path, mode):
    assert type(path) is str
    assert type(mode) is str
    assert mode in ['r', 'w']
    self.path = path
    self.mode = mode
    self.fp = jsonrpc('fopen', [path, mode])

  def read(self):
    assert self.mode == 'r'
    return jsonrpc('fread', [self.fp])

  def write(self, s):
    assert self.mode == 'w'
    assert type(s) is str
    jsonrpc('fwrite', [self.fp, s])

  def close(self):
    jsonrpc('fclose', [self.fp])

  def __enter__(self):
    pass

  def __exit__(self):
    self.close()

def open(path, mode='r'):
    return FileIO(path, mode)


class set:
    def __init__(self, iterable=None):
        iterable = iterable or []
        self._a = dict()
        for item in iterable:
            self.add(item)

    def add(self, elem):
        self._a[elem] = None
        
    def discard(self, elem):
        if elem in self._a:
            del self._a[elem]

    def remove(self, elem):
        del self._a[elem]
        
    def clear(self):
        self._a.clear()

    def update(self,other):
        for elem in other:
            self.add(elem)
        return self

    def __len__(self):
        return len(self._a)
    
    def copy(self):
        return set(self._a.keys())
    
    def __and__(self, other):
        ret = set()
        for elem in self:
            if elem in other:
                ret.add(elem)
        return ret
    
    def __or__(self, other):
        ret = self.copy()
        for elem in other:
            ret.add(elem)
        return ret

    def __sub__(self, other):
        ret = set() 
        for elem in self:
            if elem not in other: 
                ret.add(elem) 
        return ret
    
    def __xor__(self, other): 
        ret = set() 
        for elem in self: 
            if elem not in other: 
                ret.add(elem) 
        for elem in other: 
            if elem not in self: 
                ret.add(elem) 
        return ret

    def union(self, other):
        return self | other

    def intersection(self, other):
        return self & other

    def difference(self, other):
        return self - other

    def symmetric_difference(self, other):      
        return self ^ other
    
    def __eq__(self, other):
        return self.__xor__(other).__len__() == 0
    
    def isdisjoint(self, other):
        return self.__and__(other).__len__() == 0
    
    def issubset(self, other):
        return self.__sub__(other).__len__() == 0
    
    def issuperset(self, other):
        return other.__sub__(self).__len__() == 0

    def __contains__(self, elem):
        return elem in self._a
    
    def __repr__(self):
        if len(self) == 0:
            return 'set()'
        return '{'+ ', '.join(self._a.keys()) + '}'
    
    def __iter__(self):
        return self._a.keys().__iter__()
)";

const char* __OS_CODE = R"(
def listdir(path):
  assert type(path) is str
  return jsonrpc("os.listdir", [path])

def mkdir(path):
  assert type(path) is str
  return jsonrpc("os.mkdir", [path])

def rmdir(path):
  assert type(path) is str
  return jsonrpc("os.rmdir", [path])

def remove(path):
  assert type(path) is str
  return jsonrpc("os.remove", [path])

path = object()

def __path4exists(path):
  assert type(path) is str
  return jsonrpc("os.path.exists", [path])
path.exists = __path4exists
del __path4exists
)";

const char* __RANDOM_CODE = R"(
import time as _time

__all__ = ['Random', 'seed', 'random', 'randint', 'uniform']

def _int32(x):
	return int(0xffffffff & x)

class Random:
	def __init__(self, seed=None):
		if seed is None:
			seed = int(_time.time() * 1000000)
		seed = _int32(seed)
		self.mt = [0] * 624
		self.mt[0] = seed
		self.mti = 0
		for i in range(1, 624):
			self.mt[i] = _int32(1812433253 * (self.mt[i - 1] ^ self.mt[i - 1] >> 30) + i)
	
	def extract_number(self):
		if self.mti == 0:
			self.twist()
		y = self.mt[self.mti]
		y = y ^ y >> 11
		y = y ^ y << 7 & 2636928640
		y = y ^ y << 15 & 4022730752
		y = y ^ y >> 18
		self.mti = (self.mti + 1) % 624
		return _int32(y)
	
	def twist(self):
		for i in range(0, 624):
			y = _int32((self.mt[i] & 0x80000000) + (self.mt[(i + 1) % 624] & 0x7fffffff))
			self.mt[i] = (y >> 1) ^ self.mt[(i + 397) % 624]
			
			if y % 2 != 0:
				self.mt[i] = self.mt[i] ^ 0x9908b0df
				
	def seed(self, x):
		assert type(x) is int
		self.mt = [0] * 624
		self.mt[0] = _int32(x)
		self.mti = 0
		for i in range(1, 624):
			self.mt[i] = _int32(1812433253 * (self.mt[i - 1] ^ self.mt[i - 1] >> 30) + i)
			
	def random(self):
		return self.extract_number() / 2 ** 32
		
	def randint(self, a, b):
		assert type(a) is int and type(b) is int
		assert a <= b
		return int(self.random() * (b - a + 1)) + a
		
	def uniform(self, a, b):
        assert type(a) is int or type(a) is float
        assert type(b) is int or type(b) is float
		if a > b:
			a, b = b, a
		return self.random() * (b - a) + a

    def shuffle(self, L):
        for i in range(len(L)):
            j = self.randint(i, len(L) - 1)
            L[i], L[j] = L[j], L[i]

    def choice(self, L):
        return L[self.randint(0, len(L) - 1)]
		
_inst = Random()
seed = _inst.seed
random = _inst.random
randint = _inst.randint
uniform = _inst.uniform
shuffle = _inst.shuffle
choice = _inst.choice

)";


class NeedMoreLines {
public:
    NeedMoreLines(bool isClassDef) : isClassDef(isClassDef) {}
    bool isClassDef;
};

enum CompileMode {
    EXEC_MODE,
    EVAL_MODE,
    SINGLE_MODE,     // for REPL
    JSON_MODE,
};

struct SourceMetadata {
    const char* source;
    _Str filename;
    std::vector<const char*> lineStarts;
    CompileMode mode;

    std::pair<const char*,const char*> getLine(int lineno) const {
        if(lineno == -1) return {nullptr, nullptr};
        lineno -= 1;
        if(lineno < 0) lineno = 0;
        const char* _start = lineStarts.at(lineno);
        const char* i = _start;
        while(*i != '\n' && *i != '\0') i++;
        return {_start, i};
    }

    SourceMetadata(const char* source, _Str filename, CompileMode mode) {
        source = strdup(source);
        // Skip utf8 BOM if there is any.
        if (strncmp(source, "\xEF\xBB\xBF", 3) == 0) source += 3;
        this->filename = filename;
        this->source = source;
        lineStarts.push_back(source);
        this->mode = mode;
    }

    _Str snapshot(int lineno, const char* cursor=nullptr){
        _StrStream ss;
        ss << "  " << "File \"" << filename << "\", line " << lineno << '\n';
        std::pair<const char*,const char*> pair = getLine(lineno);
        _Str line = "<?>";
        int removedSpaces = 0;
        if(pair.first && pair.second){
            line = _Str(pair.first, pair.second-pair.first).__lstrip();
            removedSpaces = pair.second - pair.first - line.size();
            if(line.empty()) line = "<?>";
        }
        ss << "    " << line << '\n';
        if(cursor && line != "<?>" && cursor >= pair.first && cursor <= pair.second){
            int column = cursor - pair.first - removedSpaces;
            if(column >= 0){
                ss << "    " << std::string(column, ' ') << "^\n";
            }
        }
        return ss.str();
    }

    ~SourceMetadata(){
        free((void*)source);
    }
};

typedef pkpy::shared_ptr<SourceMetadata> _Source;

class _Error : public std::exception {
private:
    _Str _what;
public:
    _Error(_Str type, _Str msg, _Str desc){
        _what = desc + type + ": " + msg;
    }

    const char* what() const noexcept override {
        return _what.c_str();
    }
};

class CompileError : public _Error {
public:
    CompileError(_Str type, _Str msg, _Str snapshot)
        : _Error(type, msg, snapshot) {}
};

class RuntimeError : public _Error {
private:
    static _Str __concat(std::stack<_Str> snapshots){
        _StrStream ss;
        ss << "Traceback (most recent call last):" << '\n';
        while(!snapshots.empty()){
            ss << snapshots.top();
            snapshots.pop();
        }
        return ss.str();
    }
public:
    RuntimeError(_Str type, _Str msg, const std::stack<_Str>& snapshots)
        : _Error(type, msg, __concat(snapshots)) {}
};


typedef int64_t _Int;
typedef double _Float;

struct CodeObject;
struct BaseRef;
class VM;
class Frame;

typedef PyVar (*_CppFunc)(VM*, const pkpy::ArgList&);
typedef pkpy::shared_ptr<CodeObject> _Code;

struct Function {
    _Str name;
    _Code code;
    std::vector<_Str> args;
    _Str starredArg;        // empty if no *arg
    PyVarDict kwArgs;       // empty if no k=v
    std::vector<_Str> kwArgsOrder;

    bool hasName(const _Str& val) const {
        bool _0 = std::find(args.begin(), args.end(), val) != args.end();
        bool _1 = starredArg == val;
        bool _2 = kwArgs.find(val) != kwArgs.end();
        return _0 || _1 || _2;
    }
};

struct _BoundedMethod {
    PyVar obj;
    PyVar method;
};

struct _Range {
    _Int start = 0;
    _Int stop = -1;
    _Int step = 1;
};

struct _Slice {
    int start = 0;
    int stop = 0x7fffffff; 

    void normalize(int len){
        if(start < 0) start += len;
        if(stop < 0) stop += len;
        if(start < 0) start = 0;
        if(stop > len) stop = len;
    }
};

class BaseIterator {
protected:
    VM* vm;
    PyVar _ref;     // keep a reference to the object so it will not be deleted while iterating
public:
    virtual PyVar next() = 0;
    virtual bool hasNext() = 0;
    PyVarRef var;
    BaseIterator(VM* vm, PyVar _ref) : vm(vm), _ref(_ref) {}
    virtual ~BaseIterator() = default;
};

typedef pkpy::shared_ptr<Function> _Func;
typedef pkpy::shared_ptr<BaseIterator> _Iterator;

struct PyObject {
    PyVar _type;
    PyVarDict attribs;

    inline bool isType(const PyVar& type){ return this->_type == type; }
    inline virtual void* value() = 0;

    // currently __name__ is only used for 'type'
    PyVar _typeName(){ return _type->attribs[__name__]; }

    PyObject(PyVar type) : _type(type) {}
    virtual ~PyObject() = default;
};

template <typename T>
struct Py_ : PyObject {
    T _valueT;

    Py_(T val, const PyVar& type) : PyObject(type), _valueT(val) {}
    virtual void* value() override { return &_valueT; }
};

#define UNION_GET(T, obj) (((Py_<T>*)((obj).get()))->_valueT)
#define UNION_TP_NAME(obj) UNION_GET(_Str, (obj)->_typeName())
#define UNION_NAME(obj) UNION_GET(_Str, (obj)->attribs[__name__])


class RangeIterator : public BaseIterator {
private:
    _Int current;
    _Range r;
public:
    RangeIterator(VM* vm, PyVar _ref) : BaseIterator(vm, _ref) {
        this->r = UNION_GET(_Range, _ref);
        this->current = r.start;
    }

    bool hasNext() override {
        if(r.step > 0){
            return current < r.stop;
        }else{
            return current > r.stop;
        }
    }

    PyVar next() override;
};

class VectorIterator : public BaseIterator {
private:
    size_t index = 0;
    const PyVarList* vec;
public:
    VectorIterator(VM* vm, PyVar _ref) : BaseIterator(vm, _ref) {
        vec = &UNION_GET(PyVarList, _ref);
    }

    bool hasNext(){
        return index < vec->size();
    }

    PyVar next(){
        return vec->operator[](index++);
    }
};

class StringIterator : public BaseIterator {
private:
    int index = 0;
    _Str str;
public:
    StringIterator(VM* vm, PyVar _ref) : BaseIterator(vm, _ref) {
        str = UNION_GET(_Str, _ref);
    }

    bool hasNext(){
        return index < str.u8_length();
    }

    PyVar next();
};



typedef uint8_t _TokenType;

constexpr const char* __TOKENS[] = {
    "@error", "@eof", "@eol", "@sof",
    ".", ",", ":", ";", "#", "(", ")", "[", "]", "{", "}", "%",
    "+", "-", "*", "/", "//", "**", "=", ">", "<", "...", "->",
    "<<", ">>", "&", "|", "^", "?",
    "==", "!=", ">=", "<=",
    "+=", "-=", "*=", "/=", "//=", "%=", "&=", "|=", "^=",
    /** KW_BEGIN **/
    "class", "import", "as", "def", "lambda", "pass", "del", "from", "with",
    "None", "in", "is", "and", "or", "not", "True", "False", "global",
    "goto", "label",      // extended keywords, not available in cpython
    "while", "for", "if", "elif", "else", "break", "continue", "return", "assert", "raise",
    /** KW_END **/
    "is not", "not in",
    "@id", "@num", "@str", "@fstr",
    "@indent", "@dedent"
};

const _TokenType __TOKENS_LEN = sizeof(__TOKENS) / sizeof(__TOKENS[0]);

constexpr _TokenType TK(const char* const token) {
    for(int k=0; k<__TOKENS_LEN; k++){
        const char* i = __TOKENS[k];
        const char* j = token;
        while(*i && *j && *i == *j){
            i++; j++;
        }
        if(*i == *j) return k;
    }
    return 0;
}

#define TK_STR(t) __TOKENS[t]

const _TokenType __KW_BEGIN = TK("class");
const _TokenType __KW_END = TK("raise");

const emhash8::HashMap<std::string_view, _TokenType> __KW_MAP = [](){
    emhash8::HashMap<std::string_view, _TokenType> map;
    for(int k=__KW_BEGIN; k<=__KW_END; k++) map[__TOKENS[k]] = k;
    return map;
}();


struct Token{
  _TokenType type;

  const char* start; //< Begining of the token in the source.
  int length;        //< Number of chars of the token.
  int line;          //< Line number of the token (1 based).
  PyVar value;       //< Literal value of the token.

  const _Str str() const {
    return _Str(start, length);
  }

  const _Str info() const {
    _StrStream ss;
    _Str raw = str();
    if (raw == _Str("\n")) raw = "\\n";
    ss << line << ": " << TK_STR(type) << " '" << raw << "'";
    return ss.str();
  }
};

enum Precedence {
  PREC_NONE,
  PREC_ASSIGNMENT,    // =
  PREC_COMMA,         // ,
  PREC_TERNARY,       // ?:
  PREC_LOGICAL_OR,    // or
  PREC_LOGICAL_AND,   // and
  PREC_EQUALITY,      // == !=
  PREC_TEST,          // in is
  PREC_COMPARISION,   // < > <= >=
  PREC_BITWISE_OR,    // |
  PREC_BITWISE_XOR,   // ^
  PREC_BITWISE_AND,   // &
  PREC_BITWISE_SHIFT, // << >>
  PREC_TERM,          // + -
  PREC_FACTOR,        // * / % //
  PREC_UNARY,         // - not
  PREC_EXPONENT,      // **
  PREC_CALL,          // ()
  PREC_SUBSCRIPT,     // []
  PREC_ATTRIB,        // .index
  PREC_PRIMARY,
};

// The context of the parsing phase for the compiler.
struct Parser {
    _Source src;

    const char* token_start;
    const char* current_char;
    int current_line = 1;
    Token previous, current;
    std::queue<Token> nexts;
    std::stack<int> indents;

    int brackets_level_0 = 0;
    int brackets_level_1 = 0;
    int brackets_level_2 = 0;

    Token nextToken(){
        if(nexts.empty()) return makeErrToken();
        Token t = nexts.front();
        if(t.type == TK("@eof") && indents.size()>1){
            nexts.pop();
            indents.pop();
            return Token{TK("@dedent"), token_start, 0, current_line};
        }
        nexts.pop();
        return t;
    }

    char peekChar() {
        return *current_char;
    }

    char peekNextChar() {
        if (peekChar() == '\0') return '\0';
        return *(current_char + 1);
    }

    int eatSpaces(){
        int count = 0;
        while (true) {
            switch (peekChar()) {
                case ' ' : count+=1; break;
                case '\t': count+=4; break;
                default: return count;
            }
            eatChar();
        }
    }

    bool eatIndentation(){
        if(brackets_level_0 > 0 || brackets_level_1 > 0 || brackets_level_2 > 0) return true;
        int spaces = eatSpaces();
        if(peekChar() == '#') skipLineComment();
        if(peekChar() == '\0' || peekChar() == '\n') return true;
        // https://docs.python.org/3/reference/lexical_analysis.html#indentation
        if(spaces > indents.top()){
            indents.push(spaces);
            nexts.push(Token{TK("@indent"), token_start, 0, current_line});
        } else if(spaces < indents.top()){
            while(spaces < indents.top()){
                indents.pop();
                nexts.push(Token{TK("@dedent"), token_start, 0, current_line});
            }
            if(spaces != indents.top()){
                return false;
            }
        }
        return true;
    }

    char eatChar() {
        char c = peekChar();
        if(c == '\n') throw std::runtime_error("eatChar() cannot consume a newline");
        current_char++;
        return c;
    }

    char eatCharIncludeNewLine() {
        char c = peekChar();
        current_char++;
        if (c == '\n'){
            current_line++;
            src->lineStarts.push_back(current_char);
        }
        return c;
    }

    int eatName() {
        current_char--;
        while(true){
            uint8_t c = peekChar();
            int u8bytes = 0;
            if((c & 0b10000000) == 0b00000000) u8bytes = 1;
            else if((c & 0b11100000) == 0b11000000) u8bytes = 2;
            else if((c & 0b11110000) == 0b11100000) u8bytes = 3;
            else if((c & 0b11111000) == 0b11110000) u8bytes = 4;
            else return 1;
            if(u8bytes == 1){
                if(isalpha(c) || c=='_' || isdigit(c)) {
                    current_char++;
                    continue;
                }else{
                    break;
                }
            }
            // handle multibyte char
            std::string u8str(current_char, u8bytes);
            if(u8str.size() != u8bytes) return 2;
            uint32_t value = 0;
            for(int k=0; k < u8bytes; k++){
                uint8_t b = u8str[k];
                if(k==0){
                    if(u8bytes == 2) value = (b & 0b00011111) << 6;
                    else if(u8bytes == 3) value = (b & 0b00001111) << 12;
                    else if(u8bytes == 4) value = (b & 0b00000111) << 18;
                }else{
                    value |= (b & 0b00111111) << (6*(u8bytes-k-1));
                }
            }
            if(__isLoChar(value)) current_char += u8bytes;
            else break;
        }

        int length = (int)(current_char - token_start);
        if(length == 0) return 3;
        std::string_view name(token_start, length);

        if(src->mode == JSON_MODE){
            if(name == "true"){
                setNextToken(TK("True"));
            } else if(name == "false"){
                setNextToken(TK("False"));
            } else if(name == "null"){
                setNextToken(TK("None"));
            } else {
                return 4;
            }
            return 0;
        }

        if(__KW_MAP.count(name)){
            if(name == "not"){
                if(strncmp(current_char, " in", 3) == 0){
                    current_char += 3;
                    setNextToken(TK("not in"));
                    return 0;
                }
            }else if(name == "is"){
                if(strncmp(current_char, " not", 4) == 0){
                    current_char += 4;
                    setNextToken(TK("is not"));
                    return 0;
                }
            }
            setNextToken(__KW_MAP.at(name));
        } else {
            setNextToken(TK("@id"));
        }
        return 0;
    }

    void skipLineComment() {
        char c;
        while ((c = peekChar()) != '\0') {
            if (c == '\n') return;
            eatChar();
        }
    }
    
    // If the current char is [c] consume it and advance char by 1 and returns
    // true otherwise returns false.
    bool matchChar(char c) {
        if (peekChar() != c) return false;
        eatCharIncludeNewLine();
        return true;
    }

    // Returns an error token from the current position for reporting error.
    Token makeErrToken() {
        return Token{TK("@error"), token_start, (int)(current_char - token_start), current_line};
    }

    // Initialize the next token as the type.
    void setNextToken(_TokenType type, PyVar value=nullptr) {

        switch(type){
            case TK("("): brackets_level_0++; break;
            case TK(")"): brackets_level_0--; break;
            case TK("["): brackets_level_1++; break;
            case TK("]"): brackets_level_1--; break;
            case TK("{"): brackets_level_2++; break;
            case TK("}"): brackets_level_2--; break;
        }

        nexts.push( Token{
            type,
            token_start,
            (int)(current_char - token_start),
            current_line - ((type == TK("@eol")) ? 1 : 0),
            value
        });
    }

    void setNextTwoCharToken(char c, _TokenType one, _TokenType two) {
        if (matchChar(c)) setNextToken(two);
        else setNextToken(one);
    }

    Parser(_Source src) {
        this->src = src;
        this->token_start = src->source;
        this->current_char = src->source;
        this->nexts.push(Token{TK("@sof"), token_start, 0, current_line});
        this->indents.push(0);
    }
};


class Frame;

struct BaseRef {
    virtual PyVar get(VM*, Frame*) const = 0;
    virtual void set(VM*, Frame*, PyVar) const = 0;
    virtual void del(VM*, Frame*) const = 0;
    virtual ~BaseRef() = default;
};

enum NameScope {
    NAME_LOCAL = 0,
    NAME_GLOBAL = 1,
    NAME_ATTR = 2,
};

struct NameRef : BaseRef {
    const std::pair<_Str, NameScope>* pair;
    NameRef(const std::pair<_Str, NameScope>* pair) : pair(pair) {}

    PyVar get(VM* vm, Frame* frame) const;
    void set(VM* vm, Frame* frame, PyVar val) const;
    void del(VM* vm, Frame* frame) const;
};

struct AttrRef : BaseRef {
    mutable PyVar obj;
    const NameRef attr;
    AttrRef(PyVar obj, const NameRef attr) : obj(obj), attr(attr) {}

    PyVar get(VM* vm, Frame* frame) const;
    void set(VM* vm, Frame* frame, PyVar val) const;
    void del(VM* vm, Frame* frame) const;
};

struct IndexRef : BaseRef {
    mutable PyVar obj;
    PyVar index;
    IndexRef(PyVar obj, PyVar index) : obj(obj), index(index) {}

    PyVar get(VM* vm, Frame* frame) const;
    void set(VM* vm, Frame* frame, PyVar val) const;
    void del(VM* vm, Frame* frame) const;
};

struct TupleRef : BaseRef {
    PyVarList varRefs;
    TupleRef(const PyVarList& varRefs) : varRefs(varRefs) {}
    TupleRef(PyVarList&& varRefs) : varRefs(std::move(varRefs)) {}

    PyVar get(VM* vm, Frame* frame) const;
    void set(VM* vm, Frame* frame, PyVar val) const;
    void del(VM* vm, Frame* frame) const;
};

struct UserPointer : BaseRef {
    PyVarRef p;
    uint64_t f_id;
    UserPointer(PyVarRef p, uint64_t f_id) : p(p), f_id(f_id) {}

    PyVar get(VM* vm, Frame* frame) const;
    void set(VM* vm, Frame* frame, PyVar val) const;
    void del(VM* vm, Frame* frame) const;
};


enum Opcode {
    #define OPCODE(name) OP_##name,
    #ifdef OPCODE

OPCODE(NO_OP)
OPCODE(DELETED_OP)
OPCODE(LOAD_CONST)
OPCODE(IMPORT_NAME)
OPCODE(PRINT_EXPR)
OPCODE(POP_TOP)
OPCODE(CALL)
OPCODE(RETURN_VALUE)

OPCODE(BINARY_OP)
OPCODE(COMPARE_OP)
OPCODE(BITWISE_OP)
OPCODE(IS_OP)
OPCODE(CONTAINS_OP)

OPCODE(UNARY_NEGATIVE)
OPCODE(UNARY_NOT)

OPCODE(DUP_TOP)

OPCODE(BUILD_LIST)
OPCODE(BUILD_MAP)
OPCODE(BUILD_SET)
OPCODE(BUILD_SLICE)

OPCODE(LIST_APPEND)

OPCODE(GET_ITER)
OPCODE(FOR_ITER)

OPCODE(POP_JUMP_IF_FALSE)
OPCODE(JUMP_ABSOLUTE)
OPCODE(SAFE_JUMP_ABSOLUTE)
OPCODE(JUMP_IF_TRUE_OR_POP)
OPCODE(JUMP_IF_FALSE_OR_POP)

// non-standard python opcodes
OPCODE(LOAD_NONE)
OPCODE(LOAD_TRUE)
OPCODE(LOAD_FALSE)
OPCODE(LOAD_EVAL_FN)        // load eval() callable into stack
OPCODE(LOAD_LAMBDA)         // LOAD_CONST + set __module__ attr
OPCODE(LOAD_ELLIPSIS)

OPCODE(ASSERT)
OPCODE(RAISE_ERROR)

OPCODE(STORE_FUNCTION)
OPCODE(BUILD_CLASS)

OPCODE(LOAD_NAME_REF)       // no arg
OPCODE(BUILD_ATTR_REF)      // arg for the name_ptr, [ptr, name_ptr] -> (*ptr).name_ptr
OPCODE(BUILD_INDEX_REF)     // no arg, [ptr, expr] -> (*ptr)[expr]
OPCODE(STORE_NAME_REF)      // arg for the name_ptr, [expr], directly store to the name_ptr without pushing it to the stack
OPCODE(STORE_REF)           // no arg, [ptr, expr] -> *ptr = expr
OPCODE(DELETE_REF)          // no arg, [ptr] -> [] -> delete ptr
OPCODE(BUILD_ATTR_REF_PTR)  // arg for the name_ptr, [ptr, name_ptr] -> (*ptr)->name_ptr

OPCODE(BUILD_SMART_TUPLE)   // if all elements are pointers, build a compound pointer, otherwise build a tuple
OPCODE(BUILD_STRING)        // arg is the expr count, build a string from the top of the stack

OPCODE(GOTO)
OPCODE(UNARY_REF)           // for &
OPCODE(UNARY_DEREF)         // for *

OPCODE(WITH_ENTER)
OPCODE(WITH_EXIT)

#endif
    #undef OPCODE
};

static const char* OP_NAMES[] = {
    #define OPCODE(name) #name,
    #ifdef OPCODE

OPCODE(NO_OP)
OPCODE(DELETED_OP)
OPCODE(LOAD_CONST)
OPCODE(IMPORT_NAME)
OPCODE(PRINT_EXPR)
OPCODE(POP_TOP)
OPCODE(CALL)
OPCODE(RETURN_VALUE)

OPCODE(BINARY_OP)
OPCODE(COMPARE_OP)
OPCODE(BITWISE_OP)
OPCODE(IS_OP)
OPCODE(CONTAINS_OP)

OPCODE(UNARY_NEGATIVE)
OPCODE(UNARY_NOT)

OPCODE(DUP_TOP)

OPCODE(BUILD_LIST)
OPCODE(BUILD_MAP)
OPCODE(BUILD_SET)
OPCODE(BUILD_SLICE)

OPCODE(LIST_APPEND)

OPCODE(GET_ITER)
OPCODE(FOR_ITER)

OPCODE(POP_JUMP_IF_FALSE)
OPCODE(JUMP_ABSOLUTE)
OPCODE(SAFE_JUMP_ABSOLUTE)
OPCODE(JUMP_IF_TRUE_OR_POP)
OPCODE(JUMP_IF_FALSE_OR_POP)

// non-standard python opcodes
OPCODE(LOAD_NONE)
OPCODE(LOAD_TRUE)
OPCODE(LOAD_FALSE)
OPCODE(LOAD_EVAL_FN)        // load eval() callable into stack
OPCODE(LOAD_LAMBDA)         // LOAD_CONST + set __module__ attr
OPCODE(LOAD_ELLIPSIS)

OPCODE(ASSERT)
OPCODE(RAISE_ERROR)

OPCODE(STORE_FUNCTION)
OPCODE(BUILD_CLASS)

OPCODE(LOAD_NAME_REF)       // no arg
OPCODE(BUILD_ATTR_REF)      // arg for the name_ptr, [ptr, name_ptr] -> (*ptr).name_ptr
OPCODE(BUILD_INDEX_REF)     // no arg, [ptr, expr] -> (*ptr)[expr]
OPCODE(STORE_NAME_REF)      // arg for the name_ptr, [expr], directly store to the name_ptr without pushing it to the stack
OPCODE(STORE_REF)           // no arg, [ptr, expr] -> *ptr = expr
OPCODE(DELETE_REF)          // no arg, [ptr] -> [] -> delete ptr
OPCODE(BUILD_ATTR_REF_PTR)  // arg for the name_ptr, [ptr, name_ptr] -> (*ptr)->name_ptr

OPCODE(BUILD_SMART_TUPLE)   // if all elements are pointers, build a compound pointer, otherwise build a tuple
OPCODE(BUILD_STRING)        // arg is the expr count, build a string from the top of the stack

OPCODE(GOTO)
OPCODE(UNARY_REF)           // for &
OPCODE(UNARY_DEREF)         // for *

OPCODE(WITH_ENTER)
OPCODE(WITH_EXIT)

#endif
    #undef OPCODE
};

struct ByteCode{
    uint8_t op;
    int arg;
    uint16_t line;
};

_Str pad(const _Str& s, const int n){
    return s + std::string(n - s.size(), ' ');
}

struct CodeObject {
    _Source src;
    _Str name;

    CodeObject(_Source src, _Str name) {
        this->src = src;
        this->name = name;
    }

    CompileMode mode() const {
        return src->mode;
    }

    std::vector<ByteCode> co_code;
    PyVarList co_consts;
    std::vector<std::pair<_Str, NameScope>> co_names;
    std::vector<_Str> co_global_names;

    // for goto use
    // note: some opcodes moves the bytecode, such as listcomp
    // goto/label should be put at toplevel statements
    emhash8::HashMap<_Str, int> co_labels;

    void addLabel(const _Str& label){
        if(co_labels.find(label) != co_labels.end()){
            _Str msg = "label '" + label + "' already exists";
            throw std::runtime_error(msg.c_str());
        }
        co_labels[label] = co_code.size();
    }

    int addName(_Str name, NameScope scope){
        if(scope == NAME_LOCAL && std::find(co_global_names.begin(), co_global_names.end(), name) != co_global_names.end()){
            scope = NAME_GLOBAL;
        }
        auto p = std::make_pair(name, scope);
        for(int i=0; i<co_names.size(); i++){
            if(co_names[i] == p) return i;
        }
        co_names.push_back(p);
        return co_names.size() - 1;
    }

    int addConst(PyVar v){
        co_consts.push_back(v);
        return co_consts.size() - 1;
    }

    void __moveToEnd(int start, int end){
        auto _start = co_code.begin() + start;
        auto _end = co_code.begin() + end;
        co_code.insert(co_code.end(), _start, _end);
        for(int i=start; i<end; i++) co_code[i].op = OP_DELETED_OP;
    }

    _Str toString(){
        _StrStream ss;
        int prev_line = -1;
        for(int i=0; i<co_code.size(); i++){
            const ByteCode& byte = co_code[i];
            if(byte.op == OP_NO_OP) continue;
            _Str line = std::to_string(byte.line);
            if(byte.line == prev_line) line = "";
            else{
                if(prev_line != -1) ss << "\n";
                prev_line = byte.line;
            }
            ss << pad(line, 12) << " " << pad(std::to_string(i), 3);
            ss << " " << pad(OP_NAMES[byte.op], 20) << " ";
            ss << (byte.arg == -1 ? "" : std::to_string(byte.arg));
            if(i != co_code.size() - 1) ss << '\n';
        }
        _StrStream consts;
        consts << "co_consts: ";
        for(int i=0; i<co_consts.size(); i++){
            consts << UNION_TP_NAME(co_consts[i]);
            if(i != co_consts.size() - 1) consts << ", ";
        }

        _StrStream names;
        names << "co_names: ";
        for(int i=0; i<co_names.size(); i++){
            names << co_names[i].first;
            if(i != co_names.size() - 1) names << ", ";
        }
        ss << '\n' << consts.str() << '\n' << names.str() << '\n';
        // for(int i=0; i<co_consts.size(); i++){
        //     auto fn = std::get_if<_Func>(&co_consts[i]->_native);
        //     if(fn) ss << '\n' << (*fn)->code->name << ":\n" << (*fn)->code->toString();
        // }
        return _Str(ss.str());
    }
};

class Frame {
private:
    std::vector<PyVar> s_data;
    int ip = 0;
    std::stack<int> forLoops;       // record the FOR_ITER bytecode index
public:
    const CodeObject* code;
    PyVar _module;
    PyVarDict f_locals;

    uint64_t id;

    inline PyVarDict copy_f_locals(){
        return f_locals;
    }

    inline PyVarDict& f_globals(){
        return _module->attribs;
    }

    Frame(const CodeObject* code, PyVar _module, PyVarDict&& locals)
        : code(code), _module(_module), f_locals(std::move(locals)) {
        
        static uint64_t frame_id = 1;
        id = frame_id++;
    }

    inline const ByteCode& readCode() {
        return code->co_code[ip++];
    }

    _Str errorSnapshot(){
        int line = code->co_code[ip-1].line;
        return code->src->snapshot(line);
    }

    inline int stackSize() const {
        return s_data.size();
    }

    inline bool isCodeEnd() const {
        return ip >= code->co_code.size();
    }

    inline PyVar __pop(){
        if(s_data.empty()) throw std::runtime_error("s_data.empty() is true");
        PyVar v = std::move(s_data.back());
        s_data.pop_back();
        return v;
    }

    inline PyVar __deref_pointer(VM*, PyVar);

    inline PyVar popValue(VM* vm){
        return __deref_pointer(vm, __pop());
    }

    inline PyVar topValue(VM* vm){
        if(s_data.empty()) throw std::runtime_error("s_data.empty() is true");
        return __deref_pointer(vm, s_data.back());
    }

    inline PyVar& __top(){
        if(s_data.empty()) throw std::runtime_error("s_data.empty() is true");
        return s_data.back();
    }

    inline PyVar __topValueN(VM* vm, int n=-1){
        return __deref_pointer(vm, s_data[s_data.size() + n]);
    }

    template<typename T>
    inline void push(T&& obj){
        s_data.push_back(std::forward<T>(obj));
    }


    void __reportForIter(){
        int lastIp = ip - 1;
        if(forLoops.empty()) forLoops.push(lastIp);
        else{
            if(forLoops.top() == lastIp) return;
            if(forLoops.top() < lastIp) forLoops.push(lastIp);
            else UNREACHABLE();
        }
    }

    inline void jump(int i){
        this->ip = i;
    }

    void safeJump(int i){
        this->ip = i;
        while(!forLoops.empty()){
            int start = forLoops.top();
            int end = code->co_code[start].arg;
            if(i < start || i >= end){
                //printf("%d <- [%d, %d)\n", i, start, end);
                __pop();    // pop the iterator
                forLoops.pop();
            }else{
                break;
            }
        }
    }

    pkpy::ArgList popNValuesReversed(VM* vm, int n){
        pkpy::ArgList v(n);
        for(int i=n-1; i>=0; i--) v._index(i) = popValue(vm);
        return v;
    }

    pkpy::ArgList __popNReversed(int n){
        pkpy::ArgList v(n);
        for(int i=n-1; i>=0; i--) v._index(i) = __pop();
        return v;
    }
};


#define __DEF_PY_AS_C(type, ctype, ptype)                       \
    inline ctype& Py##type##_AS_C(const PyVar& obj) {           \
        __checkType(obj, ptype);                                \
        return UNION_GET(ctype, obj);                           \
    }

#define __DEF_PY(type, ctype, ptype)                            \
    inline PyVar Py##type(ctype value) {                        \
        return newObject(ptype, value);                         \
    }

#define DEF_NATIVE(type, ctype, ptype)                          \
    __DEF_PY(type, ctype, ptype)                                \
    __DEF_PY_AS_C(type, ctype, ptype)


class VM {
    std::atomic<bool> _stopFlag = false;
    std::vector<PyVar> _smallIntegers;      // [-5, 256]
protected:
    std::deque< pkpy::unique_ptr<Frame> > callstack;
    PyVarDict _modules;                     // loaded modules
    std::map<_Str, _Code> _lazyModules;     // lazy loaded modules
    PyVar __py2py_call_signal;
    
    void _checkStopFlag(){
        if(_stopFlag){
            _stopFlag = false;
            _error("KeyboardInterrupt", "");
        }
    }

    PyVar runFrame(Frame* frame){
        while(!frame->isCodeEnd()){
            const ByteCode& byte = frame->readCode();
            //printf("[%d] %s (%d)\n", frame->stackSize(), OP_NAMES[byte.op], byte.arg);
            //printf("%s\n", frame->code->src->getLine(byte.line).c_str());

            _checkStopFlag();

            switch (byte.op)
            {
            case OP_NO_OP: break;       // do nothing
            case OP_LOAD_CONST: frame->push(frame->code->co_consts[byte.arg]); break;
            case OP_LOAD_LAMBDA: {
                PyVar obj = frame->code->co_consts[byte.arg];
                setAttr(obj, __module__, frame->_module);
                frame->push(obj);
            } break;
            case OP_LOAD_NAME_REF: {
                frame->push(PyRef(NameRef(
                    &(frame->code->co_names[byte.arg])
                )));
            } break;
            case OP_STORE_NAME_REF: {
                const auto& p = frame->code->co_names[byte.arg];
                NameRef(&p).set(this, frame, frame->popValue(this));
            } break;
            case OP_BUILD_ATTR_REF: {
                const auto& attr = frame->code->co_names[byte.arg];
                PyVar obj = frame->popValue(this);
                frame->push(PyRef(AttrRef(obj, NameRef(&attr))));
            } break;
            case OP_BUILD_ATTR_REF_PTR: {
                const auto& attr = frame->code->co_names[byte.arg];
                PyVar obj = frame->popValue(this);
                __checkType(obj, _tp_user_pointer);
                const PyVarRef& var = UNION_GET(PyVarRef, obj);
                auto p = PyRef_AS_C(var);
                frame->push(PyRef(AttrRef(p->get(this, frame), &attr)));
            } break;
            case OP_BUILD_INDEX_REF: {
                PyVar index = frame->popValue(this);
                PyVarRef obj = frame->popValue(this);
                frame->push(PyRef(IndexRef(obj, index)));
            } break;
            case OP_STORE_REF: {
                PyVar obj = frame->popValue(this);
                PyVarRef r = frame->__pop();
                PyRef_AS_C(r)->set(this, frame, std::move(obj));
            } break;
            case OP_DELETE_REF: {
                PyVarRef r = frame->__pop();
                PyRef_AS_C(r)->del(this, frame);
            } break;
            case OP_BUILD_SMART_TUPLE:
            {
                pkpy::ArgList items = frame->__popNReversed(byte.arg);
                bool done = false;
                for(int i=0; i<items.size(); i++){
                    if(!items[i]->isType(_tp_ref)) {
                        done = true;
                        PyVarList values(items.size());
                        for(int i=0; i<items.size(); i++){
                            values[i] = frame->__deref_pointer(this, items[i]);
                        }
                        frame->push(PyTuple(values));
                        break;
                    }
                }
                if(done) break;
                frame->push(PyRef(TupleRef(items.toList())));
            } break;
            case OP_BUILD_STRING:
            {
                pkpy::ArgList items = frame->popNValuesReversed(this, byte.arg);
                _StrStream ss;
                for(int i=0; i<items.size(); i++) ss << PyStr_AS_C(asStr(items[i]));
                frame->push(PyStr(ss.str()));
            } break;
            case OP_LOAD_EVAL_FN: {
                frame->push(builtins->attribs[m_eval]);
            } break;
            case OP_LIST_APPEND: {
                pkpy::ArgList args(2);
                args[1] = frame->popValue(this);            // obj
                args[0] = frame->__topValueN(this, -2);     // list
                fastCall(m_append, std::move(args));
            } break;
            case OP_STORE_FUNCTION:
                {
                    PyVar obj = frame->popValue(this);
                    const _Func& fn = PyFunction_AS_C(obj);
                    setAttr(obj, __module__, frame->_module);
                    frame->f_globals()[fn->name] = obj;
                } break;
            case OP_BUILD_CLASS:
                {
                    const _Str& clsName = frame->code->co_names[byte.arg].first;
                    PyVar clsBase = frame->popValue(this);
                    if(clsBase == None) clsBase = _tp_object;
                    __checkType(clsBase, _tp_type);
                    PyVar cls = newUserClassType(clsName, clsBase);
                    while(true){
                        PyVar fn = frame->popValue(this);
                        if(fn == None) break;
                        const _Func& f = PyFunction_AS_C(fn);
                        setAttr(fn, __module__, frame->_module);
                        setAttr(cls, f->name, fn);
                    }
                    frame->f_globals()[clsName] = cls;
                } break;
            case OP_RETURN_VALUE: return frame->popValue(this);
            case OP_PRINT_EXPR:
                {
                    const PyVar expr = frame->topValue(this);
                    if(expr == None) break;
                    *_stdout << PyStr_AS_C(asRepr(expr)) << '\n';
                } break;
            case OP_POP_TOP: frame->popValue(this); break;
            case OP_BINARY_OP:
                {
                    frame->push(
                        fastCall(BINARY_SPECIAL_METHODS[byte.arg],
                        frame->popNValuesReversed(this, 2))
                    );
                } break;
            case OP_BITWISE_OP:
                {
                    frame->push(
                        fastCall(BITWISE_SPECIAL_METHODS[byte.arg],
                        frame->popNValuesReversed(this, 2))
                    );
                } break;
            case OP_COMPARE_OP:
                {
                    // for __ne__ we use the negation of __eq__
                    int op = byte.arg == 3 ? 2 : byte.arg;
                    PyVar res = fastCall(CMP_SPECIAL_METHODS[op], frame->popNValuesReversed(this, 2));
                    if(op != byte.arg) res = PyBool(!PyBool_AS_C(res));
                    frame->push(std::move(res));
                } break;
            case OP_IS_OP:
                {
                    bool ret_c = frame->popValue(this) == frame->popValue(this);
                    if(byte.arg == 1) ret_c = !ret_c;
                    frame->push(PyBool(ret_c));
                } break;
            case OP_CONTAINS_OP:
                {
                    PyVar rhs = frame->popValue(this);
                    bool ret_c = PyBool_AS_C(call(rhs, __contains__, pkpy::oneArg(frame->popValue(this))));
                    if(byte.arg == 1) ret_c = !ret_c;
                    frame->push(PyBool(ret_c));
                } break;
            case OP_UNARY_NEGATIVE:
                {
                    PyVar obj = frame->popValue(this);
                    frame->push(numNegated(obj));
                } break;
            case OP_UNARY_NOT:
                {
                    PyVar obj = frame->popValue(this);
                    const PyVar& obj_bool = asBool(obj);
                    frame->push(PyBool(!PyBool_AS_C(obj_bool)));
                } break;
            case OP_UNARY_REF:
                {
                    // _pointer to pointer
                    PyVarRef obj = frame->__pop();
                    __checkType(obj, _tp_ref);
                    frame->push(newObject(
                        _tp_user_pointer,
                        PyRef(UserPointer(obj, frame->id))
                    ));
                } break;
            case OP_UNARY_DEREF:
                {
                    // pointer to _pointer
                    PyVar obj = frame->popValue(this);
                    __checkType(obj, _tp_user_pointer);
                    frame->push(UNION_GET(PyVarRef, obj));
                } break;
            case OP_POP_JUMP_IF_FALSE:
                if(!PyBool_AS_C(asBool(frame->popValue(this)))) frame->jump(byte.arg);
                break;
            case OP_LOAD_NONE: frame->push(None); break;
            case OP_LOAD_TRUE: frame->push(True); break;
            case OP_LOAD_FALSE: frame->push(False); break;
            case OP_LOAD_ELLIPSIS: frame->push(Ellipsis); break;
            case OP_ASSERT:
                {
                    PyVar expr = frame->popValue(this);
                    _assert(PyBool_AS_C(expr), "assertion failed");
                } break;
            case OP_RAISE_ERROR:
                {
                    _Str msg = PyStr_AS_C(asRepr(frame->popValue(this)));
                    _Str type = PyStr_AS_C(frame->popValue(this));
                    _error(type, msg);
                } break;
            case OP_BUILD_LIST:
                {
                    pkpy::ArgList items = frame->popNValuesReversed(this, byte.arg);
                    frame->push(PyList(items.toList()));
                } break;
            case OP_BUILD_MAP:
                {
                    pkpy::ArgList items = frame->popNValuesReversed(this, byte.arg*2);
                    PyVar obj = call(builtins->attribs["dict"]);
                    for(int i=0; i<items.size(); i+=2){
                        call(obj, __setitem__, pkpy::twoArgs(items[i], items[i+1]));
                    }
                    frame->push(obj);
                } break;
            case OP_BUILD_SET:
                {
                    pkpy::ArgList items = frame->popNValuesReversed(this, byte.arg);
                    PyVar list = PyList(items.toList());
                    PyVar obj = call(builtins->attribs["set"], pkpy::oneArg(list));
                    frame->push(obj);
                } break;
            case OP_DUP_TOP: frame->push(frame->topValue(this)); break;
            case OP_CALL:
                {
                    int ARGC = byte.arg & 0xFFFF;
                    int KWARGC = (byte.arg >> 16) & 0xFFFF;
                    pkpy::ArgList kwargs(0);
                    if(KWARGC > 0) kwargs = frame->popNValuesReversed(this, KWARGC*2);
                    pkpy::ArgList args = frame->popNValuesReversed(this, ARGC);
                    PyVar callable = frame->popValue(this);
                    PyVar ret = call(callable, std::move(args), kwargs, true);
                    if(ret == __py2py_call_signal) return ret;
                    frame->push(std::move(ret));
                } break;
            case OP_JUMP_ABSOLUTE: frame->jump(byte.arg); break;
            case OP_SAFE_JUMP_ABSOLUTE: frame->safeJump(byte.arg); break;
            case OP_GOTO: {
                PyVar obj = frame->popValue(this);
                const _Str& label = PyStr_AS_C(obj);
                int* target = frame->code->co_labels.try_get(label);
                if(target == nullptr){
                    _error("KeyError", "label '" + label + "' not found");
                }
                frame->safeJump(*target);
            } break;
            case OP_GET_ITER:
                {
                    PyVar obj = frame->popValue(this);
                    PyVarOrNull iter_fn = getAttr(obj, __iter__, false);
                    if(iter_fn != nullptr){
                        PyVar tmp = call(iter_fn);
                        PyVarRef var = frame->__pop();
                        __checkType(var, _tp_ref);
                        PyIter_AS_C(tmp)->var = var;
                        frame->push(std::move(tmp));
                    }else{
                        typeError("'" + UNION_TP_NAME(obj) + "' object is not iterable");
                    }
                } break;
            case OP_FOR_ITER:
                {
                    frame->__reportForIter();
                    // __top() must be PyIter, so no need to __deref()
                    auto& it = PyIter_AS_C(frame->__top());
                    if(it->hasNext()){
                        PyRef_AS_C(it->var)->set(this, frame, it->next());
                    }
                    else{
                        frame->safeJump(byte.arg);
                    }
                } break;
            case OP_JUMP_IF_FALSE_OR_POP:
                {
                    const PyVar expr = frame->topValue(this);
                    if(asBool(expr)==False) frame->jump(byte.arg);
                    else frame->popValue(this);
                } break;
            case OP_JUMP_IF_TRUE_OR_POP:
                {
                    const PyVar expr = frame->topValue(this);
                    if(asBool(expr)==True) frame->jump(byte.arg);
                    else frame->popValue(this);
                } break;
            case OP_BUILD_SLICE:
                {
                    PyVar stop = frame->popValue(this);
                    PyVar start = frame->popValue(this);
                    _Slice s;
                    if(start != None) {__checkType(start, _tp_int); s.start = (int)PyInt_AS_C(start);}
                    if(stop != None) {__checkType(stop, _tp_int); s.stop = (int)PyInt_AS_C(stop);}
                    frame->push(PySlice(s));
                } break;
            case OP_IMPORT_NAME:
                {
                    const _Str& name = frame->code->co_names[byte.arg].first;
                    auto it = _modules.find(name);
                    if(it == _modules.end()){
                        auto it2 = _lazyModules.find(name);
                        if(it2 == _lazyModules.end()){
                            _error("ImportError", "module '" + name + "' not found");
                        }else{
                            _Code code = it2->second;
                            PyVar _m = newModule(name);
                            _exec(code, _m, {});
                            frame->push(_m);
                            _lazyModules.erase(it2);
                        }
                    }else{
                        frame->push(it->second);
                    }
                } break;
            case OP_WITH_ENTER: call(frame->popValue(this), __enter__); break;
            case OP_WITH_EXIT: call(frame->popValue(this), __exit__); break;
            default:
                systemError(_Str("opcode ") + OP_NAMES[byte.op] + " is not implemented");
                break;
            }
        }

        if(frame->code->src->mode == EVAL_MODE || frame->code->src->mode == JSON_MODE){
            if(frame->stackSize() != 1) systemError("stack size is not 1 in EVAL_MODE/JSON_MODE");
            return frame->popValue(this);
        }

        if(frame->stackSize() != 0) systemError("stack not empty in EXEC_MODE");
        return None;
    }

public:
    PyVarDict _types;
    PyVar None, True, False, Ellipsis;

    bool use_stdio;
    std::ostream* _stdout;
    std::ostream* _stderr;
    
    PyVar builtins;         // builtins module
    PyVar _main;            // __main__ module

    int maxRecursionDepth = 1000;

    VM(bool use_stdio){
        this->use_stdio = use_stdio;
        if(use_stdio){
            std::cout.setf(std::ios::unitbuf);
            std::cerr.setf(std::ios::unitbuf);
            this->_stdout = &std::cout;
            this->_stderr = &std::cerr;
        }else{
            this->_stdout = new _StrStream();
            this->_stderr = new _StrStream();
        }
        initializeBuiltinClasses();

        _smallIntegers.reserve(300);
        for(_Int i=-5; i<=256; i++) _smallIntegers.push_back(newObject(_tp_int, i));
    }

    void keyboardInterrupt(){
        _stopFlag = true;
    }

    void sleepForSecs(_Float sec){
        _Int ms = (_Int)(sec * 1000);
        for(_Int i=0; i<ms; i+=20){
            _checkStopFlag();
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    }

    PyVar asStr(const PyVar& obj){
        PyVarOrNull str_fn = getAttr(obj, __str__, false);
        if(str_fn != nullptr) return call(str_fn);
        return asRepr(obj);
    }

    Frame* __findFrame(uint64_t up_f_id){
        for(auto it=callstack.crbegin(); it!=callstack.crend(); ++it){
            uint64_t f_id = it->get()->id;
            if(f_id == up_f_id) return it->get();
            if(f_id < up_f_id) return nullptr;
        }
        return nullptr;
    }

    Frame* topFrame(){
        if(callstack.size() == 0) UNREACHABLE();
        return callstack.back().get();
    }

    PyVar asRepr(const PyVar& obj){
        if(obj->isType(_tp_type)) return PyStr("<class '" + UNION_GET(_Str, obj->attribs[__name__]) + "'>");
        return call(obj, __repr__);
    }

    PyVar asJson(const PyVar& obj){
        return call(obj, __json__);
    }

    const PyVar& asBool(const PyVar& obj){
        if(obj == None) return False;
        if(obj->_type == _tp_bool) return obj;
        if(obj->_type == _tp_int) return PyBool(PyInt_AS_C(obj) != 0);
        if(obj->_type == _tp_float) return PyBool(PyFloat_AS_C(obj) != 0.0);
        PyVarOrNull len_fn = getAttr(obj, __len__, false);
        if(len_fn != nullptr){
            PyVar ret = call(len_fn);
            return PyBool(PyInt_AS_C(ret) > 0);
        }
        return True;
    }

    PyVar fastCall(const _Str& name, pkpy::ArgList&& args){
        PyObject* cls = args[0]->_type.get();
        while(cls != None.get()) {
            PyVar* val = cls->attribs.try_get(name);
            if(val != nullptr) return call(*val, std::move(args));
            cls = cls->attribs[__base__].get();
        }
        attributeError(args[0], name);
        return nullptr;
    }

    inline PyVar call(const PyVar& _callable){
        return call(_callable, pkpy::noArg(), pkpy::noArg(), false);
    }

    template<typename ArgT>
    inline std::enable_if_t<std::is_same_v<std::remove_const_t<std::remove_reference_t<ArgT>>, pkpy::ArgList>, PyVar>
    call(const PyVar& _callable, ArgT&& args){
        return call(_callable, std::forward<ArgT>(args), pkpy::noArg(), false);
    }

    template<typename ArgT>
    inline std::enable_if_t<std::is_same_v<std::remove_const_t<std::remove_reference_t<ArgT>>, pkpy::ArgList>, PyVar>
    call(const PyVar& obj, const _Str& func, ArgT&& args){
        return call(getAttr(obj, func), std::forward<ArgT>(args), pkpy::noArg(), false);
    }

    inline PyVar call(const PyVar& obj, const _Str& func){
        return call(getAttr(obj, func), pkpy::noArg(), pkpy::noArg(), false);
    }

    PyVar call(const PyVar& _callable, pkpy::ArgList args, const pkpy::ArgList& kwargs, bool opCall){
        if(_callable->isType(_tp_type)){
            auto it = _callable->attribs.find(__new__);
            PyVar obj;
            if(it != _callable->attribs.end()){
                obj = call(it->second, args, kwargs, false);
            }else{
                obj = newObject(_callable, (_Int)-1);
                PyVarOrNull init_fn = getAttr(obj, __init__, false);
                if (init_fn != nullptr) call(init_fn, args, kwargs, false);
            }
            return obj;
        }

        const PyVar* callable = &_callable;
        if((*callable)->isType(_tp_bounded_method)){
            auto& bm = PyBoundedMethod_AS_C((*callable));
            // TODO: avoid insertion here, bad performance
            pkpy::ArgList new_args(args.size()+1);
            new_args[0] = bm.obj;
            for(int i=0; i<args.size(); i++) new_args[i+1] = args[i];
            callable = &bm.method;
            args = std::move(new_args);
        }
        
        if((*callable)->isType(_tp_native_function)){
            const auto& f = UNION_GET(_CppFunc, *callable);
            // _CppFunc do not support kwargs
            return f(this, args);
        } else if((*callable)->isType(_tp_function)){
            const _Func& fn = PyFunction_AS_C((*callable));
            PyVarDict locals;
            int i = 0;
            for(const auto& name : fn->args){
                if(i < args.size()){
                    locals.emplace(name, args[i++]);
                    continue;
                }
                typeError("missing positional argument '" + name + "'");
            }

            locals.insert(fn->kwArgs.begin(), fn->kwArgs.end());

            std::vector<_Str> positional_overrided_keys;
            if(!fn->starredArg.empty()){
                // handle *args
                PyVarList vargs;
                while(i < args.size()) vargs.push_back(args[i++]);
                locals.emplace(fn->starredArg, PyTuple(std::move(vargs)));
            }else{
                for(const auto& key : fn->kwArgsOrder){
                    if(i < args.size()){
                        locals[key] = args[i++];
                        positional_overrided_keys.push_back(key);
                    }else{
                        break;
                    }
                }
                if(i < args.size()) typeError("too many arguments");
            }
            
            for(int i=0; i<kwargs.size(); i+=2){
                const _Str& key = PyStr_AS_C(kwargs[i]);
                if(!fn->kwArgs.contains(key)){
                    typeError(key.__escape(true) + " is an invalid keyword argument for " + fn->name + "()");
                }
                const PyVar& val = kwargs[i+1];
                if(!positional_overrided_keys.empty()){
                    auto it = std::find(positional_overrided_keys.begin(), positional_overrided_keys.end(), key);
                    if(it != positional_overrided_keys.end()){
                        typeError("multiple values for argument '" + key + "'");
                    }
                }
                locals[key] = val;
            }

            PyVar* it_m = (*callable)->attribs.try_get(__module__);
            PyVar _module = it_m != nullptr ? *it_m : topFrame()->_module;
            if(opCall){
                __pushNewFrame(fn->code, _module, std::move(locals));
                return __py2py_call_signal;
            }
            return _exec(fn->code, _module, std::move(locals));
        }
        typeError("'" + UNION_TP_NAME(*callable) + "' object is not callable");
        return None;
    }


    // repl mode is only for setting `frame->id` to 0
    virtual PyVarOrNull exec(const _Code& code, PyVar _module=nullptr){
        if(_module == nullptr) _module = _main;
        try {
            return _exec(code, _module, {});
        }catch (const _Error& e){
            *_stderr << e.what() << '\n';
        }catch (const std::exception& e) {
            auto re = RuntimeError("UnexpectedError", e.what(), _cleanErrorAndGetSnapshots());
            *_stderr << re.what() << '\n';
        }
        return nullptr;
    }

    virtual void execAsync(const _Code& code) {
        exec(code);
    }

    Frame* __pushNewFrame(const _Code& code, PyVar _module, PyVarDict&& locals){
        if(code == nullptr) UNREACHABLE();
        if(callstack.size() > maxRecursionDepth){
            throw RuntimeError("RecursionError", "maximum recursion depth exceeded", _cleanErrorAndGetSnapshots());
        }
        Frame* frame = new Frame(code.get(), _module, std::move(locals));
        callstack.emplace_back(pkpy::unique_ptr<Frame>(frame));
        return frame;
    }

    PyVar _exec(const _Code& code, PyVar _module, PyVarDict&& locals){
        Frame* frame = __pushNewFrame(code, _module, std::move(locals));
        if(code->mode() == SINGLE_MODE) frame->id = 0;
        Frame* frameBase = frame;
        PyVar ret = nullptr;

        while(true){
            ret = runFrame(frame);
            if(ret != __py2py_call_signal){
                if(frame == frameBase){         // [ frameBase<- ]
                    break;
                }else{
                    callstack.pop_back();
                    frame = callstack.back().get();
                    frame->push(ret);
                }
            }else{
                frame = callstack.back().get();  // [ frameBase, newFrame<- ]
            }
        }

        callstack.pop_back();
        return ret;
    }

    PyVar newUserClassType(_Str name, PyVar base){
        PyVar obj = newClassType(name, base);
        setAttr(obj, __name__, PyStr(name));
        _types.erase(name);
        return obj;
    }

    PyVar newClassType(_Str name, PyVar base=nullptr) {
        if(base == nullptr) base = _tp_object;
        PyVar obj = pkpy::make_shared<PyObject, Py_<_Int>>((_Int)0, _tp_type);
        setAttr(obj, __base__, base);
        _types[name] = obj;
        return obj;
    }

    template<typename T>
    inline PyVar newObject(PyVar type, T _value) {
        __checkType(type, _tp_type);
        return pkpy::make_shared<PyObject, Py_<T>>(_value, type);
    }

    PyVar newModule(_Str name) {
        PyVar obj = newObject(_tp_module, (_Int)-2);
        setAttr(obj, __name__, PyStr(name));
        _modules[name] = obj;
        return obj;
    }

    void addLazyModule(_Str name, _Code code){
        _lazyModules[name] = code;
    }

    PyVarOrNull getAttr(const PyVar& obj, const _Str& name, bool throw_err=true) {
        PyVarDict::iterator it;
        PyObject* cls;

        if(obj->isType(_tp_super)){
            const PyVar* root = &obj;
            int depth = 1;
            while(true){
                root = &UNION_GET(PyVar, *root);
                if(!(*root)->isType(_tp_super)) break;
                depth++;
            }
            cls = (*root)->_type.get();
            for(int i=0; i<depth; i++) cls = cls->attribs[__base__].get();

            it = (*root)->attribs.find(name);
            if(it != (*root)->attribs.end()) return it->second;        
        }else{
            it = obj->attribs.find(name);
            if(it != obj->attribs.end()) return it->second;
            cls = obj->_type.get();
        }

        while(cls != None.get()) {
            it = cls->attribs.find(name);
            if(it != cls->attribs.end()){
                PyVar valueFromCls = it->second;
                if(valueFromCls->isType(_tp_function) || valueFromCls->isType(_tp_native_function)){
                    return PyBoundedMethod({obj, std::move(valueFromCls)});
                }else{
                    return valueFromCls;
                }
            }
            cls = cls->attribs[__base__].get();
        }
        if(throw_err) attributeError(obj, name);
        return nullptr;
    }

    void setAttr(PyVar& obj, const _Str& name, const PyVar& value) {
        if(obj->isType(_tp_super)){
            const PyVar* root = &obj;
            while(true){
                root = &UNION_GET(PyVar, *root);
                if(!(*root)->isType(_tp_super)) break;
            }
            (*root)->attribs[name] = value;
        }else{
            obj->attribs[name] = value;
        }
    }

    void setAttr(PyVar& obj, const _Str& name, PyVar&& value) {
        if(obj->isType(_tp_super)){
            const PyVar* root = &obj;
            while(true){
                root = &UNION_GET(PyVar, *root);
                if(!(*root)->isType(_tp_super)) break;
            }
            (*root)->attribs[name] = std::move(value);
        }else{
            obj->attribs[name] = std::move(value);
        }
    }

    void bindMethod(_Str typeName, _Str funcName, _CppFunc fn) {
        PyVar type = _types[typeName];
        PyVar func = PyNativeFunction(fn);
        setAttr(type, funcName, func);
    }

    void bindMethodMulti(std::vector<_Str> typeNames, _Str funcName, _CppFunc fn) {
        for(auto& typeName : typeNames){
            bindMethod(typeName, funcName, fn);
        }
    }

    void bindBuiltinFunc(_Str funcName, _CppFunc fn) {
        bindFunc(builtins, funcName, fn);
    }

    void bindFunc(PyVar module, _Str funcName, _CppFunc fn) {
        __checkType(module, _tp_module);
        PyVar func = PyNativeFunction(fn);
        setAttr(module, funcName, func);
    }

    bool isInstance(PyVar obj, PyVar type){
        __checkType(type, _tp_type);
        PyObject* t = obj->_type.get();
        while (t != None.get()){
            if (t == type.get()) return true;
            t = t->attribs[__base__].get();
        }
        return false;
    }

    inline bool isIntOrFloat(const PyVar& obj){
        return obj->isType(_tp_int) || obj->isType(_tp_float);
    }

    inline bool isIntOrFloat(const PyVar& obj1, const PyVar& obj2){
        return isIntOrFloat(obj1) && isIntOrFloat(obj2);
    }

    inline _Float numToFloat(const PyVar& obj){
        if (obj->isType(_tp_int)){
            return (_Float)PyInt_AS_C(obj);
        }else if(obj->isType(_tp_float)){
            return PyFloat_AS_C(obj);
        }
        UNREACHABLE();
    }

    PyVar numNegated(const PyVar& obj){
        if (obj->isType(_tp_int)){
            return PyInt(-PyInt_AS_C(obj));
        }else if(obj->isType(_tp_float)){
            return PyFloat(-PyFloat_AS_C(obj));
        }
        typeError("unsupported operand type(s) for -");
        return nullptr;
    }

    int normalizedIndex(int index, int size){
        if(index < 0) index += size;
        if(index < 0 || index >= size){
            indexError("index out of range, " + std::to_string(index) + " not in [0, " + std::to_string(size) + ")");
        }
        return index;
    }

    // for quick access
    PyVar _tp_object, _tp_type, _tp_int, _tp_float, _tp_bool, _tp_str;
    PyVar _tp_list, _tp_tuple;
    PyVar _tp_function, _tp_native_function, _tp_native_iterator, _tp_bounded_method;
    PyVar _tp_slice, _tp_range, _tp_module, _tp_ref;
    PyVar _tp_user_pointer, _tp_super;

    template<typename P>
    inline PyVarRef PyRef(P&& value) {
        static_assert(std::is_base_of<BaseRef, P>::value, "P should derive from BaseRef");
        return newObject(_tp_ref, std::forward<P>(value));
    }

    inline const BaseRef* PyRef_AS_C(const PyVar& obj)
    {
        if(!obj->isType(_tp_ref)) typeError("expected an l-value");
        return (const BaseRef*)(obj->value());
    }

    __DEF_PY_AS_C(Int, _Int, _tp_int)
    inline PyVar PyInt(_Int value) { 
        if(value >= -5 && value <= 256) return _smallIntegers[value + 5];
        return newObject(_tp_int, value);
    }

    DEF_NATIVE(Float, _Float, _tp_float)
    DEF_NATIVE(Str, _Str, _tp_str)
    DEF_NATIVE(List, PyVarList, _tp_list)
    DEF_NATIVE(Tuple, PyVarList, _tp_tuple)
    DEF_NATIVE(Function, _Func, _tp_function)
    DEF_NATIVE(NativeFunction, _CppFunc, _tp_native_function)
    DEF_NATIVE(Iter, _Iterator, _tp_native_iterator)
    DEF_NATIVE(BoundedMethod, _BoundedMethod, _tp_bounded_method)
    DEF_NATIVE(Range, _Range, _tp_range)
    DEF_NATIVE(Slice, _Slice, _tp_slice)
    
    // there is only one True/False, so no need to copy them!
    inline bool PyBool_AS_C(const PyVar& obj){return obj == True;}
    inline const PyVar& PyBool(bool value){return value ? True : False;}

    void initializeBuiltinClasses(){
        _tp_object = pkpy::make_shared<PyObject, Py_<_Int>>((_Int)0, nullptr);
        _tp_type = pkpy::make_shared<PyObject, Py_<_Int>>((_Int)0, nullptr);

        _types["object"] = _tp_object;
        _types["type"] = _tp_type;

        _tp_bool = newClassType("bool");
        _tp_int = newClassType("int");
        _tp_float = newClassType("float");
        _tp_str = newClassType("str");
        _tp_list = newClassType("list");
        _tp_tuple = newClassType("tuple");
        _tp_slice = newClassType("slice");
        _tp_range = newClassType("range");
        _tp_module = newClassType("module");
        _tp_ref = newClassType("_pointer");
        _tp_user_pointer = newClassType("pointer");

        newClassType("NoneType");
        newClassType("ellipsis");
        
        _tp_function = newClassType("function");
        _tp_native_function = newClassType("_native_function");
        _tp_native_iterator = newClassType("_native_iterator");
        _tp_bounded_method = newClassType("_bounded_method");
        _tp_super = newClassType("super");

        this->None = newObject(_types["NoneType"], (_Int)0);
        this->Ellipsis = newObject(_types["ellipsis"], (_Int)0);
        this->True = newObject(_tp_bool, true);
        this->False = newObject(_tp_bool, false);
        this->builtins = newModule("builtins");
        this->_main = newModule("__main__"_c);

        setAttr(_tp_type, __base__, _tp_object);
        _tp_type->_type = _tp_type;
        setAttr(_tp_object, __base__, None);
        _tp_object->_type = _tp_type;
        
        for (auto& [name, type] : _types) {
            setAttr(type, __name__, PyStr(name));
        }

        this->__py2py_call_signal = newObject(_tp_object, (_Int)7);

        std::vector<_Str> publicTypes = {"type", "object", "bool", "int", "float", "str", "list", "tuple", "range"};
        for (auto& name : publicTypes) {
            setAttr(builtins, name, _types[name]);
        }
    }

    _Int hash(const PyVar& obj){
        if (obj->isType(_tp_int)) return PyInt_AS_C(obj);
        if (obj->isType(_tp_bool)) return PyBool_AS_C(obj) ? 1 : 0;
        if (obj->isType(_tp_float)){
            _Float val = PyFloat_AS_C(obj);
            return (_Int)std::hash<_Float>()(val);
        }
        if (obj->isType(_tp_str)) return PyStr_AS_C(obj).hash();
        if (obj->isType(_tp_type)) return (_Int)obj.get();
        if (obj->isType(_tp_tuple)) {
            _Int x = 1000003;
            for (const auto& item : PyTuple_AS_C(obj)) {
                _Int y = hash(item);
                // this is recommended by Github Copilot
                // i am not sure whether it is a good idea
                x = x ^ (y + 0x9e3779b9 + (x << 6) + (x >> 2));
            }
            return x;
        }
        typeError("unhashable type: " +  UNION_TP_NAME(obj));
        return 0;
    }

    /***** Error Reporter *****/
private:
    void _error(const _Str& name, const _Str& msg){
        throw RuntimeError(name, msg, _cleanErrorAndGetSnapshots());
    }

    std::stack<_Str> _cleanErrorAndGetSnapshots(){
        std::stack<_Str> snapshots;
        while (!callstack.empty()){
            if(snapshots.size() < 8){
                snapshots.push(callstack.back()->errorSnapshot());
            }
            callstack.pop_back();
        }
        return snapshots;
    }

public:
    void typeError(const _Str& msg){
        _error("TypeError", msg);
    }

    void systemError(const _Str& msg){
        _error("SystemError", msg);
    }

    void nullPointerError(){
        _error("NullPointerError", "pointer is invalid");
    }

    void zeroDivisionError(){
        _error("ZeroDivisionError", "division by zero");
    }

    void indexError(const _Str& msg){
        _error("IndexError", msg);
    }

    void valueError(const _Str& msg){
        _error("ValueError", msg);
    }

    void nameError(const _Str& name){
        _error("NameError", "name '" + name + "' is not defined");
    }

    void attributeError(PyVar obj, const _Str& name){
        _error("AttributeError", "type '" +  UNION_TP_NAME(obj) + "' has no attribute '" + name + "'");
    }

    inline void __checkType(const PyVar& obj, const PyVar& type){
#ifndef PKPY_NO_TYPE_CHECK
        if(!obj->isType(type)) typeError("expected '" + UNION_NAME(type) + "', but got '" + UNION_TP_NAME(obj) + "'");
#endif
    }

    inline void __checkArgSize(const pkpy::ArgList& args, int size, bool method=false){
        if(args.size() == size) return;
        if(method) typeError(args.size()>size ? "too many arguments" : "too few arguments");
        else typeError("expected " + std::to_string(size) + " arguments, but got " + std::to_string(args.size()));
    }

    void _assert(bool val, const _Str& msg){
        if (!val) _error("AssertionError", msg);
    }

    virtual ~VM() {
        if(!use_stdio){
            delete _stdout;
            delete _stderr;
        }
    }
};

/***** Pointers' Impl *****/

PyVar NameRef::get(VM* vm, Frame* frame) const{
    PyVar* val = frame->f_locals.try_get(pair->first);
    if(val) return *val;
    val = frame->f_globals().try_get(pair->first);
    if(val) return *val;
    val = vm->builtins->attribs.try_get(pair->first);
    if(val) return *val;
    vm->nameError(pair->first);
    return nullptr;
}

void NameRef::set(VM* vm, Frame* frame, PyVar val) const{
    switch(pair->second) {
        case NAME_LOCAL: frame->f_locals[pair->first] = std::move(val); break;
        case NAME_GLOBAL:
        {
            if(frame->f_locals.count(pair->first) > 0){
                frame->f_locals[pair->first] = std::move(val);
            }else{
                frame->f_globals()[pair->first] = std::move(val);
            }
        } break;
        default: UNREACHABLE();
    }
}

void NameRef::del(VM* vm, Frame* frame) const{
    switch(pair->second) {
        case NAME_LOCAL: {
            if(frame->f_locals.count(pair->first) > 0){
                frame->f_locals.erase(pair->first);
            }else{
                vm->nameError(pair->first);
            }
        } break;
        case NAME_GLOBAL:
        {
            if(frame->f_locals.count(pair->first) > 0){
                frame->f_locals.erase(pair->first);
            }else{
                if(frame->f_globals().count(pair->first) > 0){
                    frame->f_globals().erase(pair->first);
                }else{
                    vm->nameError(pair->first);
                }
            }
        } break;
        default: UNREACHABLE();
    }
}

PyVar AttrRef::get(VM* vm, Frame* frame) const{
    return vm->getAttr(obj, attr.pair->first);
}

void AttrRef::set(VM* vm, Frame* frame, PyVar val) const{
    vm->setAttr(obj, attr.pair->first, val);
}

void AttrRef::del(VM* vm, Frame* frame) const{
    vm->typeError("cannot delete attribute");
}

PyVar IndexRef::get(VM* vm, Frame* frame) const{
    return vm->call(obj, __getitem__, pkpy::oneArg(index));
}

void IndexRef::set(VM* vm, Frame* frame, PyVar val) const{
    vm->call(obj, __setitem__, pkpy::twoArgs(index, val));
}

void IndexRef::del(VM* vm, Frame* frame) const{
    vm->call(obj, __delitem__, pkpy::oneArg(index));
}

PyVar TupleRef::get(VM* vm, Frame* frame) const{
    PyVarList args(varRefs.size());
    for (int i = 0; i < varRefs.size(); i++) {
        args[i] = vm->PyRef_AS_C(varRefs[i])->get(vm, frame);
    }
    return vm->PyTuple(args);
}

void TupleRef::set(VM* vm, Frame* frame, PyVar val) const{
    if(!val->isType(vm->_tp_tuple) && !val->isType(vm->_tp_list)){
        vm->typeError("only tuple or list can be unpacked");
    }
    const PyVarList& args = UNION_GET(PyVarList, val);
    if(args.size() > varRefs.size()) vm->valueError("too many values to unpack");
    if(args.size() < varRefs.size()) vm->valueError("not enough values to unpack");
    for (int i = 0; i < varRefs.size(); i++) {
        vm->PyRef_AS_C(varRefs[i])->set(vm, frame, args[i]);
    }
}

void TupleRef::del(VM* vm, Frame* frame) const{
    for (auto& r : varRefs) vm->PyRef_AS_C(r)->del(vm, frame);
}

PyVar UserPointer::get(VM* vm, Frame* frame) const{
    frame = vm->__findFrame(f_id);
    if(frame == nullptr) vm->nullPointerError();
    return vm->PyRef_AS_C(p)->get(vm, frame);
}

void UserPointer::set(VM* vm, Frame* frame, PyVar val) const{
    frame = vm->__findFrame(f_id);
    if(frame == nullptr) vm->nullPointerError();
    vm->PyRef_AS_C(p)->set(vm, frame, val);
}

void UserPointer::del(VM* vm, Frame* frame) const{
    vm->typeError("delete is unsupported");
}

/***** Frame's Impl *****/
inline PyVar Frame::__deref_pointer(VM* vm, PyVar v){
    if(v->isType(vm->_tp_ref)) return vm->PyRef_AS_C(v)->get(vm, this);
    return v;
}

/***** Iterators' Impl *****/
PyVar RangeIterator::next(){
    PyVar val = vm->PyInt(current);
    current += r.step;
    return val;
}

PyVar StringIterator::next(){
    return vm->PyStr(str.u8_getitem(index++));
}

enum ThreadState {
    THREAD_READY,
    THREAD_RUNNING,
    THREAD_SUSPENDED,
    THREAD_FINISHED
};

class ThreadedVM : public VM {
    std::thread* _thread = nullptr;
    std::atomic<ThreadState> _state = THREAD_READY;
    _Str _sharedStr = "";
    
    void __deleteThread(){
        if(_thread != nullptr){
            terminate();
            _thread->join();
            delete _thread;
            _thread = nullptr;
        }
    }
public:
    ThreadedVM(bool use_stdio) : VM(use_stdio) {
        bindBuiltinFunc("__string_channel_call", [](VM* vm, const pkpy::ArgList& args){
            vm->__checkArgSize(args, 1);
            _Str data = vm->PyStr_AS_C(args[0]);

            ThreadedVM* tvm = (ThreadedVM*)vm;
            tvm->_sharedStr = data;
            tvm->suspend();
            return tvm->PyStr(tvm->readJsonRpcRequest());
        });
    }

    void terminate(){
        if(_state == THREAD_RUNNING || _state == THREAD_SUSPENDED){
            keyboardInterrupt();
            while(_state != THREAD_FINISHED);
        }
    }

    void suspend(){
        if(_state != THREAD_RUNNING) UNREACHABLE();
        _state = THREAD_SUSPENDED;
        while(_state == THREAD_SUSPENDED) _checkStopFlag();
    }

    _Str readJsonRpcRequest(){
        _Str copy = _sharedStr;
        _sharedStr = "";
        return copy;
    }

    /***** For outer use *****/

    ThreadState getState(){
        return _state;
    }

    void writeJsonrpcResponse(const char* value){
        if(_state != THREAD_SUSPENDED) UNREACHABLE();
        _sharedStr = _Str(value);
        _state = THREAD_RUNNING;
    }

    void execAsync(const _Code& code) override {
        if(_state != THREAD_READY) UNREACHABLE();
        __deleteThread();
        _thread = new std::thread([this, code](){
            this->_state = THREAD_RUNNING;
            VM::exec(code);
            this->_state = THREAD_FINISHED;
        });
    }

    PyVarOrNull exec(const _Code& code, PyVar _module = nullptr) override {
        if(_state == THREAD_READY) return VM::exec(code, _module);
        auto callstackBackup = std::move(callstack);
        callstack.clear();
        PyVarOrNull ret = VM::exec(code, _module);
        callstack = std::move(callstackBackup);
        return ret;
    }

    void resetState(){
        if(this->_state != THREAD_FINISHED) return;
        this->_state = THREAD_READY;
    }

    ~ThreadedVM(){
        __deleteThread();
    }
};


class Compiler;

typedef void (Compiler::*GrammarFn)();
typedef void (Compiler::*CompilerAction)();

struct GrammarRule{
    GrammarFn prefix;
    GrammarFn infix;
    Precedence precedence;
};

struct Loop {
    int start;
    std::vector<int> breaks;
    Loop(int start) : start(start) {}
};

class Compiler {
public:
    pkpy::unique_ptr<Parser> parser;
    std::stack<_Code> codes;
    std::stack<Loop> loops;
    bool isCompilingClass = false;
    int lexingCnt = 0;
    VM* vm;

    emhash8::HashMap<_TokenType, GrammarRule> rules;

    _Code getCode() {
        return codes.top();
    }

    CompileMode mode() {
        return parser->src->mode;
    }

    Loop& getLoop() {
        return loops.top();
    }

    Compiler(VM* vm, const char* source, _Str filename, CompileMode mode){
        this->vm = vm;
        this->parser = pkpy::make_unique<Parser>(
            pkpy::make_shared<SourceMetadata>(source, filename, mode)
        );

// http://journal.stuffwithstuff.com/2011/03/19/pratt-parsers-expression-parsing-made-easy/
#define METHOD(name) &Compiler::name
#define NO_INFIX nullptr, PREC_NONE
        for(_TokenType i=0; i<__TOKENS_LEN; i++) rules[i] = { nullptr, NO_INFIX };
        rules[TK(".")] =    { nullptr,               METHOD(exprAttrib),         PREC_ATTRIB };
        rules[TK("->")] =   { nullptr,               METHOD(exprAttribPtr),      PREC_ATTRIB };
        rules[TK("(")] =    { METHOD(exprGrouping),  METHOD(exprCall),           PREC_CALL };
        rules[TK("[")] =    { METHOD(exprList),      METHOD(exprSubscript),      PREC_SUBSCRIPT };
        rules[TK("{")] =    { METHOD(exprMap),       NO_INFIX };
        rules[TK("%")] =    { nullptr,               METHOD(exprBinaryOp),       PREC_FACTOR };
        rules[TK("+")] =    { nullptr,               METHOD(exprBinaryOp),       PREC_TERM };
        rules[TK("-")] =    { METHOD(exprUnaryOp),   METHOD(exprBinaryOp),       PREC_TERM };
        rules[TK("*")] =    { METHOD(exprUnaryOp),   METHOD(exprBinaryOp),       PREC_FACTOR };
        rules[TK("/")] =    { nullptr,               METHOD(exprBinaryOp),       PREC_FACTOR };
        rules[TK("//")] =   { nullptr,               METHOD(exprBinaryOp),       PREC_FACTOR };
        rules[TK("**")] =   { nullptr,               METHOD(exprBinaryOp),       PREC_EXPONENT };
        rules[TK(">")] =    { nullptr,               METHOD(exprBinaryOp),       PREC_COMPARISION };
        rules[TK("<")] =    { nullptr,               METHOD(exprBinaryOp),       PREC_COMPARISION };
        rules[TK("==")] =   { nullptr,               METHOD(exprBinaryOp),       PREC_EQUALITY };
        rules[TK("!=")] =   { nullptr,               METHOD(exprBinaryOp),       PREC_EQUALITY };
        rules[TK(">=")] =   { nullptr,               METHOD(exprBinaryOp),       PREC_COMPARISION };
        rules[TK("<=")] =   { nullptr,               METHOD(exprBinaryOp),       PREC_COMPARISION };
        rules[TK("in")] =   { nullptr,               METHOD(exprBinaryOp),       PREC_TEST };
        rules[TK("is")] =   { nullptr,               METHOD(exprBinaryOp),       PREC_TEST };
        rules[TK("not in")] =   { nullptr,               METHOD(exprBinaryOp),       PREC_TEST };
        rules[TK("is not")] =   { nullptr,               METHOD(exprBinaryOp),       PREC_TEST };
        rules[TK("and") ] =     { nullptr,               METHOD(exprAnd),            PREC_LOGICAL_AND };
        rules[TK("or")] =       { nullptr,               METHOD(exprOr),             PREC_LOGICAL_OR };
        rules[TK("not")] =      { METHOD(exprUnaryOp),   nullptr,                    PREC_UNARY };
        rules[TK("True")] =     { METHOD(exprValue),     NO_INFIX };
        rules[TK("False")] =    { METHOD(exprValue),     NO_INFIX };
        rules[TK("lambda")] =   { METHOD(exprLambda),    NO_INFIX };
        rules[TK("None")] =     { METHOD(exprValue),     NO_INFIX };
        rules[TK("...")] =      { METHOD(exprValue),     NO_INFIX };
        rules[TK("@id")] =      { METHOD(exprName),      NO_INFIX };
        rules[TK("@num")] =     { METHOD(exprLiteral),   NO_INFIX };
        rules[TK("@str")] =     { METHOD(exprLiteral),   NO_INFIX };
        rules[TK("@fstr")] =    { METHOD(exprFString),   NO_INFIX };
        rules[TK("?")] =        { nullptr,               METHOD(exprTernary),        PREC_TERNARY };
        rules[TK("=")] =        { nullptr,               METHOD(exprAssign),         PREC_ASSIGNMENT };
        rules[TK("+=")] =       { nullptr,               METHOD(exprAssign),         PREC_ASSIGNMENT };
        rules[TK("-=")] =       { nullptr,               METHOD(exprAssign),         PREC_ASSIGNMENT };
        rules[TK("*=")] =       { nullptr,               METHOD(exprAssign),         PREC_ASSIGNMENT };
        rules[TK("/=")] =       { nullptr,               METHOD(exprAssign),         PREC_ASSIGNMENT };
        rules[TK("//=")] =      { nullptr,               METHOD(exprAssign),         PREC_ASSIGNMENT };
        rules[TK("%=")] =       { nullptr,               METHOD(exprAssign),         PREC_ASSIGNMENT };
        rules[TK("&=")] =       { nullptr,               METHOD(exprAssign),         PREC_ASSIGNMENT };
        rules[TK("|=")] =       { nullptr,               METHOD(exprAssign),         PREC_ASSIGNMENT };
        rules[TK("^=")] =       { nullptr,               METHOD(exprAssign),         PREC_ASSIGNMENT };
        rules[TK(",")] =        { nullptr,               METHOD(exprComma),          PREC_COMMA };
        rules[TK("<<")] =       { nullptr,               METHOD(exprBinaryOp),       PREC_BITWISE_SHIFT };
        rules[TK(">>")] =       { nullptr,               METHOD(exprBinaryOp),       PREC_BITWISE_SHIFT };
        rules[TK("&")] =        { METHOD(exprUnaryOp),   METHOD(exprBinaryOp),       PREC_BITWISE_AND };
        rules[TK("|")] =        { nullptr,               METHOD(exprBinaryOp),       PREC_BITWISE_OR };
        rules[TK("^")] =        { nullptr,               METHOD(exprBinaryOp),       PREC_BITWISE_XOR };
#undef METHOD
#undef NO_INFIX

#define EXPR() parsePrecedence(PREC_TERNARY)             // no '=' and ',' just a simple expression
#define EXPR_TUPLE() parsePrecedence(PREC_COMMA)            // no '=', but ',' is allowed
#define EXPR_ANY() parsePrecedence(PREC_ASSIGNMENT)
    }

    _Str eatStringUntil(char quote) {
        std::vector<char> buff;
        while (true) {
            char c = parser->eatCharIncludeNewLine();
            if (c == quote) break;
            if (c == '\0' || c == '\n') syntaxError("EOL while scanning string literal");
            if (c == '\\') {
                switch (parser->eatCharIncludeNewLine()) {
                    case '"':  buff.push_back('"');  break;
                    case '\'': buff.push_back('\''); break;
                    case '\\': buff.push_back('\\'); break;
                    case 'n':  buff.push_back('\n'); break;
                    case 'r':  buff.push_back('\r'); break;
                    case 't':  buff.push_back('\t'); break;
                    case '\n': case '\r': break;
                    default: syntaxError("invalid escape character");
                }
            } else {
                buff.push_back(c);
            }
        }
        return _Str(buff.data(), buff.size());
    }

    void eatString(char quote, bool fstr) {
        _Str s = eatStringUntil(quote);
        if(fstr){
            parser->setNextToken(TK("@fstr"), vm->PyStr(s));
        }else{
            parser->setNextToken(TK("@str"), vm->PyStr(s));
        }
    }

    void eatNumber() {
        static const std::regex pattern("^(0x)?[0-9a-fA-F]+(\\.[0-9]+)?");
        std::smatch m;

        const char* i = parser->token_start;
        while(*i != '\n' && *i != '\0') i++;
        std::string s = std::string(parser->token_start, i);

        try{
            if (std::regex_search(s, m, pattern)) {
                // here is m.length()-1, since the first char is eaten by lexToken()
                for(int j=0; j<m.length()-1; j++) parser->eatChar();

                int base = 10;
                size_t size;
                if (m[1].matched) base = 16;
                if (m[2].matched) {
                    if(base == 16) syntaxError("hex literal should not contain a dot");
                    parser->setNextToken(TK("@num"), vm->PyFloat(std::stod(m[0], &size)));
                } else {
                    parser->setNextToken(TK("@num"), vm->PyInt(std::stoll(m[0], &size, base)));
                }
                if (size != m.length()) throw std::runtime_error("length mismatch");
            }
        }catch(std::exception& _){
            syntaxError("invalid number literal");
        } 
    }

    void lexToken(){
        lexingCnt++;
        _lexToken();
        lexingCnt--;
    }

    // Lex the next token and set it as the next token.
    void _lexToken() {
        parser->previous = parser->current;
        parser->current = parser->nextToken();

        //_Str _info = parser->current.info(); std::cout << _info << '[' << parser->current_line << ']' << std::endl;

        while (parser->peekChar() != '\0') {
            parser->token_start = parser->current_char;
            char c = parser->eatCharIncludeNewLine();
            switch (c) {
                case '\'': case '"': eatString(c, false); return;
                case '#': parser->skipLineComment(); break;
                case '{': parser->setNextToken(TK("{")); return;
                case '}': parser->setNextToken(TK("}")); return;
                case ',': parser->setNextToken(TK(",")); return;
                case ':': parser->setNextToken(TK(":")); return;
                case ';': parser->setNextToken(TK(";")); return;
                case '(': parser->setNextToken(TK("(")); return;
                case ')': parser->setNextToken(TK(")")); return;
                case '[': parser->setNextToken(TK("[")); return;
                case ']': parser->setNextToken(TK("]")); return;
                case '%': parser->setNextTwoCharToken('=', TK("%"), TK("%=")); return;
                case '&': parser->setNextTwoCharToken('=', TK("&"), TK("&=")); return;
                case '|': parser->setNextTwoCharToken('=', TK("|"), TK("|=")); return;
                case '^': parser->setNextTwoCharToken('=', TK("^"), TK("^=")); return;
                case '?': parser->setNextToken(TK("?")); return;
                case '.': {
                    if(parser->matchChar('.')) {
                        if(parser->matchChar('.')) {
                            parser->setNextToken(TK("..."));
                        } else {
                            syntaxError("invalid token '..'");
                        }
                    } else {
                        parser->setNextToken(TK("."));
                    }
                    return;
                }
                case '=': parser->setNextTwoCharToken('=', TK("="), TK("==")); return;
                case '+': parser->setNextTwoCharToken('=', TK("+"), TK("+=")); return;
                case '>': {
                    if(parser->matchChar('=')) parser->setNextToken(TK(">="));
                    else if(parser->matchChar('>')) parser->setNextToken(TK(">>"));
                    else parser->setNextToken(TK(">"));
                    return;
                }
                case '<': {
                    if(parser->matchChar('=')) parser->setNextToken(TK("<="));
                    else if(parser->matchChar('<')) parser->setNextToken(TK("<<"));
                    else parser->setNextToken(TK("<"));
                    return;
                }
                case '-': {
                    if(parser->matchChar('=')) parser->setNextToken(TK("-="));
                    else if(parser->matchChar('>')) parser->setNextToken(TK("->"));
                    else parser->setNextToken(TK("-"));
                    return;
                }
                case '!':
                    if(parser->matchChar('=')) parser->setNextToken(TK("!="));
                    else syntaxError("expected '=' after '!'");
                    break;
                case '*':
                    if (parser->matchChar('*')) {
                        parser->setNextToken(TK("**"));  // '**'
                    } else {
                        parser->setNextTwoCharToken('=', TK("*"), TK("*="));
                    }
                    return;
                case '/':
                    if(parser->matchChar('/')) {
                        parser->setNextTwoCharToken('=', TK("//"), TK("//="));
                    } else {
                        parser->setNextTwoCharToken('=', TK("/"), TK("/="));
                    }
                    return;
                case '\r': break;       // just ignore '\r'
                case ' ': case '\t': parser->eatSpaces(); break;
                case '\n': {
                    parser->setNextToken(TK("@eol"));
                    if(!parser->eatIndentation()) indentationError("unindent does not match any outer indentation level");
                    return;
                }
                default: {
                    if(c == 'f'){
                        if(parser->matchChar('\'')) {eatString('\'', true); return;}
                        if(parser->matchChar('"')) {eatString('"', true); return;}
                    }
                    if (c >= '0' && c <= '9') {
                        eatNumber();
                        return;
                    }
                    
                    switch (parser->eatName())
                    {
                        case 0: break;
                        case 1: syntaxError("invalid char: " + std::string(1, c));
                        case 2: syntaxError("invalid utf8 sequence: " + std::string(1, c));
                        case 3: syntaxError("@id contains invalid char"); break;
                        case 4: syntaxError("invalid JSON token"); break;
                        default: UNREACHABLE();
                    }
                    return;
                }
            }
        }

        parser->token_start = parser->current_char;
        parser->setNextToken(TK("@eof"));
    }

    inline _TokenType peek() {
        return parser->current.type;
    }

    // not sure this will work
    _TokenType peekNext() {
        if(parser->nexts.empty()) return TK("@eof");
        return parser->nexts.front().type;
    }

    bool match(_TokenType expected) {
        if (peek() != expected) return false;
        lexToken();
        return true;
    }

    void consume(_TokenType expected) {
        if (!match(expected)){
            _StrStream ss;
            ss << "expected '" << TK_STR(expected) << "', but got '" << TK_STR(peek()) << "'";
            syntaxError(ss.str());
        }
    }

    bool matchNewLines(bool repl_throw=false) {
        bool consumed = false;
        if (peek() == TK("@eol")) {
            while (peek() == TK("@eol")) lexToken();
            consumed = true;
        }
        if (repl_throw && peek() == TK("@eof")){
            throw NeedMoreLines(isCompilingClass);
        }
        return consumed;
    }

    bool matchEndStatement() {
        if (match(TK(";"))) {
            matchNewLines();
            return true;
        }
        if (matchNewLines() || peek() == TK("@eof"))
            return true;
        if (peek() == TK("@dedent")) return true;
        return false;
    }

    void consumeEndStatement() {
        if (!matchEndStatement()) syntaxError("expected statement end");
    }

    void exprLiteral() {
        PyVar value = parser->previous.value;
        int index = getCode()->addConst(value);
        emitCode(OP_LOAD_CONST, index);
    }

    void exprFString() {
        static const std::regex pattern(R"(\{(.*?)\})");
        PyVar value = parser->previous.value;
        std::string s = vm->PyStr_AS_C(value).str();
        std::sregex_iterator begin(s.begin(), s.end(), pattern);
        std::sregex_iterator end;
        int size = 0;
        int i = 0;
        for(auto it = begin; it != end; it++) {
            std::smatch m = *it;
            if (i < m.position()) {
                std::string literal = s.substr(i, m.position() - i);
                emitCode(OP_LOAD_CONST, getCode()->addConst(vm->PyStr(literal)));
                size++;
            }
            emitCode(OP_LOAD_EVAL_FN);
            emitCode(OP_LOAD_CONST, getCode()->addConst(vm->PyStr(m[1].str())));
            emitCode(OP_CALL, 1);
            size++;
            i = (int)(m.position() + m.length());
        }
        if (i < s.size()) {
            std::string literal = s.substr(i, s.size() - i);
            emitCode(OP_LOAD_CONST, getCode()->addConst(vm->PyStr(literal)));
            size++;
        }
        emitCode(OP_BUILD_STRING, size);
    }

    void exprLambda() {
        _Func func = pkpy::make_shared<Function>();
        func->name = "<lambda>";
        if(!match(TK(":"))){
            __compileFunctionArgs(func, false);
            consume(TK(":"));
        }
        func->code = pkpy::make_shared<CodeObject>(parser->src, func->name);
        this->codes.push(func->code);
        EXPR_TUPLE();
        emitCode(OP_RETURN_VALUE);
        this->codes.pop();
        emitCode(OP_LOAD_LAMBDA, getCode()->addConst(vm->PyFunction(func)));
    }

    void exprAssign() {
        _TokenType op = parser->previous.type;
        if(op == TK("=")) {     // a = (expr)
            EXPR_TUPLE();
            emitCode(OP_STORE_REF);
        }else{                  // a += (expr) -> a = a + (expr)
            // TODO: optimization is needed for inplace operators
            emitCode(OP_DUP_TOP);
            EXPR();
            switch (op) {
                case TK("+="):      emitCode(OP_BINARY_OP, 0);  break;
                case TK("-="):      emitCode(OP_BINARY_OP, 1);  break;
                case TK("*="):      emitCode(OP_BINARY_OP, 2);  break;
                case TK("/="):      emitCode(OP_BINARY_OP, 3);  break;
                case TK("//="):     emitCode(OP_BINARY_OP, 4);  break;

                case TK("%="):      emitCode(OP_BINARY_OP, 5);  break;
                case TK("&="):      emitCode(OP_BITWISE_OP, 2);  break;
                case TK("|="):      emitCode(OP_BITWISE_OP, 3);  break;
                case TK("^="):      emitCode(OP_BITWISE_OP, 4);  break;
                default: UNREACHABLE();
            }
            emitCode(OP_STORE_REF);
        }
    }

    void exprComma() {
        int size = 1;       // an expr is in the stack now
        do {
            EXPR();         // NOTE: "1," will fail, "1,2" will be ok
            size++;
        } while(match(TK(",")));
        emitCode(OP_BUILD_SMART_TUPLE, size);
    }

    void exprOr() {
        int patch = emitCode(OP_JUMP_IF_TRUE_OR_POP);
        parsePrecedence(PREC_LOGICAL_OR);
        patchJump(patch);
    }

    void exprAnd() {
        int patch = emitCode(OP_JUMP_IF_FALSE_OR_POP);
        parsePrecedence(PREC_LOGICAL_AND);
        patchJump(patch);
    }

    void exprTernary() {
        int patch = emitCode(OP_POP_JUMP_IF_FALSE);
        EXPR();         // if true
        int patch2 = emitCode(OP_JUMP_ABSOLUTE);
        consume(TK(":"));
        patchJump(patch);
        EXPR();         // if false
        patchJump(patch2);
    }

    void exprBinaryOp() {
        _TokenType op = parser->previous.type;
        parsePrecedence((Precedence)(rules[op].precedence + 1));

        switch (op) {
            case TK("+"):   emitCode(OP_BINARY_OP, 0);  break;
            case TK("-"):   emitCode(OP_BINARY_OP, 1);  break;
            case TK("*"):   emitCode(OP_BINARY_OP, 2);  break;
            case TK("/"):   emitCode(OP_BINARY_OP, 3);  break;
            case TK("//"):  emitCode(OP_BINARY_OP, 4);  break;
            case TK("%"):   emitCode(OP_BINARY_OP, 5);  break;
            case TK("**"):  emitCode(OP_BINARY_OP, 6);  break;

            case TK("<"):   emitCode(OP_COMPARE_OP, 0);    break;
            case TK("<="):  emitCode(OP_COMPARE_OP, 1);    break;
            case TK("=="):  emitCode(OP_COMPARE_OP, 2);    break;
            case TK("!="):  emitCode(OP_COMPARE_OP, 3);    break;
            case TK(">"):   emitCode(OP_COMPARE_OP, 4);    break;
            case TK(">="):  emitCode(OP_COMPARE_OP, 5);    break;
            case TK("in"):      emitCode(OP_CONTAINS_OP, 0);   break;
            case TK("not in"):  emitCode(OP_CONTAINS_OP, 1);   break;
            case TK("is"):      emitCode(OP_IS_OP, 0);         break;
            case TK("is not"):  emitCode(OP_IS_OP, 1);         break;

            case TK("<<"):  emitCode(OP_BITWISE_OP, 0);    break;
            case TK(">>"):  emitCode(OP_BITWISE_OP, 1);    break;
            case TK("&"):   emitCode(OP_BITWISE_OP, 2);    break;
            case TK("|"):   emitCode(OP_BITWISE_OP, 3);    break;
            case TK("^"):   emitCode(OP_BITWISE_OP, 4);    break;
            default: UNREACHABLE();
        }
    }

    void exprUnaryOp() {
        _TokenType op = parser->previous.type;
        matchNewLines();
        parsePrecedence((Precedence)(PREC_UNARY + 1));

        switch (op) {
            case TK("-"):     emitCode(OP_UNARY_NEGATIVE); break;
            case TK("not"):   emitCode(OP_UNARY_NOT);      break;
            case TK("&"):     emitCode(OP_UNARY_REF);      break;
            case TK("*"):     emitCode(OP_UNARY_DEREF);    break;
            default: UNREACHABLE();
        }
    }

    void exprGrouping() {
        matchNewLines(mode()==SINGLE_MODE);
        EXPR_TUPLE();
        matchNewLines(mode()==SINGLE_MODE);
        consume(TK(")"));
    }

    void exprList() {
        int _patch = emitCode(OP_NO_OP);
        int _body_start = getCode()->co_code.size();
        int ARGC = 0;
        do {
            matchNewLines(mode()==SINGLE_MODE);
            if (peek() == TK("]")) break;
            EXPR(); ARGC++;
            matchNewLines(mode()==SINGLE_MODE);
            if(ARGC == 1 && match(TK("for"))) goto __LISTCOMP;
        } while (match(TK(",")));
        matchNewLines(mode()==SINGLE_MODE);
        consume(TK("]"));
        emitCode(OP_BUILD_LIST, ARGC);
        return;

__LISTCOMP:
        int _body_end = getCode()->co_code.size();
        getCode()->co_code[_patch].op = OP_JUMP_ABSOLUTE;
        getCode()->co_code[_patch].arg = _body_end;
        emitCode(OP_BUILD_LIST, 0);
        EXPR_FOR_VARS();consume(TK("in"));EXPR_TUPLE();
        matchNewLines(mode()==SINGLE_MODE);
        
        int _skipPatch = emitCode(OP_JUMP_ABSOLUTE);
        int _cond_start = getCode()->co_code.size();
        if(match(TK("if"))) EXPR_TUPLE();
        int _cond_end = getCode()->co_code.size();
        patchJump(_skipPatch);

        emitCode(OP_GET_ITER);
        Loop& loop = enterLoop();
        int patch = emitCode(OP_FOR_ITER);

        if(_cond_end != _cond_start) {      // there is an if condition
            getCode()->__moveToEnd(_cond_start, _cond_end);
            int ifpatch = emitCode(OP_POP_JUMP_IF_FALSE);
            getCode()->__moveToEnd(_body_start, _body_end);
            emitCode(OP_LIST_APPEND);
            patchJump(ifpatch);
        }else{
            getCode()->__moveToEnd(_body_start, _body_end);
            emitCode(OP_LIST_APPEND);
        }

        emitCode(OP_JUMP_ABSOLUTE, loop.start); keepOpcodeLine();
        patchJump(patch);
        exitLoop();
        matchNewLines(mode()==SINGLE_MODE);
        consume(TK("]"));
    }

    void exprMap() {
        bool parsing_dict = false;
        int size = 0;
        do {
            matchNewLines(mode()==SINGLE_MODE);
            if (peek() == TK("}")) break;
            EXPR();
            if(peek() == TK(":")) parsing_dict = true;
            if(parsing_dict){
                consume(TK(":"));
                EXPR();
            }
            size++;
            matchNewLines(mode()==SINGLE_MODE);
        } while (match(TK(",")));
        matchNewLines();
        consume(TK("}"));

        if(size == 0 || parsing_dict) emitCode(OP_BUILD_MAP, size);
        else emitCode(OP_BUILD_SET, size);
    }

    void exprCall() {
        int ARGC = 0;
        int KWARGC = 0;
        do {
            matchNewLines(mode()==SINGLE_MODE);
            if (peek() == TK(")")) break;
            if(peek() == TK("@id") && peekNext() == TK("=")) {
                consume(TK("@id"));
                const _Str& key = parser->previous.str();
                emitCode(OP_LOAD_CONST, getCode()->addConst(vm->PyStr(key)));
                consume(TK("="));
                EXPR();
                KWARGC++;
            } else{
                if(KWARGC > 0) syntaxError("positional argument follows keyword argument");
                EXPR();
                ARGC++;
            }
            matchNewLines(mode()==SINGLE_MODE);
        } while (match(TK(",")));
        consume(TK(")"));
        emitCode(OP_CALL, (KWARGC << 16) | ARGC);
    }

    void exprName() {
        Token tkname = parser->previous;
        int index = getCode()->addName(
            tkname.str(),
            codes.size()>1 ? NAME_LOCAL : NAME_GLOBAL
        );
        emitCode(OP_LOAD_NAME_REF, index);
    }

    void exprAttrib() {
        consume(TK("@id"));
        const _Str& name = parser->previous.str();
        int index = getCode()->addName(name, NAME_ATTR);
        emitCode(OP_BUILD_ATTR_REF, index);
    }

    void exprAttribPtr(){
        consume(TK("@id"));
        const _Str& name = parser->previous.str();
        int index = getCode()->addName(name, NAME_ATTR);
        emitCode(OP_BUILD_ATTR_REF_PTR, index);
    }

    // [:], [:b]
    // [a], [a:], [a:b]
    void exprSubscript() {
        if(match(TK(":"))){
            emitCode(OP_LOAD_NONE);
            if(match(TK("]"))){
                emitCode(OP_LOAD_NONE);
            }else{
                EXPR_TUPLE();
                consume(TK("]"));
            }
            emitCode(OP_BUILD_SLICE);
        }else{
            EXPR_TUPLE();
            if(match(TK(":"))){
                if(match(TK("]"))){
                    emitCode(OP_LOAD_NONE);
                }else{
                    EXPR_TUPLE();
                    consume(TK("]"));
                }
                emitCode(OP_BUILD_SLICE);
            }else{
                consume(TK("]"));
            }
        }

        emitCode(OP_BUILD_INDEX_REF);
    }

    void exprValue() {
        _TokenType op = parser->previous.type;
        switch (op) {
            case TK("None"):    emitCode(OP_LOAD_NONE);  break;
            case TK("True"):    emitCode(OP_LOAD_TRUE);  break;
            case TK("False"):   emitCode(OP_LOAD_FALSE); break;
            case TK("..."):     emitCode(OP_LOAD_ELLIPSIS); break;
            default: UNREACHABLE();
        }
    }

    void keepOpcodeLine(){
        int i = getCode()->co_code.size() - 1;
        getCode()->co_code[i].line = getCode()->co_code[i-1].line;
    }

    int emitCode(Opcode opcode, int arg=-1) {
        int line = parser->previous.line;
        getCode()->co_code.push_back(
            ByteCode{(uint8_t)opcode, arg, (uint16_t)line}
        );
        return getCode()->co_code.size() - 1;
    }

    inline void patchJump(int addr_index) {
        int target = getCode()->co_code.size();
        getCode()->co_code[addr_index].arg = target;
    }

    void compileBlockBody(){
        __compileBlockBody(&Compiler::compileStatement);
    }
    
    void __compileBlockBody(CompilerAction action) {
        consume(TK(":"));
        if(!matchNewLines(mode()==SINGLE_MODE)){
            syntaxError("expected a new line after ':'");
        }
        consume(TK("@indent"));
        while (peek() != TK("@dedent")) {
            matchNewLines();
            (this->*action)();
            matchNewLines();
        }
        consume(TK("@dedent"));
    }

    Token compileImportPath() {
        consume(TK("@id"));
        Token tkmodule = parser->previous;
        int index = getCode()->addName(tkmodule.str(), NAME_GLOBAL);
        emitCode(OP_IMPORT_NAME, index);
        return tkmodule;
    }

    // import a as b
    void compileRegularImport() {
        do {
            Token tkmodule = compileImportPath();
            if (match(TK("as"))) {
                consume(TK("@id"));
                tkmodule = parser->previous;
            }
            int index = getCode()->addName(tkmodule.str(), NAME_GLOBAL);
            emitCode(OP_STORE_NAME_REF, index);
        } while (match(TK(",")));
        consumeEndStatement();
    }

    // from a import b as c, d as e
    void compileFromImport() {
        Token tkmodule = compileImportPath();
        consume(TK("import"));
        do {
            emitCode(OP_DUP_TOP);
            consume(TK("@id"));
            Token tkname = parser->previous;
            int index = getCode()->addName(tkname.str(), NAME_GLOBAL);
            emitCode(OP_BUILD_ATTR_REF, index);
            if (match(TK("as"))) {
                consume(TK("@id"));
                tkname = parser->previous;
            }
            index = getCode()->addName(tkname.str(), NAME_GLOBAL);
            emitCode(OP_STORE_NAME_REF, index);
        } while (match(TK(",")));
        emitCode(OP_POP_TOP);
        consumeEndStatement();
    }

    void parsePrecedence(Precedence precedence) {
        lexToken();
        GrammarFn prefix = rules[parser->previous.type].prefix;
        if (prefix == nullptr) syntaxError(_Str("expected an expression, but got ") + TK_STR(parser->previous.type));
        (this->*prefix)();
        while (rules[peek()].precedence >= precedence) {
            lexToken();
            _TokenType op = parser->previous.type;
            GrammarFn infix = rules[op].infix;
            if(infix == nullptr) throw std::runtime_error("(infix == nullptr) is true");
            (this->*infix)();
        }
    }

    void compileIfStatement() {
        matchNewLines();
        EXPR_TUPLE();

        int ifpatch = emitCode(OP_POP_JUMP_IF_FALSE);
        compileBlockBody();

        if (match(TK("elif"))) {
            int exit_jump = emitCode(OP_JUMP_ABSOLUTE);
            patchJump(ifpatch);
            compileIfStatement();
            patchJump(exit_jump);
        } else if (match(TK("else"))) {
            int exit_jump = emitCode(OP_JUMP_ABSOLUTE);
            patchJump(ifpatch);
            compileBlockBody();
            patchJump(exit_jump);
        } else {
            patchJump(ifpatch);
        }
    }

    Loop& enterLoop(){
        Loop lp((int)getCode()->co_code.size());
        loops.push(lp);
        return loops.top();
    }

    void exitLoop(){
        Loop& lp = loops.top();
        for(int addr : lp.breaks) patchJump(addr);
        loops.pop();
    }

    void compileWhileLoop() {
        Loop& loop = enterLoop();
        EXPR_TUPLE();
        int patch = emitCode(OP_POP_JUMP_IF_FALSE);
        compileBlockBody();
        emitCode(OP_JUMP_ABSOLUTE, loop.start); keepOpcodeLine();
        patchJump(patch);
        exitLoop();
    }

    void EXPR_FOR_VARS(){
        int size = 0;
        do {
            consume(TK("@id"));
            exprName(); size++;
        } while (match(TK(",")));
        if(size > 1) emitCode(OP_BUILD_SMART_TUPLE, size);
    }

    void compileForLoop() {
        EXPR_FOR_VARS();consume(TK("in"));EXPR_TUPLE();
        emitCode(OP_GET_ITER);
        Loop& loop = enterLoop();
        int patch = emitCode(OP_FOR_ITER);
        compileBlockBody();
        emitCode(OP_JUMP_ABSOLUTE, loop.start); keepOpcodeLine();
        patchJump(patch);
        exitLoop();
    }

    void compileStatement() {
        if (match(TK("break"))) {
            if (loops.empty()) syntaxError("'break' outside loop");
            consumeEndStatement();
            int patch = emitCode(OP_SAFE_JUMP_ABSOLUTE);
            getLoop().breaks.push_back(patch);
        } else if (match(TK("continue"))) {
            if (loops.empty()) syntaxError("'continue' not properly in loop");
            consumeEndStatement();
            emitCode(OP_JUMP_ABSOLUTE, getLoop().start);
        } else if (match(TK("return"))) {
            if (codes.size() == 1)
                syntaxError("'return' outside function");
            if(matchEndStatement()){
                emitCode(OP_LOAD_NONE);
            }else{
                EXPR_TUPLE();
                consumeEndStatement();
            }
            emitCode(OP_RETURN_VALUE);
        } else if (match(TK("if"))) {
            compileIfStatement();
        } else if (match(TK("while"))) {
            compileWhileLoop();
        } else if (match(TK("for"))) {
            compileForLoop();
        } else if(match(TK("assert"))){
            EXPR();
            emitCode(OP_ASSERT);
            consumeEndStatement();
        } else if(match(TK("with"))){
            EXPR();
            consume(TK("as"));
            consume(TK("@id"));
            Token tkname = parser->previous;
            int index = getCode()->addName(
                tkname.str(),
                codes.size()>1 ? NAME_LOCAL : NAME_GLOBAL
            );
            emitCode(OP_STORE_NAME_REF, index);
            emitCode(OP_LOAD_NAME_REF, index);
            emitCode(OP_WITH_ENTER);
            compileBlockBody();
            emitCode(OP_LOAD_NAME_REF, index);
            emitCode(OP_WITH_EXIT);
        } else if(match(TK("label"))){
            if(mode() != EXEC_MODE) syntaxError("'label' is only available in EXEC_MODE");
            consume(TK(".")); consume(TK("@id"));
            getCode()->addLabel(parser->previous.str());
            consumeEndStatement();
        } else if(match(TK("goto"))){
            // https://entrian.com/goto/
            if(mode() != EXEC_MODE) syntaxError("'goto' is only available in EXEC_MODE");
            consume(TK(".")); consume(TK("@id"));
            emitCode(OP_LOAD_CONST, getCode()->addConst(vm->PyStr(parser->previous.str())));
            emitCode(OP_GOTO);
            consumeEndStatement();
        } else if(match(TK("raise"))){
            consume(TK("@id"));         // dummy exception type
            emitCode(OP_LOAD_CONST, getCode()->addConst(vm->PyStr(parser->previous.str())));
            consume(TK("("));EXPR();consume(TK(")"));
            emitCode(OP_RAISE_ERROR);
            consumeEndStatement();
        } else if(match(TK("del"))){
            EXPR();
            emitCode(OP_DELETE_REF);
            consumeEndStatement();
        } else if(match(TK("global"))){
            consume(TK("@id"));
            getCode()->co_global_names.push_back(parser->previous.str());
            consumeEndStatement();
        } else if(match(TK("pass"))){
            consumeEndStatement();
        } else {
            EXPR_ANY();
            consumeEndStatement();

            // If last op is not an assignment, pop the result.
            uint8_t lastOp = getCode()->co_code.back().op;
            if( lastOp != OP_STORE_NAME_REF && lastOp != OP_STORE_REF){
                if(mode()==SINGLE_MODE && parser->indents.top() == 0){
                    emitCode(OP_PRINT_EXPR);
                }
                emitCode(OP_POP_TOP);
            }
        }
    }

    void compileClass(){
        consume(TK("@id"));
        int clsNameIdx = getCode()->addName(parser->previous.str(), NAME_GLOBAL);
        int superClsNameIdx = -1;
        if(match(TK("("))){
            consume(TK("@id"));
            superClsNameIdx = getCode()->addName(parser->previous.str(), NAME_GLOBAL);
            consume(TK(")"));
        }
        emitCode(OP_LOAD_NONE);
        isCompilingClass = true;
        __compileBlockBody(&Compiler::compileFunction);
        isCompilingClass = false;

        if(superClsNameIdx == -1) emitCode(OP_LOAD_NONE);
        else emitCode(OP_LOAD_NAME_REF, superClsNameIdx);
        emitCode(OP_BUILD_CLASS, clsNameIdx);
    }

    void __compileFunctionArgs(_Func func, bool enableTypeHints){
        int state = 0;      // 0 for args, 1 for *args, 2 for k=v, 3 for **kwargs
        do {
            if(state == 3) syntaxError("**kwargs should be the last argument");
            matchNewLines();
            if(match(TK("*"))){
                if(state < 1) state = 1;
                else syntaxError("*args should be placed before **kwargs");
            }
            else if(match(TK("**"))){
                state = 3;
            }

            consume(TK("@id"));
            const _Str& name = parser->previous.str();
            if(func->hasName(name)) syntaxError("duplicate argument name");

            // eat type hints
            if(enableTypeHints && match(TK(":"))) consume(TK("@id"));

            if(state == 0 && peek() == TK("=")) state = 2;

            switch (state)
            {
                case 0: func->args.push_back(name); break;
                case 1: func->starredArg = name; state+=1; break;
                case 2: {
                    consume(TK("="));
                    PyVarOrNull value = readLiteral();
                    if(value == nullptr){
                        syntaxError(_Str("expect a literal, not ") + TK_STR(parser->current.type));
                    }
                    func->kwArgs[name] = value;
                    func->kwArgsOrder.push_back(name);
                } break;
                case 3: syntaxError("**kwargs is not supported yet"); break;
            }
        } while (match(TK(",")));
    }

    void compileFunction(){
        if(isCompilingClass){
            if(match(TK("pass"))) return;
            consume(TK("def"));
        }
        _Func func = pkpy::make_shared<Function>();
        consume(TK("@id"));
        func->name = parser->previous.str();

        if (match(TK("(")) && !match(TK(")"))) {
            __compileFunctionArgs(func, true);
            consume(TK(")"));
        }

        // eat type hints
        if(match(TK("->"))) consume(TK("@id"));

        func->code = pkpy::make_shared<CodeObject>(parser->src, func->name);
        this->codes.push(func->code);
        compileBlockBody();
        this->codes.pop();
        emitCode(OP_LOAD_CONST, getCode()->addConst(vm->PyFunction(func)));
        if(!isCompilingClass) emitCode(OP_STORE_FUNCTION);
    }

    PyVarOrNull readLiteral(){
        if(match(TK("-"))){
            consume(TK("@num"));
            PyVar val = parser->previous.value;
            return vm->numNegated(val);
        }
        if(match(TK("@num"))) return parser->previous.value;
        if(match(TK("@str"))) return parser->previous.value;
        if(match(TK("True"))) return vm->PyBool(true);
        if(match(TK("False"))) return vm->PyBool(false);
        if(match(TK("None"))) return vm->None;
        if(match(TK("..."))) return vm->Ellipsis;
        return nullptr;
    }

    void compileTopLevelStatement() {
        if (match(TK("class"))) {
            compileClass();
        } else if (match(TK("def"))) {
            compileFunction();
        } else if (match(TK("import"))) {
            compileRegularImport();
        } else if (match(TK("from"))) {
            compileFromImport();
        } else {
            compileStatement();
        }
    }

    _Code __fillCode(){
        _Code code = pkpy::make_shared<CodeObject>(parser->src, _Str("<module>"));
        codes.push(code);

        // Lex initial tokens. current <-- next.
        lexToken();
        lexToken();
        matchNewLines();

        if(mode()==EVAL_MODE) {
            EXPR_TUPLE();
            consume(TK("@eof"));
            return code;
        }else if(mode()==JSON_MODE){
            PyVarOrNull value = readLiteral();
            if(value != nullptr) emitCode(OP_LOAD_CONST, code->addConst(value));
            else if(match(TK("{"))) exprMap();
            else if(match(TK("["))) exprList();
            else syntaxError("expect a JSON object or array");
            consume(TK("@eof"));
            return code;
        }

        while (!match(TK("@eof"))) {
            compileTopLevelStatement();
            matchNewLines();
        }
        return code;
    }

    /***** Error Reporter *****/
    _Str getLineSnapshot(){
        int lineno = parser->current.line;
        const char* cursor = parser->current.start;
        // if error occurs in lexing, lineno should be `parser->current_line`
        if(lexingCnt > 0){
            lineno = parser->current_line;
            cursor = parser->current_char;
        }
        if(parser->peekChar() == '\n') lineno--;
        return parser->src->snapshot(lineno, cursor);
    }

    void syntaxError(_Str msg){ throw CompileError("SyntaxError", msg, getLineSnapshot()); }
    void indentationError(_Str msg){ throw CompileError("IndentationError", msg, getLineSnapshot()); }
    void unexpectedError(_Str msg){ throw CompileError("UnexpectedError", msg, getLineSnapshot()); }
};

_Code compile(VM* vm, const char* source, _Str filename, CompileMode mode=EXEC_MODE, bool noThrow=true) {
    Compiler compiler(vm, source, filename, mode);
    if(!noThrow) return compiler.__fillCode();
    try{
        return compiler.__fillCode();
    }catch(_Error& e){
        (*vm->_stderr) << e.what() << '\n';
    }catch(std::exception& e){
        auto ce = CompileError("UnexpectedError", e.what(), compiler.getLineSnapshot());
        (*vm->_stderr) << ce.what() << '\n';
    }
    return nullptr;
}


enum InputResult {
    NEED_MORE_LINES = 0,
    EXEC_DONE = 1,
    EXEC_SKIPPED = 2,
};

class REPL {
protected:
    int need_more_lines = 0;
    std::string buffer;
    VM* vm;
    bool exited = false;

    void _exit(){
        exited = true;
        exit(0);
    }
public:
    REPL(VM* vm) : vm(vm){
        (*vm->_stdout) << ("pocketpy " PK_VERSION " (" __DATE__ ", " __TIME__ ")\n");
        (*vm->_stdout) << ("https://github.com/blueloveTH/pocketpy" "\n");
        (*vm->_stdout) << ("Type \"exit()\" to exit." "\n");
    }

    bool is_need_more_lines() const {
        return need_more_lines;
    }

    InputResult input(std::string line){
        if(exited) return EXEC_SKIPPED;
        if(need_more_lines){
            buffer += line;
            buffer += '\n';
            int n = buffer.size();
            if(n>=need_more_lines){
                for(int i=buffer.size()-need_more_lines; i<buffer.size(); i++){
                    if(buffer[i] != '\n') goto __NOT_ENOUGH_LINES;
                }
                need_more_lines = 0;
                line = buffer;
                buffer.clear();
            }else{
__NOT_ENOUGH_LINES:
                return NEED_MORE_LINES;
            }
        }else{
            if(line == "exit()") _exit();
            if(line.empty()) return EXEC_SKIPPED;
        }

        try{
            _Code code = compile(vm, line.c_str(), "<stdin>", SINGLE_MODE);
            if(code == nullptr) return EXEC_SKIPPED;
            vm->execAsync(code);
            return EXEC_DONE;
        }catch(NeedMoreLines& ne){
            buffer += line;
            buffer += '\n';
            need_more_lines = ne.isClassDef ? 3 : 2;
            return NEED_MORE_LINES;
        }
    }
};


#define BIND_NUM_ARITH_OPT(name, op)                                                                    \
    _vm->bindMethodMulti({"int","float"}, #name, [](VM* vm, const pkpy::ArgList& args){                 \
        if(!vm->isIntOrFloat(args[0], args[1]))                                                         \
            vm->typeError("unsupported operand type(s) for " #op );                                     \
        if(args._index(0)->isType(vm->_tp_int) && args._index(1)->isType(vm->_tp_int)){                 \
            return vm->PyInt(vm->PyInt_AS_C(args._index(0)) op vm->PyInt_AS_C(args._index(1)));         \
        }else{                                                                                          \
            return vm->PyFloat(vm->numToFloat(args._index(0)) op vm->numToFloat(args._index(1)));       \
        }                                                                                               \
    });

#define BIND_NUM_LOGICAL_OPT(name, op, is_eq)                                                           \
    _vm->bindMethodMulti({"int","float"}, #name, [](VM* vm, const pkpy::ArgList& args){                 \
        if(!vm->isIntOrFloat(args[0], args[1])){                                                        \
            if constexpr(is_eq) return vm->PyBool(args[0] == args[1]);                                  \
            vm->typeError("unsupported operand type(s) for " #op );                                     \
        }                                                                                               \
        return vm->PyBool(vm->numToFloat(args._index(0)) op vm->numToFloat(args._index(1)));            \
    });
    

void __initializeBuiltinFunctions(VM* _vm) {
    BIND_NUM_ARITH_OPT(__add__, +)
    BIND_NUM_ARITH_OPT(__sub__, -)
    BIND_NUM_ARITH_OPT(__mul__, *)

    BIND_NUM_LOGICAL_OPT(__lt__, <, false)
    BIND_NUM_LOGICAL_OPT(__le__, <=, false)
    BIND_NUM_LOGICAL_OPT(__gt__, >, false)
    BIND_NUM_LOGICAL_OPT(__ge__, >=, false)
    BIND_NUM_LOGICAL_OPT(__eq__, ==, true)

#undef BIND_NUM_ARITH_OPT
#undef BIND_NUM_LOGICAL_OPT

    _vm->bindBuiltinFunc("__sys_stdout_write", [](VM* vm, const pkpy::ArgList& args) {
        vm->__checkArgSize(args, 1);
        (*vm->_stdout) << vm->PyStr_AS_C(args[0]);
        return vm->None;
    });

    _vm->bindBuiltinFunc("super", [](VM* vm, const pkpy::ArgList& args) {
        vm->__checkArgSize(args, 0);
        auto it = vm->topFrame()->f_locals.find("self"_c);
        if(it == vm->topFrame()->f_locals.end()) vm->typeError("super() can only be called in a class method");
        return vm->newObject(vm->_tp_super, it->second);
    });

    _vm->bindBuiltinFunc("eval", [](VM* vm, const pkpy::ArgList& args) {
        vm->__checkArgSize(args, 1);
        const _Str& expr = vm->PyStr_AS_C(args[0]);
        _Code code = compile(vm, expr.c_str(), "<eval>", EVAL_MODE, false);
        return vm->_exec(code, vm->topFrame()->_module, vm->topFrame()->copy_f_locals());
    });

    _vm->bindBuiltinFunc("isinstance", [](VM* vm, const pkpy::ArgList& args) {
        vm->__checkArgSize(args, 2);
        return vm->PyBool(vm->isInstance(args[0], args[1]));
    });

    _vm->bindBuiltinFunc("repr", [](VM* vm, const pkpy::ArgList& args) {
        vm->__checkArgSize(args, 1);
        return vm->asRepr(args[0]);
    });

    _vm->bindBuiltinFunc("hash", [](VM* vm, const pkpy::ArgList& args) {
        vm->__checkArgSize(args, 1);
        return vm->PyInt(vm->hash(args[0]));
    });

    _vm->bindBuiltinFunc("chr", [](VM* vm, const pkpy::ArgList& args) {
        vm->__checkArgSize(args, 1);
        _Int i = vm->PyInt_AS_C(args[0]);
        if (i < 0 || i > 128) vm->valueError("chr() arg not in range(128)");
        return vm->PyStr(std::string(1, (char)i));
    });

    _vm->bindBuiltinFunc("ord", [](VM* vm, const pkpy::ArgList& args) {
        vm->__checkArgSize(args, 1);
        _Str s = vm->PyStr_AS_C(args[0]);
        if (s.size() != 1) vm->typeError("ord() expected an ASCII character");
        return vm->PyInt((_Int)s[0]);
    });

    _vm->bindBuiltinFunc("globals", [](VM* vm, const pkpy::ArgList& args) {
        vm->__checkArgSize(args, 0);
        const auto& d = vm->topFrame()->f_globals();
        PyVar obj = vm->call(vm->builtins->attribs["dict"]);
        for (const auto& [k, v] : d) {
            vm->call(obj, __setitem__, pkpy::twoArgs(vm->PyStr(k), v));
        }
        return obj;
    });

    _vm->bindBuiltinFunc("locals", [](VM* vm, const pkpy::ArgList& args) {
        vm->__checkArgSize(args, 0);
        const auto& d = vm->topFrame()->f_locals;
        PyVar obj = vm->call(vm->builtins->attribs["dict"]);
        for (const auto& [k, v] : d) {
            vm->call(obj, __setitem__, pkpy::twoArgs(vm->PyStr(k), v));
        }
        return obj;
    });

    _vm->bindBuiltinFunc("dir", [](VM* vm, const pkpy::ArgList& args) {
        vm->__checkArgSize(args, 1);
        std::vector<_Str> names;
        for (auto& [k, _] : args[0]->attribs) names.push_back(k);
        for (auto& [k, _] : args[0]->_type->attribs) {
            if (k.str().find("__") == 0) continue;
            if (std::find(names.begin(), names.end(), k) == names.end()) names.push_back(k);
        }
        PyVarList ret;
        for (const auto& name : names) ret.push_back(vm->PyStr(name));
        std::sort(ret.begin(), ret.end(), [vm](const PyVar& a, const PyVar& b) {
            return vm->PyStr_AS_C(a) < vm->PyStr_AS_C(b);
        });
        return vm->PyList(ret);
    });

    _vm->bindMethod("object", "__repr__", [](VM* vm, const pkpy::ArgList& args) {
        PyVar _self = args[0];
        _Str s = "<" + UNION_TP_NAME(_self) + " object at " + std::to_string((uintptr_t)_self.get()) + ">";
        return vm->PyStr(s);
    });

    _vm->bindMethod("type", "__new__", [](VM* vm, const pkpy::ArgList& args) {
        vm->__checkArgSize(args, 1);
        return args[0]->_type;
    });

    _vm->bindMethod("range", "__new__", [](VM* vm, const pkpy::ArgList& args) {
        _Range r;
        switch (args.size()) {
            case 1: r.stop = vm->PyInt_AS_C(args[0]); break;
            case 2: r.start = vm->PyInt_AS_C(args[0]); r.stop = vm->PyInt_AS_C(args[1]); break;
            case 3: r.start = vm->PyInt_AS_C(args[0]); r.stop = vm->PyInt_AS_C(args[1]); r.step = vm->PyInt_AS_C(args[2]); break;
            default: vm->typeError("expected 1-3 arguments, but got " + std::to_string(args.size()));
        }
        return vm->PyRange(r);
    });

    _vm->bindMethod("range", "__iter__", [](VM* vm, const pkpy::ArgList& args) {
        vm->__checkType(args[0], vm->_tp_range);
        return vm->PyIter(
            pkpy::make_shared<BaseIterator, RangeIterator>(vm, args[0])
        );
    });

    _vm->bindMethod("NoneType", "__repr__", [](VM* vm, const pkpy::ArgList& args) {
        return vm->PyStr("None");
    });

    _vm->bindMethod("NoneType", "__json__", [](VM* vm, const pkpy::ArgList& args) {
        return vm->PyStr("null");
    });

    _vm->bindMethod("NoneType", "__eq__", [](VM* vm, const pkpy::ArgList& args) {
        return vm->PyBool(args[0] == args[1]);
    });

    _vm->bindMethodMulti({"int", "float"}, "__truediv__", [](VM* vm, const pkpy::ArgList& args) {
        if(!vm->isIntOrFloat(args[0], args[1]))
            vm->typeError("unsupported operand type(s) for " "/" );
        _Float rhs = vm->numToFloat(args[1]);
        if (rhs == 0) vm->zeroDivisionError();
        return vm->PyFloat(vm->numToFloat(args[0]) / rhs);
    });

    _vm->bindMethodMulti({"int", "float"}, "__pow__", [](VM* vm, const pkpy::ArgList& args) {
        if(!vm->isIntOrFloat(args[0], args[1]))
            vm->typeError("unsupported operand type(s) for " "**" );
        if(args[0]->isType(vm->_tp_int) && args[1]->isType(vm->_tp_int)){
            return vm->PyInt((_Int)round(pow(vm->PyInt_AS_C(args[0]), vm->PyInt_AS_C(args[1]))));
        }else{
            return vm->PyFloat((_Float)pow(vm->numToFloat(args[0]), vm->numToFloat(args[1])));
        }
    });

    /************ PyInt ************/
    _vm->bindMethod("int", "__new__", [](VM* vm, const pkpy::ArgList& args) {
        if(args.size() == 0) return vm->PyInt(0);
        vm->__checkArgSize(args, 1);
        if (args[0]->isType(vm->_tp_int)) return args[0];
        if (args[0]->isType(vm->_tp_float)) return vm->PyInt((_Int)vm->PyFloat_AS_C(args[0]));
        if (args[0]->isType(vm->_tp_bool)) return vm->PyInt(vm->PyBool_AS_C(args[0]) ? 1 : 0);
        if (args[0]->isType(vm->_tp_str)) {
            const _Str& s = vm->PyStr_AS_C(args[0]);
            try{
                size_t parsed = 0;
                _Int val = std::stoll(s.str(), &parsed, 10);
                if(parsed != s.str().size()) throw std::invalid_argument("");
                return vm->PyInt(val);
            }catch(std::invalid_argument&){
                vm->valueError("invalid literal for int(): '" + s + "'");
            }
        }
        vm->typeError("int() argument must be a int, float, bool or str");
        return vm->None;
    });

    _vm->bindMethod("int", "__floordiv__", [](VM* vm, const pkpy::ArgList& args) {
        if(!args[0]->isType(vm->_tp_int) || !args[1]->isType(vm->_tp_int))
            vm->typeError("unsupported operand type(s) for " "//" );
        _Int rhs = vm->PyInt_AS_C(args._index(1));
        if(rhs == 0) vm->zeroDivisionError();
        return vm->PyInt(vm->PyInt_AS_C(args._index(0)) / rhs);
    });

    _vm->bindMethod("int", "__mod__", [](VM* vm, const pkpy::ArgList& args) {
        if(!args[0]->isType(vm->_tp_int) || !args[1]->isType(vm->_tp_int))
            vm->typeError("unsupported operand type(s) for " "%" );
        _Int rhs = vm->PyInt_AS_C(args._index(1));
        if(rhs == 0) vm->zeroDivisionError();
        return vm->PyInt(vm->PyInt_AS_C(args._index(0)) % rhs);
    });

    _vm->bindMethod("int", "__repr__", [](VM* vm, const pkpy::ArgList& args) {
        return vm->PyStr(std::to_string(vm->PyInt_AS_C(args[0])));
    });

    _vm->bindMethod("int", "__json__", [](VM* vm, const pkpy::ArgList& args) {
        return vm->PyStr(std::to_string((int)vm->PyInt_AS_C(args[0])));
    });

#define __INT_BITWISE_OP(name,op) \
    _vm->bindMethod("int", #name, [](VM* vm, const pkpy::ArgList& args) {                       \
        if(!args[0]->isType(vm->_tp_int) || !args[1]->isType(vm->_tp_int))                      \
            vm->typeError("unsupported operand type(s) for " #op );                             \
        return vm->PyInt(vm->PyInt_AS_C(args._index(0)) op vm->PyInt_AS_C(args._index(1)));     \
    });

    __INT_BITWISE_OP(__lshift__, <<)
    __INT_BITWISE_OP(__rshift__, >>)
    __INT_BITWISE_OP(__and__, &)
    __INT_BITWISE_OP(__or__, |)
    __INT_BITWISE_OP(__xor__, ^)

#undef __INT_BITWISE_OP

    /************ PyFloat ************/
    _vm->bindMethod("float", "__new__", [](VM* vm, const pkpy::ArgList& args) {
        if(args.size() == 0) return vm->PyFloat(0.0);
        vm->__checkArgSize(args, 1);
        if (args[0]->isType(vm->_tp_int)) return vm->PyFloat((_Float)vm->PyInt_AS_C(args[0]));
        if (args[0]->isType(vm->_tp_float)) return args[0];
        if (args[0]->isType(vm->_tp_bool)) return vm->PyFloat(vm->PyBool_AS_C(args[0]) ? 1.0 : 0.0);
        if (args[0]->isType(vm->_tp_str)) {
            const _Str& s = vm->PyStr_AS_C(args[0]);
            if(s == "inf") return vm->PyFloat(INFINITY);
            if(s == "-inf") return vm->PyFloat(-INFINITY);
            try{
                _Float val = std::stod(s.str());
                return vm->PyFloat(val);
            }catch(std::invalid_argument&){
                vm->valueError("invalid literal for float(): '" + s + "'");
            }
        }
        vm->typeError("float() argument must be a int, float, bool or str");
        return vm->None;
    });

    _vm->bindMethod("float", "__repr__", [](VM* vm, const pkpy::ArgList& args) {
        _Float val = vm->PyFloat_AS_C(args[0]);
        if(std::isinf(val) || std::isnan(val)) return vm->PyStr(std::to_string(val));
        _StrStream ss;
        ss << std::setprecision(std::numeric_limits<_Float>::max_digits10-1) << val;
        std::string s = ss.str();
        if(std::all_of(s.begin()+1, s.end(), isdigit)) s += ".0";
        return vm->PyStr(s);
    });

    _vm->bindMethod("float", "__json__", [](VM* vm, const pkpy::ArgList& args) {
        _Float val = vm->PyFloat_AS_C(args[0]);
        if(std::isinf(val) || std::isnan(val)){
            vm->valueError("cannot jsonify 'nan' or 'inf'");
        }
        return vm->PyStr(std::to_string(val));
    });

    /************ PyString ************/
    _vm->bindMethod("str", "__new__", [](VM* vm, const pkpy::ArgList& args) {
        vm->__checkArgSize(args, 1);
        return vm->asStr(args[0]);
    });

    _vm->bindMethod("str", "__add__", [](VM* vm, const pkpy::ArgList& args) {
        if(!args[0]->isType(vm->_tp_str) || !args[1]->isType(vm->_tp_str))
            vm->typeError("unsupported operand type(s) for " "+" );
        const _Str& lhs = vm->PyStr_AS_C(args[0]);
        const _Str& rhs = vm->PyStr_AS_C(args[1]);
        return vm->PyStr(lhs + rhs);
    });

    _vm->bindMethod("str", "__len__", [](VM* vm, const pkpy::ArgList& args) {
        const _Str& _self = vm->PyStr_AS_C(args[0]);
        return vm->PyInt(_self.u8_length());
    });

    _vm->bindMethod("str", "__contains__", [](VM* vm, const pkpy::ArgList& args) {
        const _Str& _self = vm->PyStr_AS_C(args[0]);
        const _Str& _other = vm->PyStr_AS_C(args[1]);
        return vm->PyBool(_self.str().find(_other.str()) != _Str::npos);
    });

    _vm->bindMethod("str", "__str__", [](VM* vm, const pkpy::ArgList& args) {
        return args[0]; // str is immutable
    });

    _vm->bindMethod("str", "__iter__", [](VM* vm, const pkpy::ArgList& args) {
        return vm->PyIter(
            pkpy::make_shared<BaseIterator, StringIterator>(vm, args[0])
        );
    });

    _vm->bindMethod("str", "__repr__", [](VM* vm, const pkpy::ArgList& args) {
        const _Str& _self = vm->PyStr_AS_C(args[0]);
        return vm->PyStr(_self.__escape(true));
    });

    _vm->bindMethod("str", "__json__", [](VM* vm, const pkpy::ArgList& args) {
        const _Str& _self = vm->PyStr_AS_C(args[0]);
        return vm->PyStr(_self.__escape(false));
    });

    _vm->bindMethod("str", "__eq__", [](VM* vm, const pkpy::ArgList& args) {
        if(args[0]->isType(vm->_tp_str) && args[1]->isType(vm->_tp_str))
            return vm->PyBool(vm->PyStr_AS_C(args[0]) == vm->PyStr_AS_C(args[1]));
        return vm->PyBool(args[0] == args[1]);      // fallback
    });

    _vm->bindMethod("str", "__getitem__", [](VM* vm, const pkpy::ArgList& args) {
        const _Str& _self (vm->PyStr_AS_C(args[0]));

        if(args[1]->isType(vm->_tp_slice)){
            _Slice s = vm->PySlice_AS_C(args[1]);
            s.normalize(_self.u8_length());
            return vm->PyStr(_self.u8_substr(s.start, s.stop));
        }

        int _index = (int)vm->PyInt_AS_C(args[1]);
        _index = vm->normalizedIndex(_index, _self.u8_length());
        return vm->PyStr(_self.u8_getitem(_index));
    });

    _vm->bindMethod("str", "__gt__", [](VM* vm, const pkpy::ArgList& args) {
        const _Str& _self (vm->PyStr_AS_C(args[0]));
        const _Str& _obj (vm->PyStr_AS_C(args[1]));
        return vm->PyBool(_self > _obj);
    });

    _vm->bindMethod("str", "__lt__", [](VM* vm, const pkpy::ArgList& args) {
        const _Str& _self (vm->PyStr_AS_C(args[0]));
        const _Str& _obj (vm->PyStr_AS_C(args[1]));
        return vm->PyBool(_self < _obj);
    });

    _vm->bindMethod("str", "upper", [](VM* vm, const pkpy::ArgList& args) {
        vm->__checkArgSize(args, 1, true);
        const _Str& _self (vm->PyStr_AS_C(args[0]));
        _StrStream ss;
        for(auto c : _self.str()) ss << (char)toupper(c);
        return vm->PyStr(ss.str());
    });

    _vm->bindMethod("str", "lower", [](VM* vm, const pkpy::ArgList& args) {
        vm->__checkArgSize(args, 1, true);
        const _Str& _self (vm->PyStr_AS_C(args[0]));
        _StrStream ss;
        for(auto c : _self.str()) ss << (char)tolower(c);
        return vm->PyStr(ss.str());
    });

    _vm->bindMethod("str", "replace", [](VM* vm, const pkpy::ArgList& args) {
        vm->__checkArgSize(args, 3, true);
        const _Str& _self = vm->PyStr_AS_C(args[0]);
        const _Str& _old = vm->PyStr_AS_C(args[1]);
        const _Str& _new = vm->PyStr_AS_C(args[2]);
        std::string _copy = _self.str();
        // replace all occurences of _old with _new in _copy
        size_t pos = 0;
        while ((pos = _copy.find(_old.str(), pos)) != std::string::npos) {
            _copy.replace(pos, _old.str().length(), _new.str());
            pos += _new.str().length();
        }
        return vm->PyStr(_copy);
    });

    _vm->bindMethod("str", "startswith", [](VM* vm, const pkpy::ArgList& args) {
        vm->__checkArgSize(args, 2, true);
        const _Str& _self = vm->PyStr_AS_C(args[0]);
        const _Str& _prefix = vm->PyStr_AS_C(args[1]);
        return vm->PyBool(_self.str().find(_prefix.str()) == 0);
    });

    _vm->bindMethod("str", "endswith", [](VM* vm, const pkpy::ArgList& args) {
        vm->__checkArgSize(args, 2, true);
        const _Str& _self = vm->PyStr_AS_C(args[0]);
        const _Str& _suffix = vm->PyStr_AS_C(args[1]);
        return vm->PyBool(_self.str().rfind(_suffix.str()) == _self.str().length() - _suffix.str().length());
    });

    _vm->bindMethod("str", "join", [](VM* vm, const pkpy::ArgList& args) {
        vm->__checkArgSize(args, 2, true);
        const _Str& _self = vm->PyStr_AS_C(args[0]);
        PyVarList* _list;
        if(args[1]->isType(vm->_tp_list)){
            _list = &vm->PyList_AS_C(args[1]);
        }else if(args[1]->isType(vm->_tp_tuple)){
            _list = &vm->PyTuple_AS_C(args[1]);
        }else{
            vm->typeError("can only join a list or tuple");
        }
        _StrStream ss;
        for(int i = 0; i < _list->size(); i++){
            if(i > 0) ss << _self;
            ss << vm->PyStr_AS_C(vm->asStr(_list->operator[](i)));
        }
        return vm->PyStr(ss.str());
    });

    /************ PyList ************/
    _vm->bindMethod("list", "__iter__", [](VM* vm, const pkpy::ArgList& args) {
        vm->__checkType(args[0], vm->_tp_list);
        return vm->PyIter(
            pkpy::make_shared<BaseIterator, VectorIterator>(vm, args[0])
        );
    });

    _vm->bindMethod("list", "append", [](VM* vm, const pkpy::ArgList& args) {
        vm->__checkArgSize(args, 2, true);
        PyVarList& _self = vm->PyList_AS_C(args[0]);
        _self.push_back(args[1]);
        return vm->None;
    });

    _vm->bindMethod("list", "insert", [](VM* vm, const pkpy::ArgList& args) {
        vm->__checkArgSize(args, 3, true);
        PyVarList& _self = vm->PyList_AS_C(args[0]);
        int _index = (int)vm->PyInt_AS_C(args[1]);
        if(_index < 0) _index += _self.size();
        if(_index < 0) _index = 0;
        if(_index > _self.size()) _index = _self.size();
        _self.insert(_self.begin() + _index, args[2]);
        return vm->None;
    });

    _vm->bindMethod("list", "clear", [](VM* vm, const pkpy::ArgList& args) {
        vm->__checkArgSize(args, 1, true);
        vm->PyList_AS_C(args[0]).clear();
        return vm->None;
    });

    _vm->bindMethod("list", "copy", [](VM* vm, const pkpy::ArgList& args) {
        vm->__checkArgSize(args, 1, true);
        return vm->PyList(vm->PyList_AS_C(args[0]));
    });

    _vm->bindMethod("list", "__add__", [](VM* vm, const pkpy::ArgList& args) {
        const PyVarList& _self = vm->PyList_AS_C(args[0]);
        const PyVarList& _obj = vm->PyList_AS_C(args[1]);
        PyVarList _new_list = _self;
        _new_list.insert(_new_list.end(), _obj.begin(), _obj.end());
        return vm->PyList(_new_list);
    });

    _vm->bindMethod("list", "__len__", [](VM* vm, const pkpy::ArgList& args) {
        const PyVarList& _self = vm->PyList_AS_C(args[0]);
        return vm->PyInt(_self.size());
    });

    _vm->bindMethod("list", "__getitem__", [](VM* vm, const pkpy::ArgList& args) {
        const PyVarList& _self = vm->PyList_AS_C(args[0]);

        if(args[1]->isType(vm->_tp_slice)){
            _Slice s = vm->PySlice_AS_C(args[1]);
            s.normalize(_self.size());
            PyVarList _new_list;
            for(size_t i = s.start; i < s.stop; i++)
                _new_list.push_back(_self[i]);
            return vm->PyList(_new_list);
        }

        int _index = (int)vm->PyInt_AS_C(args[1]);
        _index = vm->normalizedIndex(_index, _self.size());
        return _self[_index];
    });

    _vm->bindMethod("list", "__setitem__", [](VM* vm, const pkpy::ArgList& args) {
        PyVarList& _self = vm->PyList_AS_C(args[0]);
        int _index = (int)vm->PyInt_AS_C(args[1]);
        _index = vm->normalizedIndex(_index, _self.size());
        _self[_index] = args[2];
        return vm->None;
    });

    _vm->bindMethod("list", "__delitem__", [](VM* vm, const pkpy::ArgList& args) {
        PyVarList& _self = vm->PyList_AS_C(args[0]);
        int _index = (int)vm->PyInt_AS_C(args[1]);
        _index = vm->normalizedIndex(_index, _self.size());
        _self.erase(_self.begin() + _index);
        return vm->None;
    });

    /************ PyTuple ************/
    _vm->bindMethod("tuple", "__new__", [](VM* vm, const pkpy::ArgList& args) {
        vm->__checkArgSize(args, 1);
        PyVarList _list = vm->PyList_AS_C(vm->call(vm->builtins->attribs["list"], args));
        return vm->PyTuple(_list);
    });

    _vm->bindMethod("tuple", "__iter__", [](VM* vm, const pkpy::ArgList& args) {
        vm->__checkType(args[0], vm->_tp_tuple);
        return vm->PyIter(
            pkpy::make_shared<BaseIterator, VectorIterator>(vm, args[0])
        );
    });

    _vm->bindMethod("tuple", "__len__", [](VM* vm, const pkpy::ArgList& args) {
        const PyVarList& _self = vm->PyTuple_AS_C(args[0]);
        return vm->PyInt(_self.size());
    });

    _vm->bindMethod("tuple", "__getitem__", [](VM* vm, const pkpy::ArgList& args) {
        const PyVarList& _self = vm->PyTuple_AS_C(args[0]);
        int _index = (int)vm->PyInt_AS_C(args[1]);
        _index = vm->normalizedIndex(_index, _self.size());
        return _self[_index];
    });

    /************ PyBool ************/
    _vm->bindMethod("bool", "__new__", [](VM* vm, const pkpy::ArgList& args) {
        vm->__checkArgSize(args, 1);
        return vm->asBool(args[0]);
    });

    _vm->bindMethod("bool", "__repr__", [](VM* vm, const pkpy::ArgList& args) {
        bool val = vm->PyBool_AS_C(args[0]);
        return vm->PyStr(val ? "True" : "False");
    });

    _vm->bindMethod("bool", "__json__", [](VM* vm, const pkpy::ArgList& args) {
        bool val = vm->PyBool_AS_C(args[0]);
        return vm->PyStr(val ? "true" : "false");
    });

    _vm->bindMethod("bool", "__eq__", [](VM* vm, const pkpy::ArgList& args) {
        return vm->PyBool(args[0] == args[1]);
    });

    _vm->bindMethod("bool", "__xor__", [](VM* vm, const pkpy::ArgList& args) {
        bool _self = vm->PyBool_AS_C(args[0]);
        bool _obj = vm->PyBool_AS_C(args[1]);
        return vm->PyBool(_self ^ _obj);
    });

    _vm->bindMethod("ellipsis", "__repr__", [](VM* vm, const pkpy::ArgList& args) {
        return vm->PyStr("Ellipsis");
    });

    _vm->bindMethod("_native_function", "__call__", [](VM* vm, const pkpy::ArgList& args) {
        const _CppFunc& _self = vm->PyNativeFunction_AS_C(args[0]);
        return _self(vm, args.subList(1));
    });

    _vm->bindMethod("function", "__call__", [](VM* vm, const pkpy::ArgList& args) {
        return vm->call(args[0], args.subList(1));
    });

    _vm->bindMethod("_bounded_method", "__call__", [](VM* vm, const pkpy::ArgList& args) {
        vm->__checkType(args[0], vm->_tp_bounded_method);
        const _BoundedMethod& _self = vm->PyBoundedMethod_AS_C(args[0]);
        pkpy::ArgList newArgs(args.size());
        newArgs[0] = _self.obj;
        for(int i = 1; i < args.size(); i++) newArgs[i] = args[i];
        return vm->call(_self.method, newArgs);
    });
}

#ifdef _WIN32
#define __EXPORT __declspec(dllexport)
#elif __APPLE__
#define __EXPORT __attribute__((visibility("default"))) __attribute__((used))
#else
#define __EXPORT
#endif


void __addModuleTime(VM* vm){
    PyVar mod = vm->newModule("time");
    vm->bindFunc(mod, "time", [](VM* vm, const pkpy::ArgList& args) {
        auto now = std::chrono::high_resolution_clock::now();
        return vm->PyFloat(std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count() / 1000000.0);
    });

    vm->bindFunc(mod, "sleep", [](VM* vm, const pkpy::ArgList& args) {
        vm->__checkArgSize(args, 1);
        if(!vm->isIntOrFloat(args[0])){
            vm->typeError("time.sleep() argument must be int or float");
        }
        double sec = vm->numToFloat(args[0]);
        vm->sleepForSecs(sec);
        return vm->None;
    });
}

void __addModuleSys(VM* vm){
    PyVar mod = vm->newModule("sys");
    vm->bindFunc(mod, "getrefcount", [](VM* vm, const pkpy::ArgList& args) {
        vm->__checkArgSize(args, 1);
        return vm->PyInt(args[0].use_count());
    });

    vm->bindFunc(mod, "getrecursionlimit", [](VM* vm, const pkpy::ArgList& args) {
        vm->__checkArgSize(args, 0);
        return vm->PyInt(vm->maxRecursionDepth);
    });

    vm->bindFunc(mod, "setrecursionlimit", [](VM* vm, const pkpy::ArgList& args) {
        vm->__checkArgSize(args, 1);
        vm->maxRecursionDepth = (int)vm->PyInt_AS_C(args[0]);
        return vm->None;
    });

    vm->setAttr(mod, "version", vm->PyStr(PK_VERSION));
}

void __addModuleJson(VM* vm){
    PyVar mod = vm->newModule("json");
    vm->bindFunc(mod, "loads", [](VM* vm, const pkpy::ArgList& args) {
        vm->__checkArgSize(args, 1);
        const _Str& expr = vm->PyStr_AS_C(args[0]);
        _Code code = compile(vm, expr.c_str(), "<json>", JSON_MODE, false);
        return vm->_exec(code, vm->topFrame()->_module, vm->topFrame()->copy_f_locals());
    });

    vm->bindFunc(mod, "dumps", [](VM* vm, const pkpy::ArgList& args) {
        vm->__checkArgSize(args, 1);
        return vm->asJson(args[0]);
    });
}

void __addModuleMath(VM* vm){
    PyVar mod = vm->newModule("math");
    vm->setAttr(mod, "pi", vm->PyFloat(3.1415926535897932384));
    vm->setAttr(mod, "e" , vm->PyFloat(2.7182818284590452354));

    vm->bindFunc(mod, "log", [](VM* vm, const pkpy::ArgList& args) {
        vm->__checkArgSize(args, 1);
        return vm->PyFloat(log(vm->numToFloat(args[0])));
    });

    vm->bindFunc(mod, "log10", [](VM* vm, const pkpy::ArgList& args) {
        vm->__checkArgSize(args, 1);
        return vm->PyFloat(log10(vm->numToFloat(args[0])));
    });

    vm->bindFunc(mod, "log2", [](VM* vm, const pkpy::ArgList& args) {
        vm->__checkArgSize(args, 1);
        return vm->PyFloat(log2(vm->numToFloat(args[0])));
    });

    vm->bindFunc(mod, "sin", [](VM* vm, const pkpy::ArgList& args) {
        vm->__checkArgSize(args, 1);
        return vm->PyFloat(sin(vm->numToFloat(args[0])));
    });

    vm->bindFunc(mod, "cos", [](VM* vm, const pkpy::ArgList& args) {
        vm->__checkArgSize(args, 1);
        return vm->PyFloat(cos(vm->numToFloat(args[0])));
    });

    vm->bindFunc(mod, "tan", [](VM* vm, const pkpy::ArgList& args) {
        vm->__checkArgSize(args, 1);
        return vm->PyFloat(tan(vm->numToFloat(args[0])));
    });

    vm->bindFunc(mod, "isclose", [](VM* vm, const pkpy::ArgList& args) {
        vm->__checkArgSize(args, 2);
        _Float a = vm->numToFloat(args[0]);
        _Float b = vm->numToFloat(args[1]);
        return vm->PyBool(fabs(a - b) < 1e-9);
    });

    vm->bindFunc(mod, "isnan", [](VM* vm, const pkpy::ArgList& args) {
        vm->__checkArgSize(args, 1);
        return vm->PyBool(std::isnan(vm->numToFloat(args[0])));
    });

    vm->bindFunc(mod, "isinf", [](VM* vm, const pkpy::ArgList& args) {
        vm->__checkArgSize(args, 1);
        return vm->PyBool(std::isinf(vm->numToFloat(args[0])));
    });
}

class _PkExported{
public:
    virtual ~_PkExported() = default;
    virtual void* get() = 0;
};

static std::vector<_PkExported*> _pkLookupTable;

template<typename T>
class PkExported : public _PkExported{
    T* _ptr;
public:
    template<typename... Args>
    PkExported(Args&&... args) {
        _ptr = new T(std::forward<Args>(args)...);
        _pkLookupTable.push_back(this);
    }
    
    ~PkExported() override { delete _ptr; }
    void* get() override { return _ptr; }
    operator T*() { return _ptr; }
};

#define pkpy_allocate(T, ...) *(new PkExported<T>(__VA_ARGS__))


extern "C" {
    __EXPORT
    /// Delete a pointer allocated by `pkpy_xxx_xxx`.
    /// It can be `VM*`, `REPL*`, `ThreadedVM*`, `char*`, etc.
    /// 
    /// !!!
    /// If the pointer is not allocated by `pkpy_xxx_xxx`, the behavior is undefined.
    /// For char*, you can also use trivial `delete` in your language.
    /// !!!
    void pkpy_delete(void* p){
        for(int i = 0; i < _pkLookupTable.size(); i++){
            if(_pkLookupTable[i]->get() == p){
                delete _pkLookupTable[i];
                _pkLookupTable.erase(_pkLookupTable.begin() + i);
                return;
            }
        }
        free(p);
    }

    __EXPORT
    /// Run a given source on a virtual machine.
    /// 
    /// Return `true` if there is no compile error.
    bool pkpy_vm_exec(VM* vm, const char* source){
        _Code code = compile(vm, source, "main.py");
        if(code == nullptr) return false;
        vm->exec(code);
        return true;
    }

    __EXPORT
    /// Get a global variable of a virtual machine.
    /// 
    /// Return a json representing the result.
    /// If the variable is not found, return `nullptr`.
    char* pkpy_vm_get_global(VM* vm, const char* name){
        auto it = vm->_main->attribs.find(name);
        if(it == vm->_main->attribs.end()) return nullptr;
        try{
            _Str _json = vm->PyStr_AS_C(vm->asJson(it->second));
            return strdup(_json.c_str());
        }catch(...){
            return nullptr;
        }
    }

    __EXPORT
    /// Evaluate an expression.
    /// 
    /// Return a json representing the result.
    /// If there is any error, return `nullptr`.
    char* pkpy_vm_eval(VM* vm, const char* source){
        _Code code = compile(vm, source, "<eval>", EVAL_MODE);
        if(code == nullptr) return nullptr;
        PyVarOrNull ret = vm->exec(code);
        if(ret == nullptr) return nullptr;
        try{
            _Str _json = vm->PyStr_AS_C(vm->asJson(ret));
            return strdup(_json.c_str());
        }catch(...){
            return nullptr;
        }
    }

    __EXPORT
    /// Create a REPL, using the given virtual machine as the backend.
    REPL* pkpy_new_repl(VM* vm){
        return pkpy_allocate(REPL, vm);
    }

    __EXPORT
    /// Input a source line to an interactive console.
    /// 
    /// Return `0` if need more lines,
    /// `1` if execution happened,
    /// `2` if execution skipped (compile error or empty input).
    int pkpy_repl_input(REPL* r, const char* line){
        return r->input(line);
    }

    __EXPORT
    /// Add a source module into a virtual machine.
    ///
    /// Return `true` if there is no complie error.
    bool pkpy_vm_add_module(VM* vm, const char* name, const char* source){
        // compile the module but don't execute it
        _Code code = compile(vm, source, name + _Str(".py"));
        if(code == nullptr) return false;
        vm->addLazyModule(name, code);
        return true;
    }

    void __vm_init(VM* vm){
        __initializeBuiltinFunctions(vm);
        __addModuleSys(vm);
        __addModuleTime(vm);
        __addModuleJson(vm);
        __addModuleMath(vm);

        _Code code = compile(vm, __BUILTINS_CODE, "<builtins>");
        if(code == nullptr) exit(1);
        vm->_exec(code, vm->builtins, {});
        pkpy_vm_add_module(vm, "random", __RANDOM_CODE);
        pkpy_vm_add_module(vm, "os", __OS_CODE);
    }

    __EXPORT
    /// Create a virtual machine.
    VM* pkpy_new_vm(bool use_stdio){
        VM* vm = pkpy_allocate(VM, use_stdio);
        __vm_init(vm);
        return vm;
    }

    __EXPORT
    /// Create a virtual machine that supports asynchronous execution.
    ThreadedVM* pkpy_new_tvm(bool use_stdio){
        ThreadedVM* vm = pkpy_allocate(ThreadedVM, use_stdio);
        __vm_init(vm);
        return vm;
    }

    __EXPORT
    /// Read the standard output and standard error as string of a virtual machine.
    /// The `vm->use_stdio` should be `false`.
    /// After this operation, both stream will be cleared.
    ///
    /// Return a json representing the result.
    char* pkpy_vm_read_output(VM* vm){
        if(vm->use_stdio) return nullptr;
        _StrStream* s_out = (_StrStream*)(vm->_stdout);
        _StrStream* s_err = (_StrStream*)(vm->_stderr);
        _Str _stdout = s_out->str();
        _Str _stderr = s_err->str();
        _StrStream ss;
        ss << '{' << "\"stdout\": " << _stdout.__escape(false);
        ss << ", ";
        ss << "\"stderr\": " << _stderr.__escape(false) << '}';
        s_out->str("");
        s_err->str("");
        return strdup(ss.str().c_str());
    }

    __EXPORT
    /// Get the current state of a threaded virtual machine.
    ///
    /// Return `0` for `THREAD_READY`,
    /// `1` for `THREAD_RUNNING`,
    /// `2` for `THREAD_SUSPENDED`,
    /// `3` for `THREAD_FINISHED`.
    int pkpy_tvm_get_state(ThreadedVM* vm){
        return vm->getState();
    }

    __EXPORT
    /// Set the state of a threaded virtual machine to `THREAD_READY`.
    /// The current state should be `THREAD_FINISHED`.
    void pkpy_tvm_reset_state(ThreadedVM* vm){
        vm->resetState();
    }

    __EXPORT
    /// Read the current JSONRPC request from shared string buffer.
    char* pkpy_tvm_read_jsonrpc_request(ThreadedVM* vm){
        _Str s = vm->readJsonRpcRequest();
        return strdup(s.c_str());
    }

    __EXPORT
    /// Write a JSONRPC response to shared string buffer.
    void pkpy_tvm_write_jsonrpc_response(ThreadedVM* vm, const char* value){
        vm->writeJsonrpcResponse(value);
    }

    __EXPORT
    /// Emit a KeyboardInterrupt signal to stop a running threaded virtual machine. 
    void pkpy_tvm_terminate(ThreadedVM* vm){
        vm->terminate();
    }

    __EXPORT
    /// Run a given source on a threaded virtual machine.
    /// The excution will be started in a new thread.
    /// 
    /// Return `true` if there is no compile error.
    bool pkpy_tvm_exec_async(VM* vm, const char* source){
        // although this is a method of VM, it's only used in ThreadedVM
        _Code code = compile(vm, source, "main.py");
        if(code == nullptr) return false;
        vm->execAsync(code);
        return true;
    }
}

#endif // POCKETPY_H