/*
 *  Copyright (c) 2023 blueloveTH
 *  Distributed Under The MIT License
 *  https://github.com/blueloveTH/pocketpy
 */

#ifndef POCKETPY_H
#define POCKETPY_H


#ifdef _MSC_VER
#pragma warning (disable:4267)
#pragma warning (disable:4101)
#define _CRT_NONSTDC_NO_DEPRECATE
#define strdup _strdup
#endif

#include <sstream>
#include <regex>
#include <stack>
#include <cmath>
#include <cstdlib>
#include <stdexcept>
#include <vector>
#include <string>
#include <cstring>
#include <chrono>
#include <string_view>
#include <queue>
#include <iomanip>
#include <memory>
#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <algorithm>

#define PK_VERSION				"0.9.3"
#define PK_EXTRA_CHECK 			0

#if (defined(__ANDROID__) && __ANDROID_API__ <= 22) || defined(__EMSCRIPTEN__)
#define PK_ENABLE_FILEIO 		0
#else
#define PK_ENABLE_FILEIO 		1
#endif

#if defined(__EMSCRIPTEN__) || defined(__arm__) || defined(__i386__)
typedef int32_t i64;
typedef float f64;
#define S_TO_INT std::stoi
#define S_TO_FLOAT std::stof
#else
typedef int64_t i64;
typedef double f64;
#define S_TO_INT std::stoll
#define S_TO_FLOAT std::stod
#endif

namespace pkpy{

namespace std = ::std;

struct Dummy {  };
struct DummyInstance {  };
struct DummyModule { };
#define DUMMY_VAL Dummy()

struct Type {
	int index;
	Type(): index(-1) {}
	Type(int index): index(index) {}
	inline bool operator==(Type other) const noexcept {
		return this->index == other.index;
	}
	inline bool operator!=(Type other) const noexcept {
		return this->index != other.index;
	}
};

//#define THREAD_LOCAL thread_local
#define THREAD_LOCAL
#define CPP_LAMBDA(x) ([](VM* vm, Args& args) { return x; })
#define CPP_NOT_IMPLEMENTED() ([](VM* vm, Args& args) { vm->NotImplementedError(); return vm->None; })

#ifdef POCKETPY_H
#define UNREACHABLE() throw std::runtime_error( "L" + std::to_string(__LINE__) + " UNREACHABLE()!");
#else
#define UNREACHABLE() throw std::runtime_error( __FILE__ + std::string(":") + std::to_string(__LINE__) + " UNREACHABLE()!");
#endif

const float kLocalsLoadFactor = 0.67f;
const float kInstAttrLoadFactor = 0.67f;
const float kTypeAttrLoadFactor = 0.5f;
} // namespace pkpy


namespace pkpy{

struct PyObject;

template<typename T>
struct SpAllocator {
    template<typename U>
    inline static int* alloc(){
        return (int*)malloc(sizeof(int) + sizeof(U));
    }

    inline static void dealloc(int* counter){
        ((T*)(counter + 1))->~T();
        free(counter);
    }
};

template <typename T>
struct shared_ptr {
    union {
        int* counter;
        i64 bits;
    };

#define _t() (T*)(counter + 1)
#define _inc_counter() if(!is_tagged() && counter) ++(*counter)
#define _dec_counter() if(!is_tagged() && counter && --(*counter) == 0) SpAllocator<T>::dealloc(counter)

public:
    shared_ptr() : counter(nullptr) {}
    shared_ptr(int* counter) : counter(counter) {}
    shared_ptr(const shared_ptr& other) : counter(other.counter) {
        _inc_counter();
    }
    shared_ptr(shared_ptr&& other) noexcept : counter(other.counter) {
        other.counter = nullptr;
    }
    ~shared_ptr() { _dec_counter(); }

    bool operator==(const shared_ptr& other) const { return counter == other.counter; }
    bool operator!=(const shared_ptr& other) const { return counter != other.counter; }
    bool operator<(const shared_ptr& other) const { return counter < other.counter; }
    bool operator>(const shared_ptr& other) const { return counter > other.counter; }
    bool operator<=(const shared_ptr& other) const { return counter <= other.counter; }
    bool operator>=(const shared_ptr& other) const { return counter >= other.counter; }
    bool operator==(std::nullptr_t) const { return counter == nullptr; }
    bool operator!=(std::nullptr_t) const { return counter != nullptr; }

    shared_ptr& operator=(const shared_ptr& other) {
        _dec_counter();
        counter = other.counter;
        _inc_counter();
        return *this;
    }

    shared_ptr& operator=(shared_ptr&& other) noexcept {
        _dec_counter();
        counter = other.counter;
        other.counter = nullptr;
        return *this;
    }

    T& operator*() const { return *_t(); }
    T* operator->() const { return _t(); }
    T* get() const { return _t(); }

    int use_count() const { 
        if(is_tagged()) return 1;
        return counter ? *counter : 0;
    }

    void reset(){
        _dec_counter();
        counter = nullptr;
    }

    inline constexpr bool is_tagged() const {
        if constexpr(!std::is_same_v<T, PyObject>) return false;
        return (bits & 0b11) != 0b00;
    }
    inline bool is_tag_00() const { return (bits & 0b11) == 0b00; }
    inline bool is_tag_01() const { return (bits & 0b11) == 0b01; }
    inline bool is_tag_10() const { return (bits & 0b11) == 0b10; }
    inline bool is_tag_11() const { return (bits & 0b11) == 0b11; }
};

#undef _t
#undef _inc_counter
#undef _dec_counter

    template <typename T, typename U, typename... Args>
    shared_ptr<T> make_sp(Args&&... args) {
        static_assert(std::is_base_of_v<T, U>, "U must be derived from T");
        static_assert(std::has_virtual_destructor_v<T>, "T must have virtual destructor");
        static_assert(!std::is_same_v<T, PyObject> || (!std::is_same_v<U, i64> && !std::is_same_v<U, f64>));
        int* p = SpAllocator<T>::template alloc<U>(); *p = 1;
        new(p+1) U(std::forward<Args>(args)...);
        return shared_ptr<T>(p);
    }

    template <typename T, typename... Args>
    shared_ptr<T> make_sp(Args&&... args) {
        int* p = SpAllocator<T>::template alloc<T>(); *p = 1;
        new(p+1) T(std::forward<Args>(args)...);
        return shared_ptr<T>(p);
    }

static_assert(sizeof(i64) == sizeof(int*));
static_assert(sizeof(f64) == sizeof(int*));
static_assert(sizeof(shared_ptr<PyObject>) == sizeof(int*));
static_assert(std::numeric_limits<float>::is_iec559);
static_assert(std::numeric_limits<double>::is_iec559);

template<typename T, int __Bucket, int __BucketSize=32>
struct SmallArrayPool {
    std::vector<T*> buckets[__Bucket+1];

    T* alloc(int n){
        if(n == 0) return nullptr;
        if(n > __Bucket || buckets[n].empty()){
            return new T[n];
        }else{
            T* p = buckets[n].back();
            buckets[n].pop_back();
            return p;
        }
    }

    void dealloc(T* p, int n){
        if(n == 0) return;
        if(n > __Bucket || buckets[n].size() >= __BucketSize){
            delete[] p;
        }else{
            buckets[n].push_back(p);
        }
    }

    ~SmallArrayPool(){
        for(int i=1; i<=__Bucket; i++){
            for(auto p: buckets[i]) delete[] p;
        }
    }
};


typedef shared_ptr<PyObject> PyVar;
typedef PyVar PyVarOrNull;
typedef PyVar PyVarRef;

};  // namespace pkpy



namespace pkpy {

typedef std::stringstream StrStream;

class Str : public std::string {
    mutable std::vector<uint16_t>* _u8_index = nullptr;

    void utf8_lazy_init() const{
        if(_u8_index != nullptr) return;
        _u8_index = new std::vector<uint16_t>();
        _u8_index->reserve(size());
        if(size() > 65535) throw std::runtime_error("str has more than 65535 bytes.");
        for(uint16_t i = 0; i < size(); i++){
            // https://stackoverflow.com/questions/3911536/utf-8-unicode-whats-with-0xc0-and-0x80
            if((at(i) & 0xC0) != 0x80) _u8_index->push_back(i);
        }
    }
public:
    uint16_t _cached_sn_index = 0;

    Str() : std::string() {}
    Str(const char* s) : std::string(s) {}
    Str(const char* s, size_t n) : std::string(s, n) {}
    Str(const std::string& s) : std::string(s) {}
    Str(const Str& s) : std::string(s) {
        if(s._u8_index != nullptr){
            _u8_index = new std::vector<uint16_t>(*s._u8_index);
        }
    }
    Str(Str&& s) : std::string(std::move(s)) {
        delete _u8_index;
        _u8_index = s._u8_index;
        s._u8_index = nullptr;
    }

    i64 _to_u8_index(i64 index) const{
        utf8_lazy_init();
        auto p = std::lower_bound(_u8_index->begin(), _u8_index->end(), index);
        if(p != _u8_index->end() && *p != index) UNREACHABLE();
        return p - _u8_index->begin();
    }

    int u8_length() const {
        utf8_lazy_init();
        return _u8_index->size();
    }

    Str u8_getitem(int i) const{
        return u8_substr(i, i+1);
    }

    Str u8_substr(int start, int end) const{
        utf8_lazy_init();
        if(start >= end) return Str();
        int c_end = end >= _u8_index->size() ? size() : _u8_index->at(end);
        return substr(_u8_index->at(start), c_end - _u8_index->at(start));
    }

    Str lstrip() const {
        Str copy(*this);
        copy.erase(copy.begin(), std::find_if(copy.begin(), copy.end(), [](char c) {
            // std::isspace(c) does not working on windows (Debug)
            return c != ' ' && c != '\t' && c != '\r' && c != '\n';
        }));
        return Str(copy);
    }

    size_t hash() const {
        return std::hash<std::string>()(*this);
    }

    Str escape(bool single_quote) const {
        StrStream ss;
        ss << (single_quote ? '\'' : '"');
        for (int i=0; i<length(); i++) {
            char c = this->operator[](i);
            switch (c) {
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
                    if ('\x00' <= c && c <= '\x1f') {
                        ss << "\\u"
                        << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(c);
                    } else {
                        ss << c;
                    }
            }
        }
        ss << (single_quote ? '\'' : '"');
        return ss.str();
    }

    Str& operator=(const Str& s){
        this->std::string::operator=(s);
        delete _u8_index;
        if(s._u8_index != nullptr){
            _u8_index = new std::vector<uint16_t>(*s._u8_index);
        }
        return *this;
    }

    Str& operator=(Str&& s){
        this->std::string::operator=(std::move(s));
        delete _u8_index;
        this->_u8_index = s._u8_index;
        s._u8_index = nullptr;
        return *this;
    }

    ~Str(){ delete _u8_index;}
};

const uint32_t kLoRangeA[] = {170,186,443,448,660,1488,1519,1568,1601,1646,1649,1749,1774,1786,1791,1808,1810,1869,1969,1994,2048,2112,2144,2208,2230,2308,2365,2384,2392,2418,2437,2447,2451,2474,2482,2486,2493,2510,2524,2527,2544,2556,2565,2575,2579,2602,2610,2613,2616,2649,2654,2674,2693,2703,2707,2730,2738,2741,2749,2768,2784,2809,2821,2831,2835,2858,2866,2869,2877,2908,2911,2929,2947,2949,2958,2962,2969,2972,2974,2979,2984,2990,3024,3077,3086,3090,3114,3133,3160,3168,3200,3205,3214,3218,3242,3253,3261,3294,3296,3313,3333,3342,3346,3389,3406,3412,3423,3450,3461,3482,3507,3517,3520,3585,3634,3648,3713,3716,3718,3724,3749,3751,3762,3773,3776,3804,3840,3904,3913,3976,4096,4159,4176,4186,4193,4197,4206,4213,4238,4352,4682,4688,4696,4698,4704,4746,4752,4786,4792,4800,4802,4808,4824,4882,4888,4992,5121,5743,5761,5792,5873,5888,5902,5920,5952,5984,5998,6016,6108,6176,6212,6272,6279,6314,6320,6400,6480,6512,6528,6576,6656,6688,6917,6981,7043,7086,7098,7168,7245,7258,7401,7406,7413,7418,8501,11568,11648,11680,11688,11696,11704,11712,11720,11728,11736,12294,12348,12353,12447,12449,12543,12549,12593,12704,12784,13312,19968,40960,40982,42192,42240,42512,42538,42606,42656,42895,42999,43003,43011,43015,43020,43072,43138,43250,43259,43261,43274,43312,43360,43396,43488,43495,43514,43520,43584,43588,43616,43633,43642,43646,43697,43701,43705,43712,43714,43739,43744,43762,43777,43785,43793,43808,43816,43968,44032,55216,55243,63744,64112,64285,64287,64298,64312,64318,64320,64323,64326,64467,64848,64914,65008,65136,65142,65382,65393,65440,65474,65482,65490,65498,65536,65549,65576,65596,65599,65616,65664,66176,66208,66304,66349,66370,66384,66432,66464,66504,66640,66816,66864,67072,67392,67424,67584,67592,67594,67639,67644,67647,67680,67712,67808,67828,67840,67872,67968,68030,68096,68112,68117,68121,68192,68224,68288,68297,68352,68416,68448,68480,68608,68864,69376,69415,69424,69600,69635,69763,69840,69891,69956,69968,70006,70019,70081,70106,70108,70144,70163,70272,70280,70282,70287,70303,70320,70405,70415,70419,70442,70450,70453,70461,70480,70493,70656,70727,70751,70784,70852,70855,71040,71128,71168,71236,71296,71352,71424,71680,71935,72096,72106,72161,72163,72192,72203,72250,72272,72284,72349,72384,72704,72714,72768,72818,72960,72968,72971,73030,73056,73063,73066,73112,73440,73728,74880,77824,82944,92160,92736,92880,92928,93027,93053,93952,94032,94208,100352,110592,110928,110948,110960,113664,113776,113792,113808,123136,123214,123584,124928,126464,126469,126497,126500,126503,126505,126516,126521,126523,126530,126535,126537,126539,126541,126545,126548,126551,126553,126555,126557,126559,126561,126564,126567,126572,126580,126585,126590,126592,126603,126625,126629,126635,131072,173824,177984,178208,183984,194560};
const uint32_t kLoRangeB[] = {170,186,443,451,660,1514,1522,1599,1610,1647,1747,1749,1775,1788,1791,1808,1839,1957,1969,2026,2069,2136,2154,2228,2237,2361,2365,2384,2401,2432,2444,2448,2472,2480,2482,2489,2493,2510,2525,2529,2545,2556,2570,2576,2600,2608,2611,2614,2617,2652,2654,2676,2701,2705,2728,2736,2739,2745,2749,2768,2785,2809,2828,2832,2856,2864,2867,2873,2877,2909,2913,2929,2947,2954,2960,2965,2970,2972,2975,2980,2986,3001,3024,3084,3088,3112,3129,3133,3162,3169,3200,3212,3216,3240,3251,3257,3261,3294,3297,3314,3340,3344,3386,3389,3406,3414,3425,3455,3478,3505,3515,3517,3526,3632,3635,3653,3714,3716,3722,3747,3749,3760,3763,3773,3780,3807,3840,3911,3948,3980,4138,4159,4181,4189,4193,4198,4208,4225,4238,4680,4685,4694,4696,4701,4744,4749,4784,4789,4798,4800,4805,4822,4880,4885,4954,5007,5740,5759,5786,5866,5880,5900,5905,5937,5969,5996,6000,6067,6108,6210,6264,6276,6312,6314,6389,6430,6509,6516,6571,6601,6678,6740,6963,6987,7072,7087,7141,7203,7247,7287,7404,7411,7414,7418,8504,11623,11670,11686,11694,11702,11710,11718,11726,11734,11742,12294,12348,12438,12447,12538,12543,12591,12686,12730,12799,19893,40943,40980,42124,42231,42507,42527,42539,42606,42725,42895,42999,43009,43013,43018,43042,43123,43187,43255,43259,43262,43301,43334,43388,43442,43492,43503,43518,43560,43586,43595,43631,43638,43642,43695,43697,43702,43709,43712,43714,43740,43754,43762,43782,43790,43798,43814,43822,44002,55203,55238,55291,64109,64217,64285,64296,64310,64316,64318,64321,64324,64433,64829,64911,64967,65019,65140,65276,65391,65437,65470,65479,65487,65495,65500,65547,65574,65594,65597,65613,65629,65786,66204,66256,66335,66368,66377,66421,66461,66499,66511,66717,66855,66915,67382,67413,67431,67589,67592,67637,67640,67644,67669,67702,67742,67826,67829,67861,67897,68023,68031,68096,68115,68119,68149,68220,68252,68295,68324,68405,68437,68466,68497,68680,68899,69404,69415,69445,69622,69687,69807,69864,69926,69956,70002,70006,70066,70084,70106,70108,70161,70187,70278,70280,70285,70301,70312,70366,70412,70416,70440,70448,70451,70457,70461,70480,70497,70708,70730,70751,70831,70853,70855,71086,71131,71215,71236,71338,71352,71450,71723,71935,72103,72144,72161,72163,72192,72242,72250,72272,72329,72349,72440,72712,72750,72768,72847,72966,72969,73008,73030,73061,73064,73097,73112,73458,74649,75075,78894,83526,92728,92766,92909,92975,93047,93071,94026,94032,100343,101106,110878,110930,110951,111355,113770,113788,113800,113817,123180,123214,123627,125124,126467,126495,126498,126500,126503,126514,126519,126521,126523,126530,126535,126537,126539,126543,126546,126548,126551,126553,126555,126557,126559,126562,126564,126570,126578,126583,126588,126590,126601,126619,126627,126633,126651,173782,177972,178205,183969,191456,195101};

bool is_unicode_Lo_char(uint32_t c) {
    auto index = std::lower_bound(kLoRangeA, kLoRangeA + 476, c) - kLoRangeA;
    if(c == kLoRangeA[index]) return true;
    index -= 1;
    if(index < 0) return false;
    return c >= kLoRangeA[index] && c <= kLoRangeB[index];
}


struct StrName {
    uint16_t index;
    StrName(): index(0) {}
    StrName(uint16_t index): index(index) {}
    StrName(const char* s): index(get(s).index) {}
    StrName(const Str& s){
        if(s._cached_sn_index != 0){
            index = s._cached_sn_index;
        } else {
            index = get(s).index;
        }
    }
    const Str& str() const { return _r_interned[index-1]; }
    bool empty() const { return index == 0; }

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

    static std::map<Str, uint16_t, std::less<>> _interned;
    static std::vector<Str> _r_interned;

    static StrName get(const Str& s){
        return get(s.c_str());
    }

    static StrName get(const char* s){
        auto it = _interned.find(s);
        if(it != _interned.end()) return StrName(it->second);
        uint16_t index = (uint16_t)(_r_interned.size() + 1);
        _interned[s] = index;
        _r_interned.push_back(s);
        return StrName(index);
    }
};

std::map<Str, uint16_t, std::less<>> StrName::_interned;
std::vector<Str> StrName::_r_interned;

const StrName __class__ = StrName::get("__class__");
const StrName __base__ = StrName::get("__base__");
const StrName __new__ = StrName::get("__new__");
const StrName __iter__ = StrName::get("__iter__");
const StrName __str__ = StrName::get("__str__");
const StrName __repr__ = StrName::get("__repr__");
const StrName __getitem__ = StrName::get("__getitem__");
const StrName __setitem__ = StrName::get("__setitem__");
const StrName __delitem__ = StrName::get("__delitem__");
const StrName __contains__ = StrName::get("__contains__");
const StrName __init__ = StrName::get("__init__");
const StrName __json__ = StrName::get("__json__");
const StrName __name__ = StrName::get("__name__");
const StrName __len__ = StrName::get("__len__");
const StrName __get__ = StrName::get("__get__");
const StrName __getattr__ = StrName::get("__getattr__");
const StrName __setattr__ = StrName::get("__setattr__");
const StrName __call__ = StrName::get("__call__");

const StrName m_eval = StrName::get("eval");
const StrName m_self = StrName::get("self");
const StrName __enter__ = StrName::get("__enter__");
const StrName __exit__ = StrName::get("__exit__");

const StrName CMP_SPECIAL_METHODS[] = {
    StrName::get("__lt__"), StrName::get("__le__"), StrName::get("__eq__"),
    StrName::get("__ne__"), StrName::get("__gt__"), StrName::get("__ge__")
};

const StrName BINARY_SPECIAL_METHODS[] = {
    StrName::get("__add__"), StrName::get("__sub__"), StrName::get("__mul__"),
    StrName::get("__truediv__"), StrName::get("__floordiv__"),
    StrName::get("__mod__"), StrName::get("__pow__")
};

const StrName BITWISE_SPECIAL_METHODS[] = {
    StrName::get("__lshift__"), StrName::get("__rshift__"),
    StrName::get("__and__"), StrName::get("__or__"), StrName::get("__xor__")
};

} // namespace pkpy


namespace pkpy {
    using List = std::vector<PyVar>;

    class Args {
        static THREAD_LOCAL SmallArrayPool<PyVar, 10> _pool;

        PyVar* _args;
        int _size;

        inline void _alloc(int n){
            this->_args = _pool.alloc(n);
            this->_size = n;
        }

    public:
        Args(int n){ _alloc(n); }

        Args(const Args& other){
            _alloc(other._size);
            for(int i=0; i<_size; i++) _args[i] = other._args[i];
        }

        Args(Args&& other) noexcept {
            this->_args = other._args;
            this->_size = other._size;
            other._args = nullptr;
            other._size = 0;
        }

        static pkpy::Args from_list(List&& other) noexcept {
            Args ret(other.size());
            memcpy((void*)ret._args, (void*)other.data(), sizeof(PyVar)*ret.size());
            memset((void*)other.data(), 0, sizeof(PyVar)*ret.size());
            other.clear();
            return ret;
        }

        PyVar& operator[](int i){ return _args[i]; }
        const PyVar& operator[](int i) const { return _args[i]; }

        Args& operator=(Args&& other) noexcept {
            _pool.dealloc(_args, _size);
            this->_args = other._args;
            this->_size = other._size;
            other._args = nullptr;
            other._size = 0;
            return *this;
        }

        inline int size() const { return _size; }

        List move_to_list() noexcept {
            List ret(_size);
            memcpy((void*)ret.data(), (void*)_args, sizeof(PyVar)*_size);
            memset((void*)_args, 0, sizeof(PyVar)*_size);
            return ret;
        }

        void extend_self(const PyVar& self){
            static_assert(std::is_standard_layout_v<PyVar>);
            PyVar* old_args = _args;
            int old_size = _size;
            _alloc(old_size+1);
            _args[0] = self;
            if(old_size == 0) return;

            memcpy((void*)(_args+1), (void*)old_args, sizeof(PyVar)*old_size);
            memset((void*)old_args, 0, sizeof(PyVar)*old_size);
            _pool.dealloc(old_args, old_size);
        }

        ~Args(){ _pool.dealloc(_args, _size); }
    };

    static const Args _zero(0);
    inline const Args& no_arg() { return _zero; }

    template<typename T>
    Args one_arg(T&& a) {
        Args ret(1);
        ret[0] = std::forward<T>(a);
        return ret;
    }

    template<typename T1, typename T2>
    Args two_args(T1&& a, T2&& b) {
        Args ret(2);
        ret[0] = std::forward<T1>(a);
        ret[1] = std::forward<T2>(b);
        return ret;
    }

    typedef Args Tuple;
    THREAD_LOCAL SmallArrayPool<PyVar, 10> Args::_pool;
}   // namespace pkpy


namespace pkpy{

const int kNameDictNodeSize = sizeof(StrName) + sizeof(PyVar);

template<int __Bucket, int __BucketSize=32>
struct DictArrayPool {
    std::vector<StrName*> buckets[__Bucket+1];

    StrName* alloc(uint16_t n){
        StrName* _keys;
        if(n > __Bucket || buckets[n].empty()){
            _keys = (StrName*)malloc(kNameDictNodeSize * n);
            memset((void*)_keys, 0, kNameDictNodeSize * n);
        }else{
            _keys = buckets[n].back();
            memset((void*)_keys, 0, sizeof(StrName) * n);
            buckets[n].pop_back();
        }
        return _keys;
    }

    void dealloc(StrName* head, uint16_t n){
        PyVar* _values = (PyVar*)(head + n);
        if(n > __Bucket || buckets[n].size() >= __BucketSize){
            for(int i=0; i<n; i++) _values[i].~PyVar();
            free(head);
        }else{
            buckets[n].push_back(head);
        }
    }

    ~DictArrayPool(){
        // let it leak, since this object is static
    }
};

const std::vector<uint16_t> kHashSeeds = {9629, 43049, 13267, 59509, 39251, 1249, 35803, 54469, 27689, 9719, 34897, 18973, 30661, 19913, 27919, 32143, 3467, 28019, 1051, 39419, 1361, 28547, 48197, 2609, 24317, 22861, 41467, 17623, 52837, 59053, 33589, 32117};
static DictArrayPool<32> _dict_pool;

uint16_t find_next_capacity(uint16_t n){
    uint16_t x = 2;
    while(x < n) x <<= 1;
    return x;
}

#define _hash(key, mask, hash_seed) ( ( (key).index * (hash_seed) >> 8 ) & (mask) )

uint16_t find_perfect_hash_seed(uint16_t capacity, const std::vector<StrName>& keys){
    if(keys.empty()) return kHashSeeds[0];
    std::set<uint16_t> indices;
    std::pair<uint16_t, float> best_score = {kHashSeeds[0], 0.0f};
    for(int i=0; i<kHashSeeds.size(); i++){
        indices.clear();
        for(auto key: keys){
            uint16_t index = _hash(key, capacity-1, kHashSeeds[i]);
            indices.insert(index);
        }
        float score = indices.size() / (float)keys.size();
        if(score > best_score.second) best_score = {kHashSeeds[i], score};
    }
    return best_score.first;
}

struct NameDict {
    uint16_t _capacity;
    uint16_t _size;
    float _load_factor;
    uint16_t _hash_seed;
    uint16_t _mask;
    StrName* _keys;

    inline PyVar& value(uint16_t i){
        return reinterpret_cast<PyVar*>(_keys + _capacity)[i];
    }

    inline const PyVar& value(uint16_t i) const {
        return reinterpret_cast<const PyVar*>(_keys + _capacity)[i];
    }

    NameDict(uint16_t capacity=2, float load_factor=0.67, uint16_t hash_seed=kHashSeeds[0]):
        _capacity(capacity), _size(0), _load_factor(load_factor),
        _hash_seed(hash_seed), _mask(capacity-1) {
            _keys = _dict_pool.alloc(capacity);
        }

    NameDict(const NameDict& other) {
        memcpy(this, &other, sizeof(NameDict));
        _keys = _dict_pool.alloc(_capacity);
        for(int i=0; i<_capacity; i++){
            _keys[i] = other._keys[i];
            value(i) = other.value(i);
        }
    }

    NameDict& operator=(const NameDict& other) {
        _dict_pool.dealloc(_keys, _capacity);
        memcpy(this, &other, sizeof(NameDict));
        _keys = _dict_pool.alloc(_capacity);
        for(int i=0; i<_capacity; i++){
            _keys[i] = other._keys[i];
            value(i) = other.value(i);
        }
        return *this;
    }
    
    ~NameDict(){ _dict_pool.dealloc(_keys, _capacity); }

    NameDict(NameDict&&) = delete;
    NameDict& operator=(NameDict&&) = delete;
    uint16_t size() const { return _size; }

#define HASH_PROBE(key, ok, i) \
ok = false; \
i = _hash(key, _mask, _hash_seed); \
while(!_keys[i].empty()) { \
    if(_keys[i] == (key)) { ok = true; break; } \
    i = (i + 1) & _mask; \
}

    const PyVar& operator[](StrName key) const {
        bool ok; uint16_t i;
        HASH_PROBE(key, ok, i);
        if(!ok) throw std::out_of_range("NameDict key not found: " + key.str());
        return value(i);
    }

    PyVar& get(StrName key){
        bool ok; uint16_t i;
        HASH_PROBE(key, ok, i);
        if(!ok) throw std::out_of_range("NameDict key not found: " + key.str());
        return value(i);
    }

    template<typename T>
    void set(StrName key, T&& val){
        bool ok; uint16_t i;
        HASH_PROBE(key, ok, i);
        if(!ok) {
            _size++;
            if(_size > _capacity*_load_factor){
                _rehash(true);
                HASH_PROBE(key, ok, i);
            }
            _keys[i] = key;
        }
        value(i) = std::forward<T>(val);
    }

    void _rehash(bool resize){
        StrName* old_keys = _keys;
        PyVar* old_values = &value(0);
        uint16_t old_capacity = _capacity;
        if(resize){
            _capacity = find_next_capacity(_capacity * 2);
            _mask = _capacity - 1;
        }
        _keys = _dict_pool.alloc(_capacity);
        for(uint16_t i=0; i<old_capacity; i++){
            if(old_keys[i].empty()) continue;
            bool ok; uint16_t j;
            HASH_PROBE(old_keys[i], ok, j);
            if(ok) UNREACHABLE();
            _keys[j] = old_keys[i];
            value(j) = old_values[i]; // std::move makes a segfault
        }
        _dict_pool.dealloc(old_keys, old_capacity);
    }

    void _try_perfect_rehash(){
        _hash_seed = find_perfect_hash_seed(_capacity, keys());
        _rehash(false); // do not resize
    }

    inline PyVar* try_get(StrName key){
        bool ok; uint16_t i;
        HASH_PROBE(key, ok, i);
        if(!ok) return nullptr;
        return &value(i);
    }

    inline bool try_set(StrName key, PyVar&& val){
        bool ok; uint16_t i;
        HASH_PROBE(key, ok, i);
        if(!ok) return false;
        value(i) = std::move(val);
        return true;
    }

    inline bool contains(StrName key) const {
        bool ok; uint16_t i;
        HASH_PROBE(key, ok, i);
        return ok;
    }

    void update(const NameDict& other){
        for(uint16_t i=0; i<other._capacity; i++){
            if(other._keys[i].empty()) continue;
            set(other._keys[i], other.value(i));
        }
    }

    void erase(StrName key){
        bool ok; uint16_t i;
        HASH_PROBE(key, ok, i);
        if(!ok) throw std::out_of_range("NameDict key not found: " + key.str());
        _keys[i] = StrName(); value(i).reset();
        _size--;
    }

    std::vector<std::pair<StrName, PyVar>> items() const {
        std::vector<std::pair<StrName, PyVar>> v;
        for(uint16_t i=0; i<_capacity; i++){
            if(_keys[i].empty()) continue;
            v.push_back(std::make_pair(_keys[i], value(i)));
        }
        return v;
    }

    std::vector<StrName> keys() const {
        std::vector<StrName> v;
        for(uint16_t i=0; i<_capacity; i++){
            if(_keys[i].empty()) continue;
            v.push_back(_keys[i]);
        }
        return v;
    }
#undef HASH_PROBE
#undef _hash
};

} // namespace pkpy


namespace pkpy{

struct NeedMoreLines {
    NeedMoreLines(bool is_compiling_class) : is_compiling_class(is_compiling_class) {}
    bool is_compiling_class;
};

struct HandledException {};
struct UnhandledException {};
struct ToBeRaisedException {};

enum CompileMode {
    EXEC_MODE,
    EVAL_MODE,
    REPL_MODE,
    JSON_MODE,
};

struct SourceData {
    const char* source;
    Str filename;
    std::vector<const char*> line_starts;
    CompileMode mode;

    std::pair<const char*,const char*> get_line(int lineno) const {
        if(lineno == -1) return {nullptr, nullptr};
        lineno -= 1;
        if(lineno < 0) lineno = 0;
        const char* _start = line_starts.at(lineno);
        const char* i = _start;
        while(*i != '\n' && *i != '\0') i++;
        return {_start, i};
    }

    SourceData(const char* source, Str filename, CompileMode mode) {
        source = strdup(source);
        // Skip utf8 BOM if there is any.
        if (strncmp(source, "\xEF\xBB\xBF", 3) == 0) source += 3;
        this->filename = filename;
        this->source = source;
        line_starts.push_back(source);
        this->mode = mode;
    }

    Str snapshot(int lineno, const char* cursor=nullptr){
        StrStream ss;
        ss << "  " << "File \"" << filename << "\", line " << lineno << '\n';
        std::pair<const char*,const char*> pair = get_line(lineno);
        Str line = "<?>";
        int removed_spaces = 0;
        if(pair.first && pair.second){
            line = Str(pair.first, pair.second-pair.first).lstrip();
            removed_spaces = pair.second - pair.first - line.size();
            if(line.empty()) line = "<?>";
        }
        ss << "    " << line;
        if(cursor && line != "<?>" && cursor >= pair.first && cursor <= pair.second){
            auto column = cursor - pair.first - removed_spaces;
            if(column >= 0) ss << "\n    " << std::string(column, ' ') << "^";
        }
        return ss.str();
    }

    ~SourceData() { free((void*)source); }
};

class Exception {
    StrName type;
    Str msg;
    std::stack<Str> stacktrace;
public:
    Exception(StrName type, Str msg): type(type), msg(msg) {}
    bool match_type(StrName type) const { return this->type == type;}
    bool is_re = true;

    void st_push(Str snapshot){
        if(stacktrace.size() >= 8) return;
        stacktrace.push(snapshot);
    }

    Str summary() const {
        std::stack<Str> st(stacktrace);
        StrStream ss;
        if(is_re) ss << "Traceback (most recent call last):\n";
        while(!st.empty()) { ss << st.top() << '\n'; st.pop(); }
        if (!msg.empty()) ss << type.str() << ": " << msg;
        else ss << type.str();
        return ss.str();
    }
};

}   // namespace pkpy


#include <type_traits>

namespace pkpy {
    
struct CodeObject;
struct Frame;
struct BaseRef;
class VM;

typedef std::function<PyVar(VM*, Args&)> NativeFuncRaw;
typedef shared_ptr<CodeObject> CodeObject_;
typedef shared_ptr<NameDict> NameDict_;

struct NativeFunc {
    NativeFuncRaw f;
    int argc;       // DONOT include self
    bool method;
    
    NativeFunc(NativeFuncRaw f, int argc, bool method) : f(f), argc(argc), method(method) {}
    inline PyVar operator()(VM* vm, Args& args) const;
};

struct Function {
    StrName name;
    CodeObject_ code;
    std::vector<StrName> args;
    StrName starred_arg;                // empty if no *arg
    NameDict kwargs;              // empty if no k=v
    std::vector<StrName> kwargs_order;

    // runtime settings
    PyVar _module = nullptr;
    NameDict_ _closure = nullptr;

    bool has_name(const Str& val) const {
        bool _0 = std::find(args.begin(), args.end(), val) != args.end();
        bool _1 = starred_arg == val;
        bool _2 = kwargs.contains(val);
        return _0 || _1 || _2;
    }
};

struct BoundMethod {
    PyVar obj;
    PyVar method;
    BoundMethod(const PyVar& obj, const PyVar& method) : obj(obj), method(method) {}
};

struct Range {
    i64 start = 0;
    i64 stop = -1;
    i64 step = 1;
};

struct StarWrapper {
    PyVar obj;
    bool rvalue;
    StarWrapper(const PyVar& obj, bool rvalue): obj(obj), rvalue(rvalue) {}
};

struct Slice {
    int start = 0;
    int stop = 0x7fffffff; 

    void normalize(int len){
        if(start < 0) start += len;
        if(stop < 0) stop += len;
        if(start < 0) start = 0;
        if(stop > len) stop = len;
        if(stop < start) stop = start;
    }
};

class BaseIter {
protected:
    VM* vm;
    PyVar _ref;     // keep a reference to the object so it will not be deleted while iterating
public:
    virtual PyVar next() = 0;
    PyVarRef loop_var;
    BaseIter(VM* vm, PyVar _ref) : vm(vm), _ref(_ref) {}
    virtual ~BaseIter() = default;
};

struct PyObject {
    Type type;
    NameDict* _attr;

    inline bool is_attr_valid() const noexcept { return _attr != nullptr; }
    inline NameDict& attr() noexcept { return *_attr; }
    inline const PyVar& attr(StrName name) const noexcept { return _attr->get(name); }
    virtual void* value() = 0;

    PyObject(Type type) : type(type) {}
    virtual ~PyObject() { delete _attr; }
};

template <typename T>
struct Py_ : PyObject {
    T _value;

    Py_(Type type, const T& val): PyObject(type), _value(val) { _init(); }
    Py_(Type type, T&& val): PyObject(type), _value(std::move(val)) { _init(); }

    inline void _init() noexcept {
        if constexpr (std::is_same_v<T, Type> || std::is_same_v<T, DummyModule>) {
            _attr = new NameDict(16, kTypeAttrLoadFactor);
        }else if constexpr(std::is_same_v<T, DummyInstance>){
            _attr = new NameDict(4, kInstAttrLoadFactor);
        }else if constexpr(std::is_same_v<T, Function> || std::is_same_v<T, NativeFunc>){
            _attr = new NameDict(4, kInstAttrLoadFactor);
        }else{
            _attr = nullptr;
        }
    }
    void* value() override { return &_value; }
};

#define OBJ_GET(T, obj) (((Py_<T>*)((obj).get()))->_value)
#define OBJ_NAME(obj) OBJ_GET(Str, (obj)->attr(__name__))

const int kTpIntIndex = 2;
const int kTpFloatIndex = 3;

inline bool is_type(const PyVar& obj, Type type) noexcept {
    switch(type.index){
        case kTpIntIndex: return obj.is_tag_01();
        case kTpFloatIndex: return obj.is_tag_10();
        default: return !obj.is_tagged() && obj->type == type;
    }
}

inline bool is_both_int_or_float(const PyVar& a, const PyVar& b) noexcept {
    return ((a.bits | b.bits) & 0b11) != 0b00;
}

inline bool is_both_int(const PyVar& a, const PyVar& b) noexcept {
    return (a.bits & b.bits & 0b11) == 0b01;
}

inline bool is_int(const PyVar& obj) noexcept {
    return obj.is_tag_01();
}

inline bool is_float(const PyVar& obj) noexcept {
    return obj.is_tag_10();
}

#define PY_CLASS(T, mod, name) \
    static Type _type(VM* vm) {  \
        static StrName __x0(#mod);      \
        static StrName __x1(#name);     \
        return OBJ_GET(Type, vm->_modules[__x0]->attr(__x1));               \
    }                                                                       \
    static PyVar register_class(VM* vm, PyVar mod) {                        \
        PyVar type = vm->new_type_object(mod, #name, vm->_t(vm->tp_object));\
        if(OBJ_NAME(mod) != #mod) UNREACHABLE();                            \
        T::_register(vm, mod, type);                                        \
        type->attr()._try_perfect_rehash();                                 \
        return type;                                                        \
    }                                                                       

union __8B {
    i64 _int;
    f64 _float;
    __8B(i64 val) : _int(val) {}
    __8B(f64 val) : _float(val) {}
};

template <typename, typename = void> struct is_py_class : std::false_type {};
template <typename T> struct is_py_class<T, std::void_t<decltype(T::_type)>> : std::true_type {};

template<typename T>
void _check_py_class(VM* vm, const PyVar& var);

template<typename T>
T py_pointer_cast(VM* vm, const PyVar& var);

template<typename T>
T py_value_cast(VM* vm, const PyVar& var);

struct Discarded {};

template<typename __T>
__T py_cast(VM* vm, const PyVar& obj) {
    using T = std::decay_t<__T>;
    if constexpr(std::is_pointer_v<T>){
        return py_pointer_cast<T>(vm, obj);
    }else if constexpr(is_py_class<T>::value){
        _check_py_class<T>(vm, obj);
        return OBJ_GET(T, obj);
    }else if constexpr(std::is_pod_v<T>){
        return py_value_cast<T>(vm, obj);
    }else{
        return Discarded();
    }
}

template<typename __T>
__T _py_cast(VM* vm, const PyVar& obj) {
    using T = std::decay_t<__T>;
    if constexpr(std::is_pointer_v<__T>){
        return py_pointer_cast<__T>(vm, obj);
    }else if constexpr(is_py_class<T>::value){
        return OBJ_GET(T, obj);
    }else{
        return Discarded();
    }
}

#define VAR(x) py_var(vm, x)
#define VAR_T(T, ...) vm->new_object(T::_type(vm), T(__VA_ARGS__))
#define CAST(T, x) py_cast<T>(vm, x)
#define _CAST(T, x) _py_cast<T>(vm, x)

}   // namespace pkpy


namespace pkpy{

typedef uint8_t TokenIndex;

constexpr const char* kTokens[] = {
    "@error", "@eof", "@eol", "@sof",
    ".", ",", ":", ";", "#", "(", ")", "[", "]", "{", "}", "%", "::",
    "+", "-", "*", "/", "//", "**", "=", ">", "<", "...", "->",
    "<<", ">>", "&", "|", "^", "?", "@",
    "==", "!=", ">=", "<=",
    "+=", "-=", "*=", "/=", "//=", "%=", "&=", "|=", "^=", ">>=", "<<=",
    /** KW_BEGIN **/
    "class", "import", "as", "def", "lambda", "pass", "del", "from", "with", "yield",
    "None", "in", "is", "and", "or", "not", "True", "False", "global", "try", "except", "finally",
    "goto", "label",      // extended keywords, not available in cpython
    "while", "for", "if", "elif", "else", "break", "continue", "return", "assert", "raise",
    /** KW_END **/
    "is not", "not in",
    "@id", "@num", "@str", "@fstr",
    "@indent", "@dedent"
};

const TokenIndex kTokenCount = sizeof(kTokens) / sizeof(kTokens[0]);

constexpr TokenIndex TK(const char token[]) {
    for(int k=0; k<kTokenCount; k++){
        const char* i = kTokens[k];
        const char* j = token;
        while(*i && *j && *i == *j) { i++; j++;}
        if(*i == *j) return k;
    }
    UNREACHABLE();
}

#define TK_STR(t) kTokens[t]
const TokenIndex kTokenKwBegin = TK("class");
const TokenIndex kTokenKwEnd = TK("raise");

const std::map<std::string_view, TokenIndex> kTokenKwMap = [](){
    std::map<std::string_view, TokenIndex> map;
    for(int k=kTokenKwBegin; k<=kTokenKwEnd; k++) map[kTokens[k]] = k;
    return map;
}();


struct Token{
  TokenIndex type;

  const char* start;
  int length;
  int line;
  PyVar value;

  Str str() const { return Str(start, length);}

  Str info() const {
    StrStream ss;
    Str raw = str();
    if (raw == Str("\n")) raw = "\\n";
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
    shared_ptr<SourceData> src;

    const char* token_start;
    const char* curr_char;
    int current_line = 1;
    Token prev, curr;
    std::queue<Token> nexts;
    std::stack<int> indents;

    int brackets_level = 0;

    Token next_token(){
        if(nexts.empty()){
            return Token{TK("@error"), token_start, (int)(curr_char - token_start), current_line};
        }
        Token t = nexts.front();
        if(t.type == TK("@eof") && indents.size()>1){
            nexts.pop();
            indents.pop();
            return Token{TK("@dedent"), token_start, 0, current_line};
        }
        nexts.pop();
        return t;
    }

    inline char peekchar() const{ return *curr_char; }

    bool match_n_chars(int n, char c0){
        const char* c = curr_char;
        for(int i=0; i<n; i++){
            if(*c == '\0') return false;
            if(*c != c0) return false;
            c++;
        }
        for(int i=0; i<n; i++) eatchar_include_newline();
        return true;
    }

    int eat_spaces(){
        int count = 0;
        while (true) {
            switch (peekchar()) {
                case ' ' : count+=1; break;
                case '\t': count+=4; break;
                default: return count;
            }
            eatchar();
        }
    }

    bool eat_indentation(){
        if(brackets_level > 0) return true;
        int spaces = eat_spaces();
        if(peekchar() == '#') skip_line_comment();
        if(peekchar() == '\0' || peekchar() == '\n' || peekchar() == '\r') return true;
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

    char eatchar() {
        char c = peekchar();
        if(c == '\n') throw std::runtime_error("eatchar() cannot consume a newline");
        curr_char++;
        return c;
    }

    char eatchar_include_newline() {
        char c = peekchar();
        curr_char++;
        if (c == '\n'){
            current_line++;
            src->line_starts.push_back(curr_char);
        }
        return c;
    }

    int eat_name() {
        curr_char--;
        while(true){
            uint8_t c = peekchar();
            int u8bytes = 0;
            if((c & 0b10000000) == 0b00000000) u8bytes = 1;
            else if((c & 0b11100000) == 0b11000000) u8bytes = 2;
            else if((c & 0b11110000) == 0b11100000) u8bytes = 3;
            else if((c & 0b11111000) == 0b11110000) u8bytes = 4;
            else return 1;
            if(u8bytes == 1){
                if(isalpha(c) || c=='_' || isdigit(c)) {
                    curr_char++;
                    continue;
                }else{
                    break;
                }
            }
            // handle multibyte char
            std::string u8str(curr_char, u8bytes);
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
            if(is_unicode_Lo_char(value)) curr_char += u8bytes;
            else break;
        }

        int length = (int)(curr_char - token_start);
        if(length == 0) return 3;
        std::string_view name(token_start, length);

        if(src->mode == JSON_MODE){
            if(name == "true"){
                set_next_token(TK("True"));
            } else if(name == "false"){
                set_next_token(TK("False"));
            } else if(name == "null"){
                set_next_token(TK("None"));
            } else {
                return 4;
            }
            return 0;
        }

        if(kTokenKwMap.count(name)){
            if(name == "not"){
                if(strncmp(curr_char, " in", 3) == 0){
                    curr_char += 3;
                    set_next_token(TK("not in"));
                    return 0;
                }
            }else if(name == "is"){
                if(strncmp(curr_char, " not", 4) == 0){
                    curr_char += 4;
                    set_next_token(TK("is not"));
                    return 0;
                }
            }
            set_next_token(kTokenKwMap.at(name));
        } else {
            set_next_token(TK("@id"));
        }
        return 0;
    }

    void skip_line_comment() {
        char c;
        while ((c = peekchar()) != '\0') {
            if (c == '\n') return;
            eatchar();
        }
    }
    
    bool matchchar(char c) {
        if (peekchar() != c) return false;
        eatchar_include_newline();
        return true;
    }

    void set_next_token(TokenIndex type, PyVar value=nullptr) {
        switch(type){
            case TK("{"): case TK("["): case TK("("): brackets_level++; break;
            case TK(")"): case TK("]"): case TK("}"): brackets_level--; break;
        }
        nexts.push( Token{
            type,
            token_start,
            (int)(curr_char - token_start),
            current_line - ((type == TK("@eol")) ? 1 : 0),
            value
        });
    }

    void set_next_token_2(char c, TokenIndex one, TokenIndex two) {
        if (matchchar(c)) set_next_token(two);
        else set_next_token(one);
    }

    Parser(shared_ptr<SourceData> src) {
        this->src = src;
        this->token_start = src->source;
        this->curr_char = src->source;
        this->nexts.push(Token{TK("@sof"), token_start, 0, current_line});
        this->indents.push(0);
    }
};

} // namespace pkpy


namespace pkpy{

enum NameScope {
    NAME_LOCAL = 0,
    NAME_GLOBAL,
    NAME_ATTR,
    NAME_SPECIAL,
};

enum Opcode {
    #define OPCODE(name) OP_##name,
    #ifdef OPCODE

OPCODE(NO_OP)
OPCODE(POP_TOP)
OPCODE(DUP_TOP_VALUE)
OPCODE(CALL)
OPCODE(CALL_UNPACK)
OPCODE(CALL_KWARGS)
OPCODE(CALL_KWARGS_UNPACK)
OPCODE(RETURN_VALUE)
OPCODE(ROT_TWO)

OPCODE(BINARY_OP)
OPCODE(COMPARE_OP)
OPCODE(BITWISE_OP)
OPCODE(IS_OP)
OPCODE(CONTAINS_OP)

OPCODE(UNARY_NEGATIVE)
OPCODE(UNARY_NOT)
OPCODE(UNARY_STAR)

OPCODE(BUILD_LIST)
OPCODE(BUILD_MAP)
OPCODE(BUILD_SET)
OPCODE(BUILD_SLICE)
OPCODE(BUILD_TUPLE)
OPCODE(BUILD_TUPLE_REF)
OPCODE(BUILD_STRING)

OPCODE(LIST_APPEND)
OPCODE(IMPORT_NAME)
OPCODE(PRINT_EXPR)

OPCODE(GET_ITER)
OPCODE(FOR_ITER)

OPCODE(WITH_ENTER)
OPCODE(WITH_EXIT)
OPCODE(LOOP_BREAK)
OPCODE(LOOP_CONTINUE)

OPCODE(POP_JUMP_IF_FALSE)
OPCODE(JUMP_ABSOLUTE)
OPCODE(SAFE_JUMP_ABSOLUTE)
OPCODE(JUMP_IF_TRUE_OR_POP)
OPCODE(JUMP_IF_FALSE_OR_POP)

OPCODE(GOTO)

OPCODE(LOAD_CONST)
OPCODE(LOAD_NONE)
OPCODE(LOAD_TRUE)
OPCODE(LOAD_FALSE)
OPCODE(LOAD_EVAL_FN)
OPCODE(LOAD_FUNCTION)
OPCODE(LOAD_ELLIPSIS)
OPCODE(LOAD_NAME)
OPCODE(LOAD_NAME_REF)

OPCODE(ASSERT)
OPCODE(EXCEPTION_MATCH)
OPCODE(RAISE)
OPCODE(RE_RAISE)

OPCODE(BUILD_INDEX)
OPCODE(BUILD_ATTR)
OPCODE(BUILD_ATTR_REF)
OPCODE(STORE_NAME)
OPCODE(STORE_FUNCTION)
OPCODE(STORE_REF)
OPCODE(DELETE_REF)

OPCODE(TRY_BLOCK_ENTER)
OPCODE(TRY_BLOCK_EXIT)

OPCODE(YIELD_VALUE)

OPCODE(FAST_INDEX)      // a[x]
OPCODE(FAST_INDEX_REF)       // a[x]

OPCODE(INPLACE_BINARY_OP)
OPCODE(INPLACE_BITWISE_OP)

OPCODE(SETUP_CLOSURE)
OPCODE(SETUP_DECORATOR)
OPCODE(STORE_ALL_NAMES)

OPCODE(BEGIN_CLASS)
OPCODE(END_CLASS)
OPCODE(STORE_CLASS_ATTR)

#endif
    #undef OPCODE
};

static const char* OP_NAMES[] = {
    #define OPCODE(name) #name,
    #ifdef OPCODE

OPCODE(NO_OP)
OPCODE(POP_TOP)
OPCODE(DUP_TOP_VALUE)
OPCODE(CALL)
OPCODE(CALL_UNPACK)
OPCODE(CALL_KWARGS)
OPCODE(CALL_KWARGS_UNPACK)
OPCODE(RETURN_VALUE)
OPCODE(ROT_TWO)

OPCODE(BINARY_OP)
OPCODE(COMPARE_OP)
OPCODE(BITWISE_OP)
OPCODE(IS_OP)
OPCODE(CONTAINS_OP)

OPCODE(UNARY_NEGATIVE)
OPCODE(UNARY_NOT)
OPCODE(UNARY_STAR)

OPCODE(BUILD_LIST)
OPCODE(BUILD_MAP)
OPCODE(BUILD_SET)
OPCODE(BUILD_SLICE)
OPCODE(BUILD_TUPLE)
OPCODE(BUILD_TUPLE_REF)
OPCODE(BUILD_STRING)

OPCODE(LIST_APPEND)
OPCODE(IMPORT_NAME)
OPCODE(PRINT_EXPR)

OPCODE(GET_ITER)
OPCODE(FOR_ITER)

OPCODE(WITH_ENTER)
OPCODE(WITH_EXIT)
OPCODE(LOOP_BREAK)
OPCODE(LOOP_CONTINUE)

OPCODE(POP_JUMP_IF_FALSE)
OPCODE(JUMP_ABSOLUTE)
OPCODE(SAFE_JUMP_ABSOLUTE)
OPCODE(JUMP_IF_TRUE_OR_POP)
OPCODE(JUMP_IF_FALSE_OR_POP)

OPCODE(GOTO)

OPCODE(LOAD_CONST)
OPCODE(LOAD_NONE)
OPCODE(LOAD_TRUE)
OPCODE(LOAD_FALSE)
OPCODE(LOAD_EVAL_FN)
OPCODE(LOAD_FUNCTION)
OPCODE(LOAD_ELLIPSIS)
OPCODE(LOAD_NAME)
OPCODE(LOAD_NAME_REF)

OPCODE(ASSERT)
OPCODE(EXCEPTION_MATCH)
OPCODE(RAISE)
OPCODE(RE_RAISE)

OPCODE(BUILD_INDEX)
OPCODE(BUILD_ATTR)
OPCODE(BUILD_ATTR_REF)
OPCODE(STORE_NAME)
OPCODE(STORE_FUNCTION)
OPCODE(STORE_REF)
OPCODE(DELETE_REF)

OPCODE(TRY_BLOCK_ENTER)
OPCODE(TRY_BLOCK_EXIT)

OPCODE(YIELD_VALUE)

OPCODE(FAST_INDEX)      // a[x]
OPCODE(FAST_INDEX_REF)       // a[x]

OPCODE(INPLACE_BINARY_OP)
OPCODE(INPLACE_BITWISE_OP)

OPCODE(SETUP_CLOSURE)
OPCODE(SETUP_DECORATOR)
OPCODE(STORE_ALL_NAMES)

OPCODE(BEGIN_CLASS)
OPCODE(END_CLASS)
OPCODE(STORE_CLASS_ATTR)

#endif
    #undef OPCODE
};

struct Bytecode{
    uint8_t op;
    int arg;
    int line;
    uint16_t block;
};

Str pad(const Str& s, const int n){
    if(s.size() >= n) return s.substr(0, n);
    return s + std::string(n - s.size(), ' ');
}

enum CodeBlockType {
    NO_BLOCK,
    FOR_LOOP,
    WHILE_LOOP,
    CONTEXT_MANAGER,
    TRY_EXCEPT,
};

struct CodeBlock {
    CodeBlockType type;
    int parent;         // parent index in blocks
    int start;          // start index of this block in codes, inclusive
    int end;            // end index of this block in codes, exclusive

    std::string to_string() const {
        if(parent == -1) return "";
        return "[B:" + std::to_string(type) + "]";
    }
};

struct CodeObject {
    shared_ptr<SourceData> src;
    Str name;
    bool is_generator = false;

    CodeObject(shared_ptr<SourceData> src, Str name) {
        this->src = src;
        this->name = name;
    }

    std::vector<Bytecode> codes;
    List consts;
    std::vector<std::pair<StrName, NameScope>> names;
    std::map<StrName, int> global_names;
    std::vector<CodeBlock> blocks = { CodeBlock{NO_BLOCK, -1} };
    std::map<StrName, int> labels;

    uint32_t perfect_locals_capacity = 2;
    uint32_t perfect_hash_seed = 0;

    void optimize(VM* vm);

    bool add_label(StrName label){
        if(labels.count(label)) return false;
        labels[label] = codes.size();
        return true;
    }

    int add_name(StrName name, NameScope scope){
        if(scope == NAME_LOCAL && global_names.count(name)) scope = NAME_GLOBAL;
        auto p = std::make_pair(name, scope);
        for(int i=0; i<names.size(); i++){
            if(names[i] == p) return i;
        }
        names.push_back(p);
        return names.size() - 1;
    }

    int add_const(PyVar v){
        consts.push_back(v);
        return consts.size() - 1;
    }

    /************************************************/
    int _curr_block_i = 0;
    int _rvalue = 0;
    bool _is_compiling_class = false;
    bool _is_curr_block_loop() const {
        return blocks[_curr_block_i].type == FOR_LOOP || blocks[_curr_block_i].type == WHILE_LOOP;
    }

    void _enter_block(CodeBlockType type){
        blocks.push_back(CodeBlock{type, _curr_block_i, (int)codes.size()});
        _curr_block_i = blocks.size()-1;
    }

    void _exit_block(){
        blocks[_curr_block_i].end = codes.size();
        _curr_block_i = blocks[_curr_block_i].parent;
        if(_curr_block_i < 0) UNREACHABLE();
    }
    /************************************************/
};


} // namespace pkpy


namespace pkpy{

static THREAD_LOCAL uint64_t kFrameGlobalId = 0;

struct Frame {
    std::vector<PyVar> _data;
    int _ip = -1;
    int _next_ip = 0;

    const CodeObject* co;
    PyVar _module;
    NameDict_ _locals;
    NameDict_ _closure;
    const uint64_t id;
    std::vector<std::pair<int, std::vector<PyVar>>> s_try_block;

    inline NameDict& f_locals() noexcept { return _locals != nullptr ? *_locals : _module->attr(); }
    inline NameDict& f_globals() noexcept { return _module->attr(); }

    inline PyVar* f_closure_try_get(StrName name) noexcept {
        if(_closure == nullptr) return nullptr;
        return _closure->try_get(name);
    }

    Frame(const CodeObject_& co,
        const PyVar& _module,
        const NameDict_& _locals=nullptr,
        const NameDict_& _closure=nullptr)
            : co(co.get()), _module(_module), _locals(_locals), _closure(_closure), id(kFrameGlobalId++) { }

    inline const Bytecode& next_bytecode() {
        _ip = _next_ip++;
        return co->codes[_ip];
    }

    Str snapshot(){
        int line = co->codes[_ip].line;
        return co->src->snapshot(line);
    }

    // Str stack_info(){
    //     StrStream ss;
    //     ss << "[";
    //     for(int i=0; i<_data.size(); i++){
    //         ss << OBJ_TP_NAME(_data[i]);
    //         if(i != _data.size()-1) ss << ", ";
    //     }
    //     ss << "]";
    //     return ss.str();
    // }

    inline bool has_next_bytecode() const {
        return _next_ip < co->codes.size();
    }

    inline PyVar pop(){
#if PK_EXTRA_CHECK
        if(_data.empty()) throw std::runtime_error("_data.empty() is true");
#endif
        PyVar v = std::move(_data.back());
        _data.pop_back();
        return v;
    }

    inline void _pop(){
#if PK_EXTRA_CHECK
        if(_data.empty()) throw std::runtime_error("_data.empty() is true");
#endif
        _data.pop_back();
    }

    inline void try_deref(VM*, PyVar&);

    inline PyVar pop_value(VM* vm){
        PyVar value = pop();
        try_deref(vm, value);
        return value;
    }

    inline PyVar top_value(VM* vm){
        PyVar value = top();
        try_deref(vm, value);
        return value;
    }

    inline PyVar& top(){
#if PK_EXTRA_CHECK
        if(_data.empty()) throw std::runtime_error("_data.empty() is true");
#endif
        return _data.back();
    }

    inline PyVar& top_1(){
#if PK_EXTRA_CHECK
        if(_data.size() < 2) throw std::runtime_error("_data.size() < 2");
#endif
        return _data[_data.size()-2];
    }

    template<typename T>
    inline void push(T&& obj){ _data.push_back(std::forward<T>(obj)); }

    inline void jump_abs(int i){ _next_ip = i; }
    inline void jump_rel(int i){ _next_ip += i; }

    inline void on_try_block_enter(){
        s_try_block.emplace_back(co->codes[_ip].block, _data);
    }

    inline void on_try_block_exit(){
        s_try_block.pop_back();
    }

    bool jump_to_exception_handler(){
        if(s_try_block.empty()) return false;
        PyVar obj = pop();
        auto& p = s_try_block.back();
        _data = std::move(p.second);
        _data.push_back(obj);
        _next_ip = co->blocks[p.first].end;
        on_try_block_exit();
        return true;
    }

    int _exit_block(int i){
        if(co->blocks[i].type == FOR_LOOP) _pop();
        else if(co->blocks[i].type == TRY_EXCEPT) on_try_block_exit();
        return co->blocks[i].parent;
    }

    void jump_abs_safe(int target){
        const Bytecode& prev = co->codes[_ip];
        int i = prev.block;
        _next_ip = target;
        if(_next_ip >= co->codes.size()){
            while(i>=0) i = _exit_block(i);
        }else{
            const Bytecode& next = co->codes[target];
            while(i>=0 && i!=next.block) i = _exit_block(i);
            if(i!=next.block) throw std::runtime_error("invalid jump");
        }
    }

    Args pop_n_values_reversed(VM* vm, int n){
        Args v(n);
        for(int i=n-1; i>=0; i--){
            v[i] = pop();
            try_deref(vm, v[i]);
        }
        return v;
    }

    Args pop_n_reversed(int n){
        Args v(n);
        for(int i=n-1; i>=0; i--) v[i] = pop();
        return v;
    }
};

}; // namespace pkpy


namespace pkpy{

#define DEF_NATIVE_2(ctype, ptype)                                      \
    template<> ctype py_cast<ctype>(VM* vm, const PyVar& obj) {         \
        vm->check_type(obj, vm->ptype);                                 \
        return OBJ_GET(ctype, obj);                                     \
    }                                                                   \
    template<> ctype _py_cast<ctype>(VM* vm, const PyVar& obj) {        \
        return OBJ_GET(ctype, obj);                                     \
    }                                                                   \
    template<> ctype& py_cast<ctype&>(VM* vm, const PyVar& obj) {       \
        vm->check_type(obj, vm->ptype);                                 \
        return OBJ_GET(ctype, obj);                                     \
    }                                                                   \
    template<> ctype& _py_cast<ctype&>(VM* vm, const PyVar& obj) {      \
        return OBJ_GET(ctype, obj);                                     \
    }                                                                   \
    PyVar py_var(VM* vm, const ctype& value) { return vm->new_object(vm->ptype, value);}     \
    PyVar py_var(VM* vm, ctype&& value) { return vm->new_object(vm->ptype, std::move(value));}

class Generator: public BaseIter {
    std::unique_ptr<Frame> frame;
    int state; // 0,1,2
public:
    Generator(VM* vm, std::unique_ptr<Frame>&& frame)
        : BaseIter(vm, nullptr), frame(std::move(frame)), state(0) {}

    PyVar next();
};

class VM {
    VM* vm;     // self reference for simplify code
public:
    std::stack< std::unique_ptr<Frame> > callstack;
    PyVar _py_op_call;
    PyVar _py_op_yield;
    std::vector<PyVar> _all_types;

    PyVar run_frame(Frame* frame);

    NameDict _types;
    NameDict _modules;                            // loaded modules
    std::map<StrName, Str> _lazy_modules;       // lazy loaded modules
    PyVar None, True, False, Ellipsis;

    bool use_stdio;
    std::ostream* _stdout;
    std::ostream* _stderr;
    
    PyVar builtins;         // builtins module
    PyVar _main;            // __main__ module

    int recursionlimit = 1000;

    VM(bool use_stdio){
        this->vm = this;
        this->use_stdio = use_stdio;
        if(use_stdio){
            this->_stdout = &std::cout;
            this->_stderr = &std::cerr;
        }else{
            this->_stdout = new StrStream();
            this->_stderr = new StrStream();
        }

        init_builtin_types();
        // for(int i=0; i<128; i++) _ascii_str_pool[i] = new_object(tp_str, std::string(1, (char)i));
    }

    PyVar asStr(const PyVar& obj){
        PyVarOrNull f = getattr(obj, __str__, false, true);
        if(f != nullptr) return call(f);
        return asRepr(obj);
    }

    inline Frame* top_frame() const {
#if PK_EXTRA_CHECK
        if(callstack.empty()) UNREACHABLE();
#endif
        return callstack.top().get();
    }

    PyVar asIter(const PyVar& obj){
        if(is_type(obj, tp_native_iterator)) return obj;
        PyVarOrNull iter_f = getattr(obj, __iter__, false, true);
        if(iter_f != nullptr) return call(iter_f);
        TypeError(OBJ_NAME(_t(obj)).escape(true) + " object is not iterable");
        return nullptr;
    }

    PyVar asList(const PyVar& iterable){
        if(is_type(iterable, tp_list)) return iterable;
        return call(_t(tp_list), one_arg(iterable));
    }

    PyVar fast_call(StrName name, Args&& args){
        PyObject* cls = _t(args[0]).get();
        while(cls != None.get()) {
            PyVar* val = cls->attr().try_get(name);
            if(val != nullptr) return call(*val, std::move(args));
            cls = cls->attr(__base__).get();
        }
        AttributeError(args[0], name);
        return nullptr;
    }

    inline PyVar call(const PyVar& _callable){
        return call(_callable, no_arg(), no_arg(), false);
    }

    template<typename ArgT>
    inline std::enable_if_t<std::is_same_v<std::decay_t<ArgT>, Args>, PyVar>
    call(const PyVar& _callable, ArgT&& args){
        return call(_callable, std::forward<ArgT>(args), no_arg(), false);
    }

    template<typename ArgT>
    inline std::enable_if_t<std::is_same_v<std::decay_t<ArgT>, Args>, PyVar>
    call(const PyVar& obj, const StrName name, ArgT&& args){
        return call(getattr(obj, name, true, true), std::forward<ArgT>(args), no_arg(), false);
    }

    inline PyVar call(const PyVar& obj, StrName name){
        return call(getattr(obj, name, true, true), no_arg(), no_arg(), false);
    }


    // repl mode is only for setting `frame->id` to 0
    PyVarOrNull exec(Str source, Str filename, CompileMode mode, PyVar _module=nullptr){
        if(_module == nullptr) _module = _main;
        try {
            CodeObject_ code = compile(source, filename, mode);
            return _exec(code, _module);
        }catch (const Exception& e){
            *_stderr << e.summary() << '\n';
        }catch (const std::exception& e) {
            *_stderr << "An std::exception occurred! It could be a bug.\n";
            *_stderr << e.what() << '\n';
        }
        callstack = {};
        return nullptr;
    }

    template<typename ...Args>
    inline std::unique_ptr<Frame> _new_frame(Args&&... args){
        if(callstack.size() > recursionlimit){
            _error("RecursionError", "maximum recursion depth exceeded");
        }
        return std::make_unique<Frame>(std::forward<Args>(args)...);
    }

    template<typename ...Args>
    inline PyVar _exec(Args&&... args){
        callstack.push(_new_frame(std::forward<Args>(args)...));
        return _exec();
    }

    Type _new_type_object(StrName name, Type base=0) {
        PyVar obj = make_sp<PyObject, Py_<Type>>(tp_type, _all_types.size());
        setattr(obj, __base__, _t(base));
        _types.set(name, obj);
        _all_types.push_back(obj);
        return OBJ_GET(Type, obj);
    }

    template<typename T>
    inline PyVar new_object(const PyVar& type, const T& _value) {
#if PK_EXTRA_CHECK
        if(!is_type(type, tp_type)) UNREACHABLE();
#endif
        return make_sp<PyObject, Py_<std::decay_t<T>>>(OBJ_GET(Type, type), _value);
    }
    template<typename T>
    inline PyVar new_object(const PyVar& type, T&& _value) {
#if PK_EXTRA_CHECK
        if(!is_type(type, tp_type)) UNREACHABLE();
#endif
        return make_sp<PyObject, Py_<std::decay_t<T>>>(OBJ_GET(Type, type), std::move(_value));
    }

    template<typename T>
    inline PyVar new_object(Type type, const T& _value) {
        return make_sp<PyObject, Py_<std::decay_t<T>>>(type, _value);
    }
    template<typename T>
    inline PyVar new_object(Type type, T&& _value) {
        return make_sp<PyObject, Py_<std::decay_t<T>>>(type, std::move(_value));
    }

    template<int ARGC>
    void bind_func(Str typeName, Str funcName, NativeFuncRaw fn) {
        bind_func<ARGC>(_types[typeName], funcName, fn);     
    }

    template<int ARGC>
    void bind_method(Str typeName, Str funcName, NativeFuncRaw fn) {
        bind_method<ARGC>(_types[typeName], funcName, fn);
    }

    template<int ARGC, typename... Args>
    void bind_static_method(Args&&... args) {
        bind_func<ARGC>(std::forward<Args>(args)...);
    }

    template<int ARGC>
    void _bind_methods(std::vector<Str> typeNames, Str funcName, NativeFuncRaw fn) {
        for(auto& typeName : typeNames) bind_method<ARGC>(typeName, funcName, fn);
    }

    template<int ARGC>
    void bind_builtin_func(Str funcName, NativeFuncRaw fn) {
        bind_func<ARGC>(builtins, funcName, fn);
    }

    int normalized_index(int index, int size){
        if(index < 0) index += size;
        if(index < 0 || index >= size){
            IndexError(std::to_string(index) + " not in [0, " + std::to_string(size) + ")");
        }
        return index;
    }

    // for quick access
    Type tp_object, tp_type, tp_int, tp_float, tp_bool, tp_str;
    Type tp_list, tp_tuple;
    Type tp_function, tp_native_function, tp_native_iterator, tp_bound_method;
    Type tp_slice, tp_range, tp_module, tp_ref;
    Type tp_super, tp_exception, tp_star_wrapper;

    template<typename P>
    inline PyVar PyIter(P&& value) {
        static_assert(std::is_base_of_v<BaseIter, std::decay_t<P>>);
        return new_object(tp_native_iterator, std::forward<P>(value));
    }

    inline BaseIter* PyIter_AS_C(const PyVar& obj)
    {
        check_type(obj, tp_native_iterator);
        return static_cast<BaseIter*>(obj->value());
    }
    
    /***** Error Reporter *****/
    void _error(StrName name, const Str& msg){
        _error(Exception(name, msg));
    }

    void _raise(){
        bool ok = top_frame()->jump_to_exception_handler();
        if(ok) throw HandledException();
        else throw UnhandledException();
    }

public:
    void IOError(const Str& msg) { _error("IOError", msg); }
    void NotImplementedError(){ _error("NotImplementedError", ""); }
    void TypeError(const Str& msg){ _error("TypeError", msg); }
    void ZeroDivisionError(){ _error("ZeroDivisionError", "division by zero"); }
    void IndexError(const Str& msg){ _error("IndexError", msg); }
    void ValueError(const Str& msg){ _error("ValueError", msg); }
    void NameError(StrName name){ _error("NameError", "name " + name.str().escape(true) + " is not defined"); }

    void AttributeError(PyVar obj, StrName name){
        _error("AttributeError", "type " +  OBJ_NAME(_t(obj)).escape(true) + " has no attribute " + name.str().escape(true));
    }

    void AttributeError(Str msg){ _error("AttributeError", msg); }

    inline void check_type(const PyVar& obj, Type type){
        if(is_type(obj, type)) return;
        TypeError("expected " + OBJ_NAME(_t(type)).escape(true) + ", but got " + OBJ_NAME(_t(obj)).escape(true));
    }

    inline PyVar& _t(Type t){
        return _all_types[t.index];
    }

    inline PyVar& _t(const PyVar& obj){
        if(is_int(obj)) return _t(tp_int);
        if(is_float(obj)) return _t(tp_float);
        return _all_types[OBJ_GET(Type, _t(obj->type)).index];
    }

    ~VM() {
        if(!use_stdio){
            delete _stdout;
            delete _stderr;
        }
    }

    CodeObject_ compile(Str source, Str filename, CompileMode mode);
    void post_init();
    PyVar num_negated(const PyVar& obj);
    f64 num_to_float(const PyVar& obj);
    const PyVar& asBool(const PyVar& obj);
    i64 hash(const PyVar& obj);
    PyVar asRepr(const PyVar& obj);
    PyVar new_type_object(PyVar mod, StrName name, PyVar base);
    PyVar new_module(StrName name);
    Str disassemble(CodeObject_ co);
    void init_builtin_types();
    PyVar call(const PyVar& _callable, Args args, const Args& kwargs, bool opCall);
    void unpack_args(Args& args);
    PyVarOrNull getattr(const PyVar& obj, StrName name, bool throw_err=true, bool class_only=false);
    template<typename T>
    void setattr(PyVar& obj, StrName name, T&& value);
    template<int ARGC>
    void bind_method(PyVar obj, Str funcName, NativeFuncRaw fn);
    template<int ARGC>
    void bind_func(PyVar obj, Str funcName, NativeFuncRaw fn);
    void _error(Exception e);
    PyVar _exec();

    template<typename P>
    PyVarRef PyRef(P&& value);
    const BaseRef* PyRef_AS_C(const PyVar& obj);
};

PyVar NativeFunc::operator()(VM* vm, Args& args) const{
    int args_size = args.size() - (int)method;  // remove self
    if(argc != -1 && args_size != argc) {
        vm->TypeError("expected " + std::to_string(argc) + " arguments, but got " + std::to_string(args_size));
    }
    return f(vm, args);
}

void CodeObject::optimize(VM* vm){
    std::vector<StrName> keys;
    for(auto& p: names) if(p.second == NAME_LOCAL) keys.push_back(p.first);
    uint32_t base_n = (uint32_t)(keys.size() / kLocalsLoadFactor + 0.5);
    perfect_locals_capacity = find_next_capacity(base_n);
    perfect_hash_seed = find_perfect_hash_seed(perfect_locals_capacity, keys);

    for(int i=1; i<codes.size(); i++){
        if(codes[i].op == OP_UNARY_NEGATIVE && codes[i-1].op == OP_LOAD_CONST){
            codes[i].op = OP_NO_OP;
            int pos = codes[i-1].arg;
            consts[pos] = vm->num_negated(consts[pos]);
        }

        if(i>=2 && codes[i].op == OP_BUILD_INDEX){
            const Bytecode& a = codes[i-1];
            const Bytecode& x = codes[i-2];
            if(codes[i].arg == 1){
                if(a.op == OP_LOAD_NAME && x.op == OP_LOAD_NAME){
                    codes[i].op = OP_FAST_INDEX;
                }else continue;
            }else{
                if(a.op == OP_LOAD_NAME_REF && x.op == OP_LOAD_NAME_REF){
                    codes[i].op = OP_FAST_INDEX_REF;
                }else continue;
            }
            codes[i].arg = (a.arg << 16) | x.arg;
            codes[i-1].op = OP_NO_OP;
            codes[i-2].op = OP_NO_OP;
        }
    }

    // pre-compute sn in co_consts
    for(int i=0; i<consts.size(); i++){
        if(is_type(consts[i], vm->tp_str)){
            Str& s = OBJ_GET(Str, consts[i]);
            s._cached_sn_index = StrName::get(s.c_str()).index;
        }
    }
}

DEF_NATIVE_2(Str, tp_str)
DEF_NATIVE_2(List, tp_list)
DEF_NATIVE_2(Tuple, tp_tuple)
DEF_NATIVE_2(Function, tp_function)
DEF_NATIVE_2(NativeFunc, tp_native_function)
DEF_NATIVE_2(BoundMethod, tp_bound_method)
DEF_NATIVE_2(Range, tp_range)
DEF_NATIVE_2(Slice, tp_slice)
DEF_NATIVE_2(Exception, tp_exception)
DEF_NATIVE_2(StarWrapper, tp_star_wrapper)

#define PY_CAST_INT(T) \
template<> T py_cast<T>(VM* vm, const PyVar& obj){ \
    vm->check_type(obj, vm->tp_int); \
    return (T)(obj.bits >> 2); \
} \
template<> T _py_cast<T>(VM* vm, const PyVar& obj){ \
    return (T)(obj.bits >> 2); \
}

PY_CAST_INT(char)
PY_CAST_INT(short)
PY_CAST_INT(int)
PY_CAST_INT(long)
PY_CAST_INT(long long)
PY_CAST_INT(unsigned char)
PY_CAST_INT(unsigned short)
PY_CAST_INT(unsigned int)
PY_CAST_INT(unsigned long)
PY_CAST_INT(unsigned long long)


template<> float py_cast<float>(VM* vm, const PyVar& obj){
    vm->check_type(obj, vm->tp_float);
    i64 bits = obj.bits;
    bits = (bits >> 2) << 2;
    return __8B(bits)._float;
}
template<> float _py_cast<float>(VM* vm, const PyVar& obj){
    i64 bits = obj.bits;
    bits = (bits >> 2) << 2;
    return __8B(bits)._float;
}
template<> double py_cast<double>(VM* vm, const PyVar& obj){
    vm->check_type(obj, vm->tp_float);
    i64 bits = obj.bits;
    bits = (bits >> 2) << 2;
    return __8B(bits)._float;
}
template<> double _py_cast<double>(VM* vm, const PyVar& obj){
    i64 bits = obj.bits;
    bits = (bits >> 2) << 2;
    return __8B(bits)._float;
}


#define PY_VAR_INT(T) \
    PyVar py_var(VM* vm, T _val){           \
        i64 val = static_cast<i64>(_val);   \
        if(((val << 2) >> 2) != val){       \
            vm->_error("OverflowError", std::to_string(val) + " is out of range");  \
        }                                                                           \
        val = (val << 2) | 0b01;                                                    \
        return PyVar(reinterpret_cast<int*>(val));                                  \
    }

PY_VAR_INT(char)
PY_VAR_INT(short)
PY_VAR_INT(int)
PY_VAR_INT(long)
PY_VAR_INT(long long)
PY_VAR_INT(unsigned char)
PY_VAR_INT(unsigned short)
PY_VAR_INT(unsigned int)
PY_VAR_INT(unsigned long)
PY_VAR_INT(unsigned long long)

#define PY_VAR_FLOAT(T) \
    PyVar py_var(VM* vm, T _val){           \
        f64 val = static_cast<f64>(_val);   \
        i64 bits = __8B(val)._int;          \
        bits = (bits >> 2) << 2;            \
        bits |= 0b10;                       \
        return PyVar(reinterpret_cast<int*>(bits)); \
    }

PY_VAR_FLOAT(float)
PY_VAR_FLOAT(double)

const PyVar& py_var(VM* vm, bool val){
    return val ? vm->True : vm->False;
}

template<> bool py_cast<bool>(VM* vm, const PyVar& obj){
    vm->check_type(obj, vm->tp_bool);
    return obj == vm->True;
}
template<> bool _py_cast<bool>(VM* vm, const PyVar& obj){
    return obj == vm->True;
}

PyVar py_var(VM* vm, const char val[]){
    return VAR(Str(val));
}

PyVar py_var(VM* vm, std::string val){
    return VAR(Str(std::move(val)));
}

template<typename T>
void _check_py_class(VM* vm, const PyVar& obj){
    vm->check_type(obj, T::_type(vm));
}

PyVar VM::num_negated(const PyVar& obj){
    if (is_int(obj)){
        return VAR(-CAST(i64, obj));
    }else if(is_float(obj)){
        return VAR(-CAST(f64, obj));
    }
    TypeError("expected 'int' or 'float', got " + OBJ_NAME(_t(obj)).escape(true));
    return nullptr;
}

f64 VM::num_to_float(const PyVar& obj){
    if(is_float(obj)){
        return CAST(f64, obj);
    } else if (is_int(obj)){
        return (f64)CAST(i64, obj);
    }
    TypeError("expected 'int' or 'float', got " + OBJ_NAME(_t(obj)).escape(true));
    return 0;
}

const PyVar& VM::asBool(const PyVar& obj){
    if(is_type(obj, tp_bool)) return obj;
    if(obj == None) return False;
    if(is_type(obj, tp_int)) return VAR(CAST(i64, obj) != 0);
    if(is_type(obj, tp_float)) return VAR(CAST(f64, obj) != 0.0);
    PyVarOrNull len_fn = getattr(obj, __len__, false, true);
    if(len_fn != nullptr){
        PyVar ret = call(len_fn);
        return VAR(CAST(i64, ret) > 0);
    }
    return True;
}

i64 VM::hash(const PyVar& obj){
    if (is_type(obj, tp_str)) return CAST(Str&, obj).hash();
    if (is_int(obj)) return CAST(i64, obj);
    if (is_type(obj, tp_tuple)) {
        i64 x = 1000003;
        const Tuple& items = CAST(Tuple&, obj);
        for (int i=0; i<items.size(); i++) {
            i64 y = hash(items[i]);
            x = x ^ (y + 0x9e3779b9 + (x << 6) + (x >> 2)); // recommended by Github Copilot
        }
        return x;
    }
    if (is_type(obj, tp_type)) return obj.bits;
    if (is_type(obj, tp_bool)) return _CAST(bool, obj) ? 1 : 0;
    if (is_float(obj)){
        f64 val = CAST(f64, obj);
        return (i64)std::hash<f64>()(val);
    }
    TypeError("unhashable type: " +  OBJ_NAME(_t(obj)).escape(true));
    return 0;
}

PyVar VM::asRepr(const PyVar& obj){
    return call(obj, __repr__);
}

PyVar VM::new_type_object(PyVar mod, StrName name, PyVar base){
    if(!is_type(base, tp_type)) UNREACHABLE();
    PyVar obj = make_sp<PyObject, Py_<Type>>(tp_type, _all_types.size());
    setattr(obj, __base__, base);
    Str fullName = name.str();
    if(mod != builtins) fullName = OBJ_NAME(mod) + "." + name.str();
    setattr(obj, __name__, VAR(fullName));
    setattr(mod, name, obj);
    _all_types.push_back(obj);
    return obj;
}

PyVar VM::new_module(StrName name) {
    PyVar obj = new_object(tp_module, DummyModule());
    setattr(obj, __name__, VAR(name.str()));
    _modules.set(name, obj);
    return obj;
}

Str VM::disassemble(CodeObject_ co){
    std::vector<int> jumpTargets;
    for(auto byte : co->codes){
        if(byte.op == OP_JUMP_ABSOLUTE || byte.op == OP_SAFE_JUMP_ABSOLUTE || byte.op == OP_POP_JUMP_IF_FALSE){
            jumpTargets.push_back(byte.arg);
        }
    }
    StrStream ss;
    ss << std::string(54, '-') << '\n';
    ss << co->name << ":\n";
    int prev_line = -1;
    for(int i=0; i<co->codes.size(); i++){
        const Bytecode& byte = co->codes[i];
        if(byte.op == OP_NO_OP) continue;
        Str line = std::to_string(byte.line);
        if(byte.line == prev_line) line = "";
        else{
            if(prev_line != -1) ss << "\n";
            prev_line = byte.line;
        }

        std::string pointer;
        if(std::find(jumpTargets.begin(), jumpTargets.end(), i) != jumpTargets.end()){
            pointer = "-> ";
        }else{
            pointer = "   ";
        }
        ss << pad(line, 8) << pointer << pad(std::to_string(i), 3);
        ss << " " << pad(OP_NAMES[byte.op], 20) << " ";
        // ss << pad(byte.arg == -1 ? "" : std::to_string(byte.arg), 5);
        std::string argStr = byte.arg == -1 ? "" : std::to_string(byte.arg);
        if(byte.op == OP_LOAD_CONST){
            argStr += " (" + CAST(Str, asRepr(co->consts[byte.arg])) + ")";
        }
        if(byte.op == OP_LOAD_NAME_REF || byte.op == OP_LOAD_NAME || byte.op == OP_RAISE || byte.op == OP_STORE_NAME){
            argStr += " (" + co->names[byte.arg].first.str().escape(true) + ")";
        }
        if(byte.op == OP_FAST_INDEX || byte.op == OP_FAST_INDEX_REF){
            auto& a = co->names[byte.arg & 0xFFFF];
            auto& x = co->names[(byte.arg >> 16) & 0xFFFF];
            argStr += " (" + a.first.str() + '[' + x.first.str() + "])";
        }
        ss << pad(argStr, 20);      // may overflow
        ss << co->blocks[byte.block].to_string();
        if(i != co->codes.size() - 1) ss << '\n';
    }
    StrStream consts;
    consts << "co_consts: ";
    consts << CAST(Str, asRepr(VAR(co->consts)));

    StrStream names;
    names << "co_names: ";
    List list;
    for(int i=0; i<co->names.size(); i++){
        list.push_back(VAR(co->names[i].first.str()));
    }
    names << CAST(Str, asRepr(VAR(list)));
    ss << '\n' << consts.str() << '\n' << names.str() << '\n';

    for(int i=0; i<co->consts.size(); i++){
        PyVar obj = co->consts[i];
        if(is_type(obj, tp_function)){
            const auto& f = CAST(Function&, obj);
            ss << disassemble(f.code);
        }
    }
    return Str(ss.str());
}

void VM::init_builtin_types(){
    PyVar _tp_object = make_sp<PyObject, Py_<Type>>(1, 0);
    PyVar _tp_type = make_sp<PyObject, Py_<Type>>(1, 1);
    _all_types.push_back(_tp_object);
    _all_types.push_back(_tp_type);
    tp_object = 0; tp_type = 1;

    _types.set("object", _tp_object);
    _types.set("type", _tp_type);

    tp_int = _new_type_object("int");
    tp_float = _new_type_object("float");
    if(tp_int.index != kTpIntIndex || tp_float.index != kTpFloatIndex) UNREACHABLE();

    tp_bool = _new_type_object("bool");
    tp_str = _new_type_object("str");
    tp_list = _new_type_object("list");
    tp_tuple = _new_type_object("tuple");
    tp_slice = _new_type_object("slice");
    tp_range = _new_type_object("range");
    tp_module = _new_type_object("module");
    tp_ref = _new_type_object("_ref");
    tp_star_wrapper = _new_type_object("_star_wrapper");
    
    tp_function = _new_type_object("function");
    tp_native_function = _new_type_object("native_function");
    tp_native_iterator = _new_type_object("native_iterator");
    tp_bound_method = _new_type_object("bound_method");
    tp_super = _new_type_object("super");
    tp_exception = _new_type_object("Exception");

    this->None = new_object(_new_type_object("NoneType"), DUMMY_VAL);
    this->Ellipsis = new_object(_new_type_object("ellipsis"), DUMMY_VAL);
    this->True = new_object(tp_bool, true);
    this->False = new_object(tp_bool, false);
    this->builtins = new_module("builtins");
    this->_main = new_module("__main__");
    this->_py_op_call = new_object(_new_type_object("_py_op_call"), DUMMY_VAL);
    this->_py_op_yield = new_object(_new_type_object("_py_op_yield"), DUMMY_VAL);

    setattr(_t(tp_type), __base__, _t(tp_object));
    setattr(_t(tp_object), __base__, None);
    
    for(auto [k, v]: _types.items()){
        setattr(v, __name__, VAR(k.str()));
    }

    std::vector<Str> pb_types = {"type", "object", "bool", "int", "float", "str", "list", "tuple", "range"};
    for (auto& name : pb_types) {
        setattr(builtins, name, _types[name]);
    }

    post_init();
    for(auto [k, v]: _types.items()) v->attr()._try_perfect_rehash();
    for(auto [k, v]: _modules.items()) v->attr()._try_perfect_rehash();
}

PyVar VM::call(const PyVar& _callable, Args args, const Args& kwargs, bool opCall){
    if(is_type(_callable, tp_type)){
        PyVar* new_f = _callable->attr().try_get(__new__);
        PyVar obj;
        if(new_f != nullptr){
            obj = call(*new_f, std::move(args), kwargs, false);
        }else{
            obj = new_object(_callable, DummyInstance());
            PyVarOrNull init_f = getattr(obj, __init__, false, true);
            if (init_f != nullptr) call(init_f, std::move(args), kwargs, false);
        }
        return obj;
    }

    const PyVar* callable = &_callable;
    if(is_type(*callable, tp_bound_method)){
        auto& bm = CAST(BoundMethod&, *callable);
        callable = &bm.method;      // get unbound method
        args.extend_self(bm.obj);
    }
    
    if(is_type(*callable, tp_native_function)){
        const auto& f = OBJ_GET(NativeFunc, *callable);
        if(kwargs.size() != 0) TypeError("native_function does not accept keyword arguments");
        return f(this, args);
    } else if(is_type(*callable, tp_function)){
        const Function& fn = CAST(Function&, *callable);
        NameDict_ locals = make_sp<NameDict>(
            fn.code->perfect_locals_capacity,
            kLocalsLoadFactor,
            fn.code->perfect_hash_seed
        );

        int i = 0;
        for(StrName name : fn.args){
            if(i < args.size()){
                locals->set(name, std::move(args[i++]));
                continue;
            }
            TypeError("missing positional argument " + name.str().escape(true));
        }

        locals->update(fn.kwargs);

        if(!fn.starred_arg.empty()){
            List vargs;        // handle *args
            while(i < args.size()) vargs.push_back(std::move(args[i++]));
            locals->set(fn.starred_arg, VAR(Tuple::from_list(std::move(vargs))));
        }else{
            for(StrName key : fn.kwargs_order){
                if(i < args.size()){
                    locals->set(key, std::move(args[i++]));
                }else{
                    break;
                }
            }
            if(i < args.size()) TypeError("too many arguments");
        }
        
        for(int i=0; i<kwargs.size(); i+=2){
            const Str& key = CAST(Str&, kwargs[i]);
            if(!fn.kwargs.contains(key)){
                TypeError(key.escape(true) + " is an invalid keyword argument for " + fn.name.str() + "()");
            }
            locals->set(key, kwargs[i+1]);
        }
        const PyVar& _module = fn._module != nullptr ? fn._module : top_frame()->_module;
        auto _frame = _new_frame(fn.code, _module, locals, fn._closure);
        if(fn.code->is_generator) return PyIter(Generator(this, std::move(_frame)));
        callstack.push(std::move(_frame));
        if(opCall) return _py_op_call;
        return _exec();
    }

    PyVarOrNull call_f = getattr(_callable, __call__, false, true);
    if(call_f != nullptr){
        return call(call_f, std::move(args), kwargs, false);
    }
    TypeError(OBJ_NAME(_t(*callable)).escape(true) + " object is not callable");
    return None;
}

void VM::unpack_args(Args& args){
    List unpacked;
    for(int i=0; i<args.size(); i++){
        if(is_type(args[i], tp_star_wrapper)){
            auto& star = _CAST(StarWrapper&, args[i]);
            if(!star.rvalue) UNREACHABLE();
            PyVar list = asList(star.obj);
            List& list_c = CAST(List&, list);
            unpacked.insert(unpacked.end(), list_c.begin(), list_c.end());
        }else{
            unpacked.push_back(args[i]);
        }
    }
    args = Args::from_list(std::move(unpacked));
}

PyVarOrNull VM::getattr(const PyVar& obj, StrName name, bool throw_err, bool class_only) {
    PyVar* val;
    PyObject* cls;

    if(is_type(obj, tp_super)){
        const PyVar* root = &obj;
        int depth = 1;
        while(true){
            root = &OBJ_GET(PyVar, *root);
            if(!is_type(*root, tp_super)) break;
            depth++;
        }
        cls = _t(*root).get();
        for(int i=0; i<depth; i++) cls = cls->attr(__base__).get();

        if(!class_only){
            val = (*root)->attr().try_get(name);
            if(val != nullptr) return *val;
        }
    }else{
        if(!class_only && !obj.is_tagged() && obj->is_attr_valid()){
            val = obj->attr().try_get(name);
            if(val != nullptr) return *val;
        }
        cls = _t(obj).get();
    }

    while(cls != None.get()) {
        val = cls->attr().try_get(name);
        if(val != nullptr){
            PyVarOrNull descriptor = getattr(*val, __get__, false, true);
            if(descriptor != nullptr) return call(descriptor, one_arg(obj));
            if(is_type(*val, tp_function) || is_type(*val, tp_native_function)){
                return VAR(BoundMethod(obj, *val));
            }else{
                return *val;
            }
        }else{
            // this operation is expensive!!!
            const Str& s = name.str();
            if(s.empty() || s[0] != '_'){
                PyVar* interceptor = cls->attr().try_get(__getattr__);
                if(interceptor != nullptr){
                    return call(*interceptor, two_args(obj, VAR(s)));
                }
            }
        }
        cls = cls->attr(__base__).get();
    }
    if(throw_err) AttributeError(obj, name);
    return nullptr;
}

template<typename T>
void VM::setattr(PyVar& obj, StrName name, T&& value) {
    if(obj.is_tagged()) TypeError("cannot set attribute");
    PyObject* p = obj.get();
    while(p->type == tp_super) p = static_cast<PyVar*>(p->value())->get();
    if(!p->is_attr_valid()) TypeError("cannot set attribute");
    p->attr().set(name, std::forward<T>(value));
}

template<int ARGC>
void VM::bind_method(PyVar obj, Str funcName, NativeFuncRaw fn) {
    check_type(obj, tp_type);
    setattr(obj, funcName, VAR(NativeFunc(fn, ARGC, true)));
}

template<int ARGC>
void VM::bind_func(PyVar obj, Str funcName, NativeFuncRaw fn) {
    setattr(obj, funcName, VAR(NativeFunc(fn, ARGC, false)));
}

void VM::_error(Exception e){
    if(callstack.empty()){
        e.is_re = false;
        throw e;
    }
    top_frame()->push(VAR(e));
    _raise();
}

PyVar VM::_exec(){
    Frame* frame = top_frame();
    i64 base_id = frame->id;
    PyVar ret = nullptr;
    bool need_raise = false;

    while(true){
        if(frame->id < base_id) UNREACHABLE();
        try{
            if(need_raise){ need_raise = false; _raise(); }
            ret = run_frame(frame);
            if(ret == _py_op_yield) return _py_op_yield;
            if(ret != _py_op_call){
                if(frame->id == base_id){      // [ frameBase<- ]
                    callstack.pop();
                    return ret;
                }else{
                    callstack.pop();
                    frame = callstack.top().get();
                    frame->push(ret);
                }
            }else{
                frame = callstack.top().get();  // [ frameBase, newFrame<- ]
            }
        }catch(HandledException& e){
            continue;
        }catch(UnhandledException& e){
            PyVar obj = frame->pop();
            Exception& _e = CAST(Exception&, obj);
            _e.st_push(frame->snapshot());
            callstack.pop();
            if(callstack.empty()) throw _e;
            frame = callstack.top().get();
            frame->push(obj);
            if(frame->id < base_id) throw ToBeRaisedException();
            need_raise = true;
        }catch(ToBeRaisedException& e){
            need_raise = true;
        }
    }
}

}   // namespace pkpy


namespace pkpy {

struct BaseRef {
    virtual PyVar get(VM*, Frame*) const = 0;
    virtual void set(VM*, Frame*, PyVar) const = 0;
    virtual void del(VM*, Frame*) const = 0;
    virtual ~BaseRef() = default;
};

struct NameRef : BaseRef {
    const std::pair<StrName, NameScope> pair;
    inline StrName name() const { return pair.first; }
    inline NameScope scope() const { return pair.second; }
    NameRef(const std::pair<StrName, NameScope>& pair) : pair(pair) {}

    PyVar get(VM* vm, Frame* frame) const{
        PyVar* val;
        val = frame->f_locals().try_get(name());
        if(val != nullptr) return *val;
        val = frame->f_closure_try_get(name());
        if(val != nullptr) return *val;
        val = frame->f_globals().try_get(name());
        if(val != nullptr) return *val;
        val = vm->builtins->attr().try_get(name());
        if(val != nullptr) return *val;
        vm->NameError(name());
        return nullptr;
    }

    void set(VM* vm, Frame* frame, PyVar val) const{
        switch(scope()) {
            case NAME_LOCAL: frame->f_locals().set(name(), std::move(val)); break;
            case NAME_GLOBAL:
                if(frame->f_locals().try_set(name(), std::move(val))) return;
                frame->f_globals().set(name(), std::move(val));
                break;
            default: UNREACHABLE();
        }
    }

    void del(VM* vm, Frame* frame) const{
        switch(scope()) {
            case NAME_LOCAL: {
                if(frame->f_locals().contains(name())){
                    frame->f_locals().erase(name());
                }else{
                    vm->NameError(name());
                }
            } break;
            case NAME_GLOBAL:
            {
                if(frame->f_locals().contains(name())){
                    frame->f_locals().erase(name());
                }else{
                    if(frame->f_globals().contains(name())){
                        frame->f_globals().erase(name());
                    }else{
                        vm->NameError(name());
                    }
                }
            } break;
            default: UNREACHABLE();
        }
    }
};

struct AttrRef : BaseRef {
    mutable PyVar obj;
    NameRef attr;
    AttrRef(PyVar obj, NameRef attr) : obj(obj), attr(attr) {}

    PyVar get(VM* vm, Frame* frame) const{
        return vm->getattr(obj, attr.name());
    }

    void set(VM* vm, Frame* frame, PyVar val) const{
        vm->setattr(obj, attr.name(), std::move(val));
    }

    void del(VM* vm, Frame* frame) const{
        if(!obj->is_attr_valid()) vm->TypeError("cannot delete attribute");
        if(!obj->attr().contains(attr.name())) vm->AttributeError(obj, attr.name());
        obj->attr().erase(attr.name());
    }
};

struct IndexRef : BaseRef {
    mutable PyVar obj;
    PyVar index;
    IndexRef(PyVar obj, PyVar index) : obj(obj), index(index) {}

    PyVar get(VM* vm, Frame* frame) const{
        return vm->fast_call(__getitem__, two_args(obj, index));
    }

    void set(VM* vm, Frame* frame, PyVar val) const{
        Args args(3);
        args[0] = obj; args[1] = index; args[2] = std::move(val);
        vm->fast_call(__setitem__, std::move(args));
    }

    void del(VM* vm, Frame* frame) const{
        vm->fast_call(__delitem__, two_args(obj, index));
    }
};

struct TupleRef : BaseRef {
    Tuple objs;
    TupleRef(Tuple&& objs) : objs(std::move(objs)) {}

    PyVar get(VM* vm, Frame* frame) const{
        Tuple args(objs.size());
        for (int i = 0; i < objs.size(); i++) {
            args[i] = vm->PyRef_AS_C(objs[i])->get(vm, frame);
        }
        return VAR(std::move(args));
    }

    void set(VM* vm, Frame* frame, PyVar val) const{
        val = vm->asIter(val);
        BaseIter* iter = vm->PyIter_AS_C(val);
        for(int i=0; i<objs.size(); i++){
            PyVarOrNull x;
            if(is_type(objs[i], vm->tp_star_wrapper)){
                auto& star = _CAST(StarWrapper&, objs[i]);
                if(star.rvalue) vm->ValueError("can't use starred expression here");
                if(i != objs.size()-1) vm->ValueError("* can only be used at the end");
                auto ref = vm->PyRef_AS_C(star.obj);
                List list;
                while((x = iter->next()) != nullptr) list.push_back(x);
                ref->set(vm, frame, VAR(std::move(list)));
                return;
            }else{
                x = iter->next();
                if(x == nullptr) vm->ValueError("not enough values to unpack");
                vm->PyRef_AS_C(objs[i])->set(vm, frame, x);
            }
        }
        PyVarOrNull x = iter->next();
        if(x != nullptr) vm->ValueError("too many values to unpack");
    }

    void del(VM* vm, Frame* frame) const{
        for(int i=0; i<objs.size(); i++) vm->PyRef_AS_C(objs[i])->del(vm, frame);
    }
};


template<typename P>
PyVarRef VM::PyRef(P&& value) {
    static_assert(std::is_base_of_v<BaseRef, std::decay_t<P>>);
    return new_object(tp_ref, std::forward<P>(value));
}

const BaseRef* VM::PyRef_AS_C(const PyVar& obj)
{
    if(!is_type(obj, tp_ref)) TypeError("expected an l-value");
    return static_cast<const BaseRef*>(obj->value());
}

/***** Frame's Impl *****/
inline void Frame::try_deref(VM* vm, PyVar& v){
    if(is_type(v, vm->tp_ref)) v = vm->PyRef_AS_C(v)->get(vm, this);
}

}   // namespace pkpy


namespace pkpy{

PyVar VM::run_frame(Frame* frame){
    while(frame->has_next_bytecode()){
        const Bytecode& byte = frame->next_bytecode();
        // if(true || frame->_module != builtins){
        //     printf("%d: %s (%d) %s\n",                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 frame->_ip, OP_NAMES[byte.op], byte.arg, frame->stack_info().c_str());
        // }
        switch (byte.op)
        {
        case OP_NO_OP: continue;
        case OP_SETUP_DECORATOR: continue;
        case OP_LOAD_CONST: frame->push(frame->co->consts[byte.arg]); continue;
        case OP_LOAD_FUNCTION: {
            const PyVar obj = frame->co->consts[byte.arg];
            Function f = CAST(Function, obj);  // copy
            f._module = frame->_module;
            frame->push(VAR(f));
        } continue;
        case OP_SETUP_CLOSURE: {
            Function& f = CAST(Function&, frame->top());    // reference
            f._closure = frame->_locals;
        } continue;
        case OP_LOAD_NAME_REF: {
            frame->push(PyRef(NameRef(frame->co->names[byte.arg])));
        } continue;
        case OP_LOAD_NAME: {
            frame->push(NameRef(frame->co->names[byte.arg]).get(this, frame));
        } continue;
        case OP_STORE_NAME: {
            auto& p = frame->co->names[byte.arg];
            NameRef(p).set(this, frame, frame->pop());
        } continue;
        case OP_BUILD_ATTR_REF: case OP_BUILD_ATTR: {
            auto& attr = frame->co->names[byte.arg];
            PyVar obj = frame->pop_value(this);
            AttrRef ref = AttrRef(obj, NameRef(attr));
            if(byte.op == OP_BUILD_ATTR) frame->push(ref.get(this, frame));
            else frame->push(PyRef(ref));
        } continue;
        case OP_BUILD_INDEX: {
            PyVar index = frame->pop_value(this);
            auto ref = IndexRef(frame->pop_value(this), index);
            if(byte.arg > 0) frame->push(ref.get(this, frame));
            else frame->push(PyRef(ref));
        } continue;
        case OP_FAST_INDEX: case OP_FAST_INDEX_REF: {
            auto& a = frame->co->names[byte.arg & 0xFFFF];
            auto& x = frame->co->names[(byte.arg >> 16) & 0xFFFF];
            auto ref = IndexRef(NameRef(a).get(this, frame), NameRef(x).get(this, frame));
            if(byte.op == OP_FAST_INDEX) frame->push(ref.get(this, frame));
            else frame->push(PyRef(ref));
        } continue;
        case OP_ROT_TWO: ::std::swap(frame->top(), frame->top_1()); continue;
        case OP_STORE_REF: {
            // PyVar obj = frame->pop_value(this);
            // PyVarRef r = frame->pop();
            // PyRef_AS_C(r)->set(this, frame, std::move(obj));
            PyRef_AS_C(frame->top_1())->set(this, frame, frame->top_value(this));
            frame->_pop(); frame->_pop();
        } continue;
        case OP_DELETE_REF: 
            PyRef_AS_C(frame->top())->del(this, frame);
            frame->_pop();
            continue;
        case OP_BUILD_TUPLE: {
            Args items = frame->pop_n_values_reversed(this, byte.arg);
            frame->push(VAR(std::move(items)));
        } continue;
        case OP_BUILD_TUPLE_REF: {
            Args items = frame->pop_n_reversed(byte.arg);
            frame->push(PyRef(TupleRef(std::move(items))));
        } continue;
        case OP_BUILD_STRING: {
            Args items = frame->pop_n_values_reversed(this, byte.arg);
            StrStream ss;
            for(int i=0; i<items.size(); i++) ss << CAST(Str, asStr(items[i]));
            frame->push(VAR(ss.str()));
        } continue;
        case OP_LOAD_EVAL_FN: frame->push(builtins->attr(m_eval)); continue;
        case OP_LIST_APPEND: {
            PyVar obj = frame->pop_value(this);
            List& list = CAST(List&, frame->top_1());
            list.push_back(std::move(obj));
        } continue;
        case OP_BEGIN_CLASS: {
            auto& name = frame->co->names[byte.arg];
            PyVar clsBase = frame->pop_value(this);
            if(clsBase == None) clsBase = _t(tp_object);
            check_type(clsBase, tp_type);
            PyVar cls = new_type_object(frame->_module, name.first, clsBase);
            frame->push(cls);
        } continue;
        case OP_END_CLASS: {
            PyVar cls = frame->pop();
            cls->attr()._try_perfect_rehash();
        }; continue;
        case OP_STORE_CLASS_ATTR: {
            auto& name = frame->co->names[byte.arg];
            PyVar obj = frame->pop_value(this);
            PyVar& cls = frame->top();
            cls->attr().set(name.first, std::move(obj));
        } continue;
        case OP_RETURN_VALUE: return frame->pop_value(this);
        case OP_PRINT_EXPR: {
            const PyVar expr = frame->top_value(this);
            if(expr != None) *_stdout << CAST(Str, asRepr(expr)) << '\n';
        } continue;
        case OP_POP_TOP: frame->_pop(); continue;
        case OP_BINARY_OP: {
            Args args(2);
            args[1] = frame->pop_value(this);
            args[0] = frame->top_value(this);
            frame->top() = fast_call(BINARY_SPECIAL_METHODS[byte.arg], std::move(args));
        } continue;
        case OP_BITWISE_OP: {
            Args args(2);
            args[1] = frame->pop_value(this);
            args[0] = frame->top_value(this);
            frame->top() = fast_call(BITWISE_SPECIAL_METHODS[byte.arg], std::move(args));
        } continue;
        case OP_INPLACE_BINARY_OP: {
            Args args(2);
            args[1] = frame->pop();
            args[0] = frame->top_value(this);
            PyVar ret = fast_call(BINARY_SPECIAL_METHODS[byte.arg], std::move(args));
            PyRef_AS_C(frame->top())->set(this, frame, std::move(ret));
            frame->_pop();
        } continue;
        case OP_INPLACE_BITWISE_OP: {
            Args args(2);
            args[1] = frame->pop_value(this);
            args[0] = frame->top_value(this);
            PyVar ret = fast_call(BITWISE_SPECIAL_METHODS[byte.arg], std::move(args));
            PyRef_AS_C(frame->top())->set(this, frame, std::move(ret));
            frame->_pop();
        } continue;
        case OP_COMPARE_OP: {
            Args args(2);
            args[1] = frame->pop_value(this);
            args[0] = frame->top_value(this);
            frame->top() = fast_call(CMP_SPECIAL_METHODS[byte.arg], std::move(args));
        } continue;
        case OP_IS_OP: {
            PyVar rhs = frame->pop_value(this);
            bool ret_c = rhs == frame->top_value(this);
            if(byte.arg == 1) ret_c = !ret_c;
            frame->top() = VAR(ret_c);
        } continue;
        case OP_CONTAINS_OP: {
            PyVar rhs = frame->pop_value(this);
            bool ret_c = CAST(bool, call(rhs, __contains__, one_arg(frame->pop_value(this))));
            if(byte.arg == 1) ret_c = !ret_c;
            frame->push(VAR(ret_c));
        } continue;
        case OP_UNARY_NEGATIVE:
            frame->top() = num_negated(frame->top_value(this));
            continue;
        case OP_UNARY_NOT: {
            PyVar obj = frame->pop_value(this);
            const PyVar& obj_bool = asBool(obj);
            frame->push(VAR(!_CAST(bool, obj_bool)));
        } continue;
        case OP_POP_JUMP_IF_FALSE:
            if(!_CAST(bool, asBool(frame->pop_value(this)))) frame->jump_abs(byte.arg);
            continue;
        case OP_LOAD_NONE: frame->push(None); continue;
        case OP_LOAD_TRUE: frame->push(True); continue;
        case OP_LOAD_FALSE: frame->push(False); continue;
        case OP_LOAD_ELLIPSIS: frame->push(Ellipsis); continue;
        case OP_ASSERT: {
            PyVar _msg = frame->pop_value(this);
            Str msg = CAST(Str, asStr(_msg));
            PyVar expr = frame->pop_value(this);
            if(asBool(expr) != True) _error("AssertionError", msg);
        } continue;
        case OP_EXCEPTION_MATCH: {
            const auto& e = CAST(Exception&, frame->top());
            StrName name = frame->co->names[byte.arg].first;
            frame->push(VAR(e.match_type(name)));
        } continue;
        case OP_RAISE: {
            PyVar obj = frame->pop_value(this);
            Str msg = obj == None ? "" : CAST(Str, asStr(obj));
            StrName type = frame->co->names[byte.arg].first;
            _error(type, msg);
        } continue;
        case OP_RE_RAISE: _raise(); continue;
        case OP_BUILD_LIST:
            frame->push(VAR(frame->pop_n_values_reversed(this, byte.arg).move_to_list()));
            continue;
        case OP_BUILD_MAP: {
            Args items = frame->pop_n_values_reversed(this, byte.arg*2);
            PyVar obj = call(builtins->attr("dict"));
            for(int i=0; i<items.size(); i+=2){
                call(obj, __setitem__, two_args(items[i], items[i+1]));
            }
            frame->push(obj);
        } continue;
        case OP_BUILD_SET: {
            PyVar list = VAR(
                frame->pop_n_values_reversed(this, byte.arg).move_to_list()
            );
            PyVar obj = call(builtins->attr("set"), one_arg(list));
            frame->push(obj);
        } continue;
        case OP_DUP_TOP_VALUE: frame->push(frame->top_value(this)); continue;
        case OP_UNARY_STAR: {
            if(byte.arg > 0){   // rvalue
                frame->top() = VAR(StarWrapper(frame->top_value(this), true));
            }else{
                PyRef_AS_C(frame->top()); // check ref
                frame->top() = VAR(StarWrapper(frame->top(), false));
            }
        } continue;
        case OP_CALL_KWARGS_UNPACK: case OP_CALL_KWARGS: {
            int ARGC = byte.arg & 0xFFFF;
            int KWARGC = (byte.arg >> 16) & 0xFFFF;
            Args kwargs = frame->pop_n_values_reversed(this, KWARGC*2);
            Args args = frame->pop_n_values_reversed(this, ARGC);
            if(byte.op == OP_CALL_KWARGS_UNPACK) unpack_args(args);
            PyVar callable = frame->pop_value(this);
            PyVar ret = call(callable, std::move(args), kwargs, true);
            if(ret == _py_op_call) return ret;
            frame->push(std::move(ret));
        } continue;
        case OP_CALL_UNPACK: case OP_CALL: {
            Args args = frame->pop_n_values_reversed(this, byte.arg);
            if(byte.op == OP_CALL_UNPACK) unpack_args(args);
            PyVar callable = frame->pop_value(this);
            PyVar ret = call(callable, std::move(args), no_arg(), true);
            if(ret == _py_op_call) return ret;
            frame->push(std::move(ret));
        } continue;
        case OP_JUMP_ABSOLUTE: frame->jump_abs(byte.arg); continue;
        case OP_SAFE_JUMP_ABSOLUTE: frame->jump_abs_safe(byte.arg); continue;
        case OP_GOTO: {
            StrName label = frame->co->names[byte.arg].first;
            auto it = frame->co->labels.find(label);
            if(it == frame->co->labels.end()) _error("KeyError", "label " + label.str().escape(true) + " not found");
            frame->jump_abs_safe(it->second);
        } continue;
        case OP_GET_ITER: {
            PyVar obj = frame->pop_value(this);
            PyVar iter = asIter(obj);
            check_type(frame->top(), tp_ref);
            PyIter_AS_C(iter)->loop_var = frame->pop();
            frame->push(std::move(iter));
        } continue;
        case OP_FOR_ITER: {
            BaseIter* it = PyIter_AS_C(frame->top());
            PyVar obj = it->next();
            if(obj != nullptr){
                PyRef_AS_C(it->loop_var)->set(this, frame, std::move(obj));
            }else{
                int blockEnd = frame->co->blocks[byte.block].end;
                frame->jump_abs_safe(blockEnd);
            }
        } continue;
        case OP_LOOP_CONTINUE: {
            int blockStart = frame->co->blocks[byte.block].start;
            frame->jump_abs(blockStart);
        } continue;
        case OP_LOOP_BREAK: {
            int blockEnd = frame->co->blocks[byte.block].end;
            frame->jump_abs_safe(blockEnd);
        } continue;
        case OP_JUMP_IF_FALSE_OR_POP: {
            const PyVar expr = frame->top_value(this);
            if(asBool(expr)==False) frame->jump_abs(byte.arg);
            else frame->pop_value(this);
        } continue;
        case OP_JUMP_IF_TRUE_OR_POP: {
            const PyVar expr = frame->top_value(this);
            if(asBool(expr)==True) frame->jump_abs(byte.arg);
            else frame->pop_value(this);
        } continue;
        case OP_BUILD_SLICE: {
            PyVar stop = frame->pop_value(this);
            PyVar start = frame->pop_value(this);
            Slice s;
            if(start != None) { s.start = CAST(int, start);}
            if(stop != None) { s.stop = CAST(int, stop);}
            frame->push(VAR(s));
        } continue;
        case OP_IMPORT_NAME: {
            StrName name = frame->co->names[byte.arg].first;
            PyVar* ext_mod = _modules.try_get(name);
            if(ext_mod == nullptr){
                auto it2 = _lazy_modules.find(name);
                if(it2 == _lazy_modules.end()){
                    _error("ImportError", "module " + name.str().escape(true) + " not found");
                }else{
                    const Str& source = it2->second;
                    CodeObject_ code = compile(source, name.str(), EXEC_MODE);
                    PyVar new_mod = new_module(name);
                    _exec(code, new_mod);
                    frame->push(new_mod);
                    _lazy_modules.erase(it2);
                    new_mod->attr()._try_perfect_rehash();
                }
            }else{
                frame->push(*ext_mod);
            }
        } continue;
        case OP_STORE_ALL_NAMES: {
            PyVar obj = frame->pop_value(this);
            for(auto& [name, value]: obj->attr().items()){
                Str s = name.str();
                if(s.empty() || s[0] == '_') continue;
                frame->f_globals().set(name, value);
            }
        }; continue;
        case OP_YIELD_VALUE: return _py_op_yield;
        // TODO: using "goto" inside with block may cause __exit__ not called
        case OP_WITH_ENTER: call(frame->pop_value(this), __enter__); continue;
        case OP_WITH_EXIT: call(frame->pop_value(this), __exit__); continue;
        case OP_TRY_BLOCK_ENTER: frame->on_try_block_enter(); continue;
        case OP_TRY_BLOCK_EXIT: frame->on_try_block_exit(); continue;
        default: throw std::runtime_error(Str("opcode ") + OP_NAMES[byte.op] + " is not implemented");
        }
    }

    if(frame->co->src->mode == EVAL_MODE || frame->co->src->mode == JSON_MODE){
        if(frame->_data.size() != 1) throw std::runtime_error("_data.size() != 1 in EVAL/JSON_MODE");
        return frame->pop_value(this);
    }
#if PK_EXTRA_CHECK
    if(!frame->_data.empty()) throw std::runtime_error("_data.size() != 0 in EXEC_MODE");
#endif
    return None;
}

} // namespace pkpy


namespace pkpy{

class Compiler;
typedef void (Compiler::*GrammarFn)();
typedef void (Compiler::*CompilerAction)();

struct GrammarRule{
    GrammarFn prefix;
    GrammarFn infix;
    Precedence precedence;
};

enum StringType { NORMAL_STRING, RAW_STRING, F_STRING };

class Compiler {
    std::unique_ptr<Parser> parser;
    std::stack<CodeObject_> codes;
    int lexing_count = 0;
    bool used = false;
    VM* vm;
    std::map<TokenIndex, GrammarRule> rules;

    CodeObject_ co() const{ return codes.top(); }
    CompileMode mode() const{ return parser->src->mode; }
    NameScope name_scope() const { return codes.size()>1 ? NAME_LOCAL : NAME_GLOBAL; }

public:
    Compiler(VM* vm, const char* source, Str filename, CompileMode mode){
        this->vm = vm;
        this->parser = std::make_unique<Parser>(
            make_sp<SourceData>(source, filename, mode)
        );

// http://journal.stuffwithstuff.com/2011/03/19/pratt-parsers-expression-parsing-made-easy/
#define METHOD(name) &Compiler::name
#define NO_INFIX nullptr, PREC_NONE
        for(TokenIndex i=0; i<kTokenCount; i++) rules[i] = { nullptr, NO_INFIX };
        rules[TK(".")] =    { nullptr,               METHOD(exprAttrib),         PREC_ATTRIB };
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
        rules[TK(">>=")] =      { nullptr,               METHOD(exprAssign),         PREC_ASSIGNMENT };
        rules[TK("<<=")] =      { nullptr,               METHOD(exprAssign),         PREC_ASSIGNMENT };
        rules[TK(",")] =        { nullptr,               METHOD(exprComma),          PREC_COMMA };
        rules[TK("<<")] =       { nullptr,               METHOD(exprBinaryOp),       PREC_BITWISE_SHIFT };
        rules[TK(">>")] =       { nullptr,               METHOD(exprBinaryOp),       PREC_BITWISE_SHIFT };
        rules[TK("&")] =        { nullptr,               METHOD(exprBinaryOp),       PREC_BITWISE_AND };
        rules[TK("|")] =        { nullptr,               METHOD(exprBinaryOp),       PREC_BITWISE_OR };
        rules[TK("^")] =        { nullptr,               METHOD(exprBinaryOp),       PREC_BITWISE_XOR };
#undef METHOD
#undef NO_INFIX

#define EXPR() parse_expression(PREC_TERNARY)             // no '=' and ',' just a simple expression
#define EXPR_TUPLE() parse_expression(PREC_COMMA)         // no '=', but ',' is allowed
#define EXPR_ANY() parse_expression(PREC_ASSIGNMENT)
    }

private:
    Str eat_string_until(char quote, bool raw) {
        bool quote3 = parser->match_n_chars(2, quote);
        std::vector<char> buff;
        while (true) {
            char c = parser->eatchar_include_newline();
            if (c == quote){
                if(quote3 && !parser->match_n_chars(2, quote)){
                    buff.push_back(c);
                    continue;
                }
                break;
            }
            if (c == '\0'){
                if(quote3 && parser->src->mode == REPL_MODE){
                    throw NeedMoreLines(false);
                }
                SyntaxError("EOL while scanning string literal");
            }
            if (c == '\n'){
                if(!quote3) SyntaxError("EOL while scanning string literal");
                else{
                    buff.push_back(c);
                    continue;
                }
            }
            if (!raw && c == '\\') {
                switch (parser->eatchar_include_newline()) {
                    case '"':  buff.push_back('"');  break;
                    case '\'': buff.push_back('\''); break;
                    case '\\': buff.push_back('\\'); break;
                    case 'n':  buff.push_back('\n'); break;
                    case 'r':  buff.push_back('\r'); break;
                    case 't':  buff.push_back('\t'); break;
                    default: SyntaxError("invalid escape char");
                }
            } else {
                buff.push_back(c);
            }
        }
        return Str(buff.data(), buff.size());
    }

    void eat_string(char quote, StringType type) {
        Str s = eat_string_until(quote, type == RAW_STRING);
        if(type == F_STRING){
            parser->set_next_token(TK("@fstr"), VAR(s));
        }else{
            parser->set_next_token(TK("@str"), VAR(s));
        }
    }

    void eat_number() {
        static const std::regex pattern("^(0x)?[0-9a-fA-F]+(\\.[0-9]+)?");
        std::smatch m;

        const char* i = parser->token_start;
        while(*i != '\n' && *i != '\0') i++;
        std::string s = std::string(parser->token_start, i);

        try{
            if (std::regex_search(s, m, pattern)) {
                // here is m.length()-1, since the first char was eaten by lex_token()
                for(int j=0; j<m.length()-1; j++) parser->eatchar();

                int base = 10;
                size_t size;
                if (m[1].matched) base = 16;
                if (m[2].matched) {
                    if(base == 16) SyntaxError("hex literal should not contain a dot");
                    parser->set_next_token(TK("@num"), VAR(S_TO_FLOAT(m[0], &size)));
                } else {
                    parser->set_next_token(TK("@num"), VAR(S_TO_INT(m[0], &size, base)));
                }
                if (size != m.length()) UNREACHABLE();
            }
        }catch(std::exception& _){
            SyntaxError("invalid number literal");
        } 
    }

    void lex_token(){
        lexing_count++;
        _lex_token();
        lexing_count--;
    }

    // Lex the next token and set it as the next token.
    void _lex_token() {
        parser->prev = parser->curr;
        parser->curr = parser->next_token();
        //std::cout << parser->curr.info() << std::endl;

        while (parser->peekchar() != '\0') {
            parser->token_start = parser->curr_char;
            char c = parser->eatchar_include_newline();
            switch (c) {
                case '\'': case '"': eat_string(c, NORMAL_STRING); return;
                case '#': parser->skip_line_comment(); break;
                case '{': parser->set_next_token(TK("{")); return;
                case '}': parser->set_next_token(TK("}")); return;
                case ',': parser->set_next_token(TK(",")); return;
                case ':': parser->set_next_token_2(':', TK(":"), TK("::")); return;
                case ';': parser->set_next_token(TK(";")); return;
                case '(': parser->set_next_token(TK("(")); return;
                case ')': parser->set_next_token(TK(")")); return;
                case '[': parser->set_next_token(TK("[")); return;
                case ']': parser->set_next_token(TK("]")); return;
                case '@': parser->set_next_token(TK("@")); return;
                case '%': parser->set_next_token_2('=', TK("%"), TK("%=")); return;
                case '&': parser->set_next_token_2('=', TK("&"), TK("&=")); return;
                case '|': parser->set_next_token_2('=', TK("|"), TK("|=")); return;
                case '^': parser->set_next_token_2('=', TK("^"), TK("^=")); return;
                case '?': parser->set_next_token(TK("?")); return;
                case '.': {
                    if(parser->matchchar('.')) {
                        if(parser->matchchar('.')) {
                            parser->set_next_token(TK("..."));
                        } else {
                            SyntaxError("invalid token '..'");
                        }
                    } else {
                        parser->set_next_token(TK("."));
                    }
                    return;
                }
                case '=': parser->set_next_token_2('=', TK("="), TK("==")); return;
                case '+': parser->set_next_token_2('=', TK("+"), TK("+=")); return;
                case '>': {
                    if(parser->matchchar('=')) parser->set_next_token(TK(">="));
                    else if(parser->matchchar('>')) parser->set_next_token_2('=', TK(">>"), TK(">>="));
                    else parser->set_next_token(TK(">"));
                    return;
                }
                case '<': {
                    if(parser->matchchar('=')) parser->set_next_token(TK("<="));
                    else if(parser->matchchar('<')) parser->set_next_token_2('=', TK("<<"), TK("<<="));
                    else parser->set_next_token(TK("<"));
                    return;
                }
                case '-': {
                    if(parser->matchchar('=')) parser->set_next_token(TK("-="));
                    else if(parser->matchchar('>')) parser->set_next_token(TK("->"));
                    else parser->set_next_token(TK("-"));
                    return;
                }
                case '!':
                    if(parser->matchchar('=')) parser->set_next_token(TK("!="));
                    else SyntaxError("expected '=' after '!'");
                    break;
                case '*':
                    if (parser->matchchar('*')) {
                        parser->set_next_token(TK("**"));  // '**'
                    } else {
                        parser->set_next_token_2('=', TK("*"), TK("*="));
                    }
                    return;
                case '/':
                    if(parser->matchchar('/')) {
                        parser->set_next_token_2('=', TK("//"), TK("//="));
                    } else {
                        parser->set_next_token_2('=', TK("/"), TK("/="));
                    }
                    return;
                case '\r': break;       // just ignore '\r'
                case ' ': case '\t': parser->eat_spaces(); break;
                case '\n': {
                    parser->set_next_token(TK("@eol"));
                    if(!parser->eat_indentation()) IndentationError("unindent does not match any outer indentation level");
                    return;
                }
                default: {
                    if(c == 'f'){
                        if(parser->matchchar('\'')) {eat_string('\'', F_STRING); return;}
                        if(parser->matchchar('"')) {eat_string('"', F_STRING); return;}
                    }else if(c == 'r'){
                        if(parser->matchchar('\'')) {eat_string('\'', RAW_STRING); return;}
                        if(parser->matchchar('"')) {eat_string('"', RAW_STRING); return;}
                    }

                    if (c >= '0' && c <= '9') {
                        eat_number();
                        return;
                    }
                    
                    switch (parser->eat_name())
                    {
                        case 0: break;
                        case 1: SyntaxError("invalid char: " + std::string(1, c));
                        case 2: SyntaxError("invalid utf8 sequence: " + std::string(1, c));
                        case 3: SyntaxError("@id contains invalid char"); break;
                        case 4: SyntaxError("invalid JSON token"); break;
                        default: UNREACHABLE();
                    }
                    return;
                }
            }
        }

        parser->token_start = parser->curr_char;
        parser->set_next_token(TK("@eof"));
    }

    inline TokenIndex peek() {
        return parser->curr.type;
    }

    // not sure this will work
    TokenIndex peek_next() {
        if(parser->nexts.empty()) return TK("@eof");
        return parser->nexts.front().type;
    }

    bool match(TokenIndex expected) {
        if (peek() != expected) return false;
        lex_token();
        return true;
    }

    void consume(TokenIndex expected) {
        if (!match(expected)){
            StrStream ss;
            ss << "expected '" << TK_STR(expected) << "', but got '" << TK_STR(peek()) << "'";
            SyntaxError(ss.str());
        }
    }

    bool match_newlines(bool repl_throw=false) {
        bool consumed = false;
        if (peek() == TK("@eol")) {
            while (peek() == TK("@eol")) lex_token();
            consumed = true;
        }
        if (repl_throw && peek() == TK("@eof")){
            throw NeedMoreLines(co()->_is_compiling_class);
        }
        return consumed;
    }

    bool match_end_stmt() {
        if (match(TK(";"))) { match_newlines(); return true; }
        if (match_newlines() || peek()==TK("@eof")) return true;
        if (peek() == TK("@dedent")) return true;
        return false;
    }

    void consume_end_stmt() {
        if (!match_end_stmt()) SyntaxError("expected statement end");
    }

    void exprLiteral() {
        PyVar value = parser->prev.value;
        int index = co()->add_const(value);
        emit(OP_LOAD_CONST, index);
    }

    void exprFString() {
        static const std::regex pattern(R"(\{(.*?)\})");
        PyVar value = parser->prev.value;
        Str s = CAST(Str, value);
        std::sregex_iterator begin(s.begin(), s.end(), pattern);
        std::sregex_iterator end;
        int size = 0;
        int i = 0;
        for(auto it = begin; it != end; it++) {
            std::smatch m = *it;
            if (i < m.position()) {
                std::string literal = s.substr(i, m.position() - i);
                emit(OP_LOAD_CONST, co()->add_const(VAR(literal)));
                size++;
            }
            emit(OP_LOAD_EVAL_FN);
            emit(OP_LOAD_CONST, co()->add_const(VAR(m[1].str())));
            emit(OP_CALL, 1);
            size++;
            i = (int)(m.position() + m.length());
        }
        if (i < s.size()) {
            std::string literal = s.substr(i, s.size() - i);
            emit(OP_LOAD_CONST, co()->add_const(VAR(literal)));
            size++;
        }
        emit(OP_BUILD_STRING, size);
    }

    void exprLambda() {
        Function func;
        func.name = "<lambda>";
        if(!match(TK(":"))){
            _compile_f_args(func, false);
            consume(TK(":"));
        }
        func.code = make_sp<CodeObject>(parser->src, func.name.str());
        this->codes.push(func.code);
        co()->_rvalue += 1; EXPR(); co()->_rvalue -= 1;
        emit(OP_RETURN_VALUE);
        func.code->optimize(vm);
        this->codes.pop();
        emit(OP_LOAD_FUNCTION, co()->add_const(VAR(func)));
        if(name_scope() == NAME_LOCAL) emit(OP_SETUP_CLOSURE);
    }

    void exprAssign() {
        int lhs = co()->codes.empty() ? -1 : co()->codes.size() - 1;
        co()->_rvalue += 1;
        TokenIndex op = parser->prev.type;
        if(op == TK("=")) {     // a = (expr)
            EXPR_TUPLE();
            if(lhs!=-1 && co()->codes[lhs].op == OP_LOAD_NAME_REF){
                if(co()->_is_compiling_class){
                    emit(OP_STORE_CLASS_ATTR, co()->codes[lhs].arg);
                }else{
                    emit(OP_STORE_NAME, co()->codes[lhs].arg);
                }
                co()->codes[lhs].op = OP_NO_OP;
                co()->codes[lhs].arg = -1;
            }else{
                if(co()->_is_compiling_class) SyntaxError();
                emit(OP_STORE_REF);
            }
        }else{                  // a += (expr) -> a = a + (expr)
            if(co()->_is_compiling_class) SyntaxError();
            EXPR();
            switch (op) {
                case TK("+="):      emit(OP_INPLACE_BINARY_OP, 0);  break;
                case TK("-="):      emit(OP_INPLACE_BINARY_OP, 1);  break;
                case TK("*="):      emit(OP_INPLACE_BINARY_OP, 2);  break;
                case TK("/="):      emit(OP_INPLACE_BINARY_OP, 3);  break;
                case TK("//="):     emit(OP_INPLACE_BINARY_OP, 4);  break;
                case TK("%="):      emit(OP_INPLACE_BINARY_OP, 5);  break;
                case TK("<<="):     emit(OP_INPLACE_BITWISE_OP, 0);  break;
                case TK(">>="):     emit(OP_INPLACE_BITWISE_OP, 1);  break;
                case TK("&="):      emit(OP_INPLACE_BITWISE_OP, 2);  break;
                case TK("|="):      emit(OP_INPLACE_BITWISE_OP, 3);  break;
                case TK("^="):      emit(OP_INPLACE_BITWISE_OP, 4);  break;
                default: UNREACHABLE();
            }
        }
        co()->_rvalue -= 1;
    }

    void exprComma() {
        int size = 1;       // an expr is in the stack now
        do {
            EXPR();         // NOTE: "1," will fail, "1,2" will be ok
            size++;
        } while(match(TK(",")));
        emit(co()->_rvalue ? OP_BUILD_TUPLE : OP_BUILD_TUPLE_REF, size);
    }

    void exprOr() {
        int patch = emit(OP_JUMP_IF_TRUE_OR_POP);
        parse_expression(PREC_LOGICAL_OR);
        patch_jump(patch);
    }

    void exprAnd() {
        int patch = emit(OP_JUMP_IF_FALSE_OR_POP);
        parse_expression(PREC_LOGICAL_AND);
        patch_jump(patch);
    }

    void exprTernary() {
        int patch = emit(OP_POP_JUMP_IF_FALSE);
        EXPR();         // if true
        int patch2 = emit(OP_JUMP_ABSOLUTE);
        consume(TK(":"));
        patch_jump(patch);
        EXPR();         // if false
        patch_jump(patch2);
    }

    void exprBinaryOp() {
        TokenIndex op = parser->prev.type;
        parse_expression((Precedence)(rules[op].precedence + 1));

        switch (op) {
            case TK("+"):   emit(OP_BINARY_OP, 0);  break;
            case TK("-"):   emit(OP_BINARY_OP, 1);  break;
            case TK("*"):   emit(OP_BINARY_OP, 2);  break;
            case TK("/"):   emit(OP_BINARY_OP, 3);  break;
            case TK("//"):  emit(OP_BINARY_OP, 4);  break;
            case TK("%"):   emit(OP_BINARY_OP, 5);  break;
            case TK("**"):  emit(OP_BINARY_OP, 6);  break;

            case TK("<"):   emit(OP_COMPARE_OP, 0);    break;
            case TK("<="):  emit(OP_COMPARE_OP, 1);    break;
            case TK("=="):  emit(OP_COMPARE_OP, 2);    break;
            case TK("!="):  emit(OP_COMPARE_OP, 3);    break;
            case TK(">"):   emit(OP_COMPARE_OP, 4);    break;
            case TK(">="):  emit(OP_COMPARE_OP, 5);    break;
            case TK("in"):      emit(OP_CONTAINS_OP, 0);   break;
            case TK("not in"):  emit(OP_CONTAINS_OP, 1);   break;
            case TK("is"):      emit(OP_IS_OP, 0);         break;
            case TK("is not"):  emit(OP_IS_OP, 1);         break;

            case TK("<<"):  emit(OP_BITWISE_OP, 0);    break;
            case TK(">>"):  emit(OP_BITWISE_OP, 1);    break;
            case TK("&"):   emit(OP_BITWISE_OP, 2);    break;
            case TK("|"):   emit(OP_BITWISE_OP, 3);    break;
            case TK("^"):   emit(OP_BITWISE_OP, 4);    break;
            default: UNREACHABLE();
        }
    }

    void exprUnaryOp() {
        TokenIndex op = parser->prev.type;
        parse_expression((Precedence)(PREC_UNARY + 1));
        switch (op) {
            case TK("-"):     emit(OP_UNARY_NEGATIVE); break;
            case TK("not"):   emit(OP_UNARY_NOT);      break;
            case TK("*"):     emit(OP_UNARY_STAR, co()->_rvalue);   break;
            default: UNREACHABLE();
        }
    }

    void exprGrouping() {
        match_newlines(mode()==REPL_MODE);
        EXPR_TUPLE();
        match_newlines(mode()==REPL_MODE);
        consume(TK(")"));
    }

    void exprList() {
        int _patch = emit(OP_NO_OP);
        int _body_start = co()->codes.size();
        int ARGC = 0;
        do {
            match_newlines(mode()==REPL_MODE);
            if (peek() == TK("]")) break;
            EXPR(); ARGC++;
            match_newlines(mode()==REPL_MODE);
            if(ARGC == 1 && match(TK("for"))) goto __LISTCOMP;
        } while (match(TK(",")));
        match_newlines(mode()==REPL_MODE);
        consume(TK("]"));
        emit(OP_BUILD_LIST, ARGC);
        return;

__LISTCOMP:
        int _body_end_return = emit(OP_JUMP_ABSOLUTE, -1);
        int _body_end = co()->codes.size();
        co()->codes[_patch].op = OP_JUMP_ABSOLUTE;
        co()->codes[_patch].arg = _body_end;
        emit(OP_BUILD_LIST, 0);
        EXPR_FOR_VARS();consume(TK("in"));EXPR_TUPLE();
        match_newlines(mode()==REPL_MODE);
        
        int _skipPatch = emit(OP_JUMP_ABSOLUTE);
        int _cond_start = co()->codes.size();
        int _cond_end_return = -1;
        if(match(TK("if"))) {
            EXPR_TUPLE();
            _cond_end_return = emit(OP_JUMP_ABSOLUTE, -1);
        }
        patch_jump(_skipPatch);

        emit(OP_GET_ITER);
        co()->_enter_block(FOR_LOOP);
        emit(OP_FOR_ITER);

        if(_cond_end_return != -1) {      // there is an if condition
            emit(OP_JUMP_ABSOLUTE, _cond_start);
            patch_jump(_cond_end_return);
            int ifpatch = emit(OP_POP_JUMP_IF_FALSE);
            emit(OP_JUMP_ABSOLUTE, _body_start);
            patch_jump(_body_end_return);
            emit(OP_LIST_APPEND);
            patch_jump(ifpatch);
        }else{
            emit(OP_JUMP_ABSOLUTE, _body_start);
            patch_jump(_body_end_return);
            emit(OP_LIST_APPEND);
        }

        emit(OP_LOOP_CONTINUE, -1, true);
        co()->_exit_block();
        match_newlines(mode()==REPL_MODE);
        consume(TK("]"));
    }

    void exprMap() {
        bool parsing_dict = false;
        int size = 0;
        do {
            match_newlines(mode()==REPL_MODE);
            if (peek() == TK("}")) break;
            EXPR();
            if(peek() == TK(":")) parsing_dict = true;
            if(parsing_dict){
                consume(TK(":"));
                EXPR();
            }
            size++;
            match_newlines(mode()==REPL_MODE);
        } while (match(TK(",")));
        consume(TK("}"));

        if(size == 0 || parsing_dict) emit(OP_BUILD_MAP, size);
        else emit(OP_BUILD_SET, size);
    }

    void exprCall() {
        int ARGC = 0;
        int KWARGC = 0;
        bool need_unpack = false;
        do {
            match_newlines(mode()==REPL_MODE);
            if (peek() == TK(")")) break;
            if(peek() == TK("@id") && peek_next() == TK("=")) {
                consume(TK("@id"));
                const Str& key = parser->prev.str();
                emit(OP_LOAD_CONST, co()->add_const(VAR(key)));
                consume(TK("="));
                co()->_rvalue += 1; EXPR(); co()->_rvalue -= 1;
                KWARGC++;
            } else{
                if(KWARGC > 0) SyntaxError("positional argument follows keyword argument");
                co()->_rvalue += 1; EXPR(); co()->_rvalue -= 1;
                if(co()->codes.back().op == OP_UNARY_STAR) need_unpack = true;
                ARGC++;
            }
            match_newlines(mode()==REPL_MODE);
        } while (match(TK(",")));
        consume(TK(")"));
        if(ARGC > 32767) SyntaxError("too many positional arguments");
        if(KWARGC > 32767) SyntaxError("too many keyword arguments");
        if(KWARGC > 0){
            emit(need_unpack ? OP_CALL_KWARGS_UNPACK : OP_CALL_KWARGS, (KWARGC << 16) | ARGC);
        }else{
            emit(need_unpack ? OP_CALL_UNPACK : OP_CALL, ARGC);
        }
    }

    void exprName(){ _exprName(false); }

    void _exprName(bool force_lvalue) {
        Token tkname = parser->prev;
        int index = co()->add_name(tkname.str(), name_scope());
        bool fast_load = !force_lvalue && co()->_rvalue>0;
        emit(fast_load ? OP_LOAD_NAME : OP_LOAD_NAME_REF, index);
    }

    void exprAttrib() {
        consume(TK("@id"));
        const Str& name = parser->prev.str();
        int index = co()->add_name(name, NAME_ATTR);
        emit(co()->_rvalue ? OP_BUILD_ATTR : OP_BUILD_ATTR_REF, index);
    }

    // [:], [:b]
    // [a], [a:], [a:b]
    void exprSubscript() {
        if(match(TK(":"))){
            emit(OP_LOAD_NONE);
            if(match(TK("]"))){
                emit(OP_LOAD_NONE);
            }else{
                EXPR_TUPLE();
                consume(TK("]"));
            }
            emit(OP_BUILD_SLICE);
        }else{
            EXPR_TUPLE();
            if(match(TK(":"))){
                if(match(TK("]"))){
                    emit(OP_LOAD_NONE);
                }else{
                    EXPR_TUPLE();
                    consume(TK("]"));
                }
                emit(OP_BUILD_SLICE);
            }else{
                consume(TK("]"));
            }
        }

        emit(OP_BUILD_INDEX, (int)(co()->_rvalue>0));
    }

    void exprValue() {
        TokenIndex op = parser->prev.type;
        switch (op) {
            case TK("None"):    emit(OP_LOAD_NONE);  break;
            case TK("True"):    emit(OP_LOAD_TRUE);  break;
            case TK("False"):   emit(OP_LOAD_FALSE); break;
            case TK("..."):     emit(OP_LOAD_ELLIPSIS); break;
            default: UNREACHABLE();
        }
    }

    int emit(Opcode opcode, int arg=-1, bool keepline=false) {
        int line = parser->prev.line;
        co()->codes.push_back(
            Bytecode{(uint8_t)opcode, arg, line, (uint16_t)co()->_curr_block_i}
        );
        int i = co()->codes.size() - 1;
        if(keepline && i>=1) co()->codes[i].line = co()->codes[i-1].line;
        return i;
    }

    inline void patch_jump(int addr_index) {
        int target = co()->codes.size();
        co()->codes[addr_index].arg = target;
    }

    void compile_block_body(CompilerAction action=nullptr) {
        if(action == nullptr) action = &Compiler::compile_stmt;
        consume(TK(":"));
        if(peek()!=TK("@eol") && peek()!=TK("@eof")){
            (this->*action)();  // inline block
            return;
        }
        if(!match_newlines(mode()==REPL_MODE)){
            SyntaxError("expected a new line after ':'");
        }
        consume(TK("@indent"));
        while (peek() != TK("@dedent")) {
            match_newlines();
            (this->*action)();
            match_newlines();
        }
        consume(TK("@dedent"));
    }

    Token _compile_import() {
        consume(TK("@id"));
        Token tkmodule = parser->prev;
        int index = co()->add_name(tkmodule.str(), NAME_SPECIAL);
        emit(OP_IMPORT_NAME, index);
        return tkmodule;
    }

    // import a as b
    void compile_normal_import() {
        do {
            Token tkmodule = _compile_import();
            if (match(TK("as"))) {
                consume(TK("@id"));
                tkmodule = parser->prev;
            }
            int index = co()->add_name(tkmodule.str(), name_scope());
            emit(OP_STORE_NAME, index);
        } while (match(TK(",")));
        consume_end_stmt();
    }

    // from a import b as c, d as e
    void compile_from_import() {
        Token tkmodule = _compile_import();
        consume(TK("import"));
        if (match(TK("*"))) {
            if(name_scope() != NAME_GLOBAL) SyntaxError("import * can only be used in global scope");
            emit(OP_STORE_ALL_NAMES);
            consume_end_stmt();
            return;
        }
        do {
            emit(OP_DUP_TOP_VALUE);
            consume(TK("@id"));
            Token tkname = parser->prev;
            int index = co()->add_name(tkname.str(), NAME_ATTR);
            emit(OP_BUILD_ATTR, index);
            if (match(TK("as"))) {
                consume(TK("@id"));
                tkname = parser->prev;
            }
            index = co()->add_name(tkname.str(), name_scope());
            emit(OP_STORE_NAME, index);
        } while (match(TK(",")));
        emit(OP_POP_TOP);
        consume_end_stmt();
    }

    void parse_expression(Precedence precedence) {
        lex_token();
        GrammarFn prefix = rules[parser->prev.type].prefix;
        if (prefix == nullptr) SyntaxError(Str("expected an expression, but got ") + TK_STR(parser->prev.type));
        (this->*prefix)();
        bool meet_assign_token = false;
        while (rules[peek()].precedence >= precedence) {
            lex_token();
            TokenIndex op = parser->prev.type;
            if (op == TK("=")){
                if(meet_assign_token) SyntaxError();
                meet_assign_token = true;
            }
            GrammarFn infix = rules[op].infix;
            if(infix == nullptr) throw std::runtime_error("(infix == nullptr) is true");
            (this->*infix)();
        }
    }

    void compile_if_stmt() {
        match_newlines();
        co()->_rvalue += 1;
        EXPR_TUPLE();   // condition
        co()->_rvalue -= 1;
        int ifpatch = emit(OP_POP_JUMP_IF_FALSE);
        compile_block_body();

        if (match(TK("elif"))) {
            int exit_jump = emit(OP_JUMP_ABSOLUTE);
            patch_jump(ifpatch);
            compile_if_stmt();
            patch_jump(exit_jump);
        } else if (match(TK("else"))) {
            int exit_jump = emit(OP_JUMP_ABSOLUTE);
            patch_jump(ifpatch);
            compile_block_body();
            patch_jump(exit_jump);
        } else {
            patch_jump(ifpatch);
        }
    }

    void compile_while_loop() {
        co()->_enter_block(WHILE_LOOP);
        co()->_rvalue += 1;
        EXPR_TUPLE();   // condition
        co()->_rvalue -= 1;
        int patch = emit(OP_POP_JUMP_IF_FALSE);
        compile_block_body();
        emit(OP_LOOP_CONTINUE, -1, true);
        patch_jump(patch);
        co()->_exit_block();
    }

    void EXPR_FOR_VARS(){
        int size = 0;
        do {
            consume(TK("@id"));
            _exprName(true); size++;
        } while (match(TK(",")));
        if(size > 1) emit(OP_BUILD_TUPLE_REF, size);
    }

    void compile_for_loop() {
        EXPR_FOR_VARS();consume(TK("in"));
        co()->_rvalue += 1; EXPR_TUPLE(); co()->_rvalue -= 1;
        emit(OP_GET_ITER);
        co()->_enter_block(FOR_LOOP);
        emit(OP_FOR_ITER);
        compile_block_body();
        emit(OP_LOOP_CONTINUE, -1, true);
        co()->_exit_block();
    }

    void compile_try_except() {
        co()->_enter_block(TRY_EXCEPT);
        emit(OP_TRY_BLOCK_ENTER);
        compile_block_body();
        emit(OP_TRY_BLOCK_EXIT);
        std::vector<int> patches = { emit(OP_JUMP_ABSOLUTE) };
        co()->_exit_block();

        do {
            consume(TK("except"));
            if(match(TK("@id"))){
                int name_idx = co()->add_name(parser->prev.str(), NAME_SPECIAL);
                emit(OP_EXCEPTION_MATCH, name_idx);
            }else{
                emit(OP_LOAD_TRUE);
            }
            int patch = emit(OP_POP_JUMP_IF_FALSE);
            emit(OP_POP_TOP);       // pop the exception on match
            compile_block_body();
            patches.push_back(emit(OP_JUMP_ABSOLUTE));
            patch_jump(patch);
        }while(peek() == TK("except"));
        emit(OP_RE_RAISE);      // no match, re-raise
        for (int patch : patches) patch_jump(patch);
    }

    void compile_stmt() {
        if (match(TK("break"))) {
            if (!co()->_is_curr_block_loop()) SyntaxError("'break' outside loop");
            consume_end_stmt();
            emit(OP_LOOP_BREAK);
        } else if (match(TK("continue"))) {
            if (!co()->_is_curr_block_loop()) SyntaxError("'continue' not properly in loop");
            consume_end_stmt();
            emit(OP_LOOP_CONTINUE);
        } else if (match(TK("yield"))) {
            if (codes.size() == 1) SyntaxError("'yield' outside function");
            co()->_rvalue += 1;
            EXPR_TUPLE();
            co()->_rvalue -= 1;
            consume_end_stmt();
            co()->is_generator = true;
            emit(OP_YIELD_VALUE, -1, true);
        } else if (match(TK("return"))) {
            if (codes.size() == 1) SyntaxError("'return' outside function");
            if(match_end_stmt()){
                emit(OP_LOAD_NONE);
            }else{
                co()->_rvalue += 1;
                EXPR_TUPLE();   // return value
                co()->_rvalue -= 1;
                consume_end_stmt();
            }
            emit(OP_RETURN_VALUE, -1, true);
        } else if (match(TK("if"))) {
            compile_if_stmt();
        } else if (match(TK("while"))) {
            compile_while_loop();
        } else if (match(TK("for"))) {
            compile_for_loop();
        } else if (match(TK("import"))){
            compile_normal_import();
        } else if (match(TK("from"))){
            compile_from_import();
        } else if (match(TK("def"))){
            compile_function();
        } else if (match(TK("@"))){
            EXPR();
            if(!match_newlines(mode()==REPL_MODE)){
                SyntaxError("expected a new line after '@'");
            }
            emit(OP_SETUP_DECORATOR);
            consume(TK("def"));
            compile_function();
        } else if (match(TK("try"))) {
            compile_try_except();
        } else if(match(TK("assert"))) {
            co()->_rvalue += 1;
            EXPR();
            if (match(TK(","))) EXPR();
            else emit(OP_LOAD_CONST, co()->add_const(VAR("")));
            co()->_rvalue -= 1;
            emit(OP_ASSERT);
            consume_end_stmt();
        } else if(match(TK("with"))){
            EXPR();
            consume(TK("as"));
            consume(TK("@id"));
            Token tkname = parser->prev;
            int index = co()->add_name(tkname.str(), name_scope());
            emit(OP_STORE_NAME, index);
            emit(OP_LOAD_NAME_REF, index);
            emit(OP_WITH_ENTER);
            compile_block_body();
            emit(OP_LOAD_NAME_REF, index);
            emit(OP_WITH_EXIT);
        } else if(match(TK("label"))){
            if(mode() != EXEC_MODE) SyntaxError("'label' is only available in EXEC_MODE");
            consume(TK(".")); consume(TK("@id"));
            Str label = parser->prev.str();
            bool ok = co()->add_label(label);
            if(!ok) SyntaxError("label '" + label + "' already exists");
            consume_end_stmt();
        } else if(match(TK("goto"))){ // https://entrian.com/goto/
            if(mode() != EXEC_MODE) SyntaxError("'goto' is only available in EXEC_MODE");
            consume(TK(".")); consume(TK("@id"));
            emit(OP_GOTO, co()->add_name(parser->prev.str(), NAME_SPECIAL));
            consume_end_stmt();
        } else if(match(TK("raise"))){
            consume(TK("@id"));
            int dummy_t = co()->add_name(parser->prev.str(), NAME_SPECIAL);
            if(match(TK("(")) && !match(TK(")"))){
                EXPR(); consume(TK(")"));
            }else{
                emit(OP_LOAD_NONE);
            }
            emit(OP_RAISE, dummy_t);
            consume_end_stmt();
        } else if(match(TK("del"))){
            EXPR_TUPLE();
            emit(OP_DELETE_REF);
            consume_end_stmt();
        } else if(match(TK("global"))){
            do {
                consume(TK("@id"));
                co()->global_names[parser->prev.str()] = 1;
            } while (match(TK(",")));
            consume_end_stmt();
        } else if(match(TK("pass"))){
            consume_end_stmt();
        } else {
            EXPR_ANY();
            consume_end_stmt();
            // If last op is not an assignment, pop the result.
            uint8_t last_op = co()->codes.back().op;
            if( last_op!=OP_STORE_NAME && last_op!=OP_STORE_REF &&
            last_op!=OP_INPLACE_BINARY_OP && last_op!=OP_INPLACE_BITWISE_OP &&
            last_op!=OP_STORE_ALL_NAMES && last_op!=OP_STORE_CLASS_ATTR){
                if(last_op == OP_BUILD_TUPLE_REF) co()->codes.back().op = OP_BUILD_TUPLE;
                if(mode()==REPL_MODE && name_scope() == NAME_GLOBAL) emit(OP_PRINT_EXPR, -1, true);
                emit(OP_POP_TOP, -1, true);
            }
        }
    }

    void compile_class(){
        consume(TK("@id"));
        int cls_name_idx = co()->add_name(parser->prev.str(), NAME_GLOBAL);
        int super_cls_name_idx = -1;
        if(match(TK("(")) && match(TK("@id"))){
            super_cls_name_idx = co()->add_name(parser->prev.str(), NAME_GLOBAL);
            consume(TK(")"));
        }
        if(super_cls_name_idx == -1) emit(OP_LOAD_NONE);
        else emit(OP_LOAD_NAME, super_cls_name_idx);
        emit(OP_BEGIN_CLASS, cls_name_idx);
        co()->_is_compiling_class = true;
        compile_block_body();
        co()->_is_compiling_class = false;
        emit(OP_END_CLASS);
    }

    void _compile_f_args(Function& func, bool enable_type_hints){
        int state = 0;      // 0 for args, 1 for *args, 2 for k=v, 3 for **kwargs
        do {
            if(state == 3) SyntaxError("**kwargs should be the last argument");
            match_newlines();
            if(match(TK("*"))){
                if(state < 1) state = 1;
                else SyntaxError("*args should be placed before **kwargs");
            }
            else if(match(TK("**"))){
                state = 3;
            }

            consume(TK("@id"));
            const Str& name = parser->prev.str();
            if(func.has_name(name)) SyntaxError("duplicate argument name");

            // eat type hints
            if(enable_type_hints && match(TK(":"))) consume(TK("@id"));

            if(state == 0 && peek() == TK("=")) state = 2;

            switch (state)
            {
                case 0: func.args.push_back(name); break;
                case 1: func.starred_arg = name; state+=1; break;
                case 2: {
                    consume(TK("="));
                    PyVarOrNull value = read_literal();
                    if(value == nullptr){
                        SyntaxError(Str("expect a literal, not ") + TK_STR(parser->curr.type));
                    }
                    func.kwargs.set(name, value);
                    func.kwargs_order.push_back(name);
                } break;
                case 3: SyntaxError("**kwargs is not supported yet"); break;
            }
        } while (match(TK(",")));
    }

    void compile_function(){
        bool has_decorator = !co()->codes.empty() && co()->codes.back().op == OP_SETUP_DECORATOR;
        Function func;
        StrName obj_name;
        consume(TK("@id"));
        func.name = parser->prev.str();
        if(!co()->_is_compiling_class && match(TK("::"))){
            consume(TK("@id"));
            obj_name = func.name;
            func.name = parser->prev.str();
        }
        consume(TK("("));
        if (!match(TK(")"))) {
            _compile_f_args(func, true);
            consume(TK(")"));
        }
        if(match(TK("->"))) consume(TK("@id")); // eat type hints
        func.code = make_sp<CodeObject>(parser->src, func.name.str());
        this->codes.push(func.code);
        compile_block_body();
        func.code->optimize(vm);
        this->codes.pop();
        emit(OP_LOAD_FUNCTION, co()->add_const(VAR(func)));
        if(name_scope() == NAME_LOCAL) emit(OP_SETUP_CLOSURE);
        if(!co()->_is_compiling_class){
            if(obj_name.empty()){
                if(has_decorator) emit(OP_CALL, 1);
                emit(OP_STORE_NAME, co()->add_name(func.name, name_scope()));
            } else {
                if(has_decorator) SyntaxError("decorator is not supported here");
                emit(OP_LOAD_NAME, co()->add_name(obj_name, name_scope()));
                int index = co()->add_name(func.name, NAME_ATTR);
                emit(OP_BUILD_ATTR_REF, index);
                emit(OP_ROT_TWO);
                emit(OP_STORE_REF);
            }
        }else{
            if(has_decorator) emit(OP_CALL, 1);
            emit(OP_STORE_CLASS_ATTR, co()->add_name(func.name, name_scope()));
        }
    }

    PyVarOrNull read_literal(){
        if(match(TK("-"))){
            consume(TK("@num"));
            PyVar val = parser->prev.value;
            return vm->num_negated(val);
        }
        if(match(TK("@num"))) return parser->prev.value;
        if(match(TK("@str"))) return parser->prev.value;
        if(match(TK("True"))) return VAR(true);
        if(match(TK("False"))) return VAR(false);
        if(match(TK("None"))) return vm->None;
        if(match(TK("..."))) return vm->Ellipsis;
        return nullptr;
    }

    /***** Error Reporter *****/
    void throw_err(Str type, Str msg){
        int lineno = parser->curr.line;
        const char* cursor = parser->curr.start;
        // if error occurs in lexing, lineno should be `parser->current_line`
        if(lexing_count > 0){
            lineno = parser->current_line;
            cursor = parser->curr_char;
        }
        if(parser->peekchar() == '\n') lineno--;
        auto e = Exception("SyntaxError", msg);
        e.st_push(parser->src->snapshot(lineno, cursor));
        throw e;
    }
    void SyntaxError(Str msg){ throw_err("SyntaxError", msg); }
    void SyntaxError(){ throw_err("SyntaxError", "invalid syntax"); }
    void IndentationError(Str msg){ throw_err("IndentationError", msg); }

public:
    CodeObject_ compile(){
        // can only be called once
        if(used) UNREACHABLE();
        used = true;

        CodeObject_ code = make_sp<CodeObject>(parser->src, Str("<module>"));
        codes.push(code);

        lex_token(); lex_token();
        match_newlines();

        if(mode()==EVAL_MODE) {
            EXPR_TUPLE();
            consume(TK("@eof"));
            code->optimize(vm);
            return code;
        }else if(mode()==JSON_MODE){
            PyVarOrNull value = read_literal();
            if(value != nullptr) emit(OP_LOAD_CONST, code->add_const(value));
            else if(match(TK("{"))) exprMap();
            else if(match(TK("["))) exprList();
            else SyntaxError("expect a JSON object or array");
            consume(TK("@eof"));
            return code;    // no need to optimize for JSON decoding
        }

        while (!match(TK("@eof"))) {
            if (match(TK("class"))) {
                compile_class();
            } else {
                compile_stmt();
            }
            match_newlines();
        }
        code->optimize(vm);
        return code;
    }
};

} // namespace pkpy


namespace pkpy{

class REPL {
protected:
    int need_more_lines = 0;
    std::string buffer;
    VM* vm;
public:
    REPL(VM* vm) : vm(vm){
        (*vm->_stdout) << ("pocketpy " PK_VERSION " (" __DATE__ ", " __TIME__ ")\n");
        (*vm->_stdout) << ("https://github.com/blueloveTH/pocketpy" "\n");
        (*vm->_stdout) << ("Type \"exit()\" to exit." "\n");
    }

    bool input(std::string line){
        CompileMode mode = REPL_MODE;
        if(need_more_lines){
            buffer += line;
            buffer += '\n';
            int n = buffer.size();
            if(n>=need_more_lines){
                for(int i=buffer.size()-need_more_lines; i<buffer.size(); i++){
                    // no enough lines
                    if(buffer[i] != '\n') return true;
                }
                need_more_lines = 0;
                line = buffer;
                buffer.clear();
                mode = EXEC_MODE;
            }else{
                return true;
            }
        }
        
        try{
            vm->exec(line, "<stdin>", mode);
        }catch(NeedMoreLines& ne){
            buffer += line;
            buffer += '\n';
            need_more_lines = ne.is_compiling_class ? 3 : 2;
            if (need_more_lines) return true;
        }
        return false;
    }
};

} // namespace pkpy


namespace pkpy{

class RangeIter : public BaseIter {
    i64 current;
    Range r;
public:
    RangeIter(VM* vm, PyVar _ref) : BaseIter(vm, _ref) {
        this->r = OBJ_GET(Range, _ref);
        this->current = r.start;
    }

    inline bool _has_next(){
        return r.step > 0 ? current < r.stop : current > r.stop;
    }

    PyVar next(){
        if(!_has_next()) return nullptr;
        current += r.step;
        return VAR(current-r.step);
    }
};

template <typename T>
class ArrayIter : public BaseIter {
    size_t index = 0;
    const T* p;
public:
    ArrayIter(VM* vm, PyVar _ref) : BaseIter(vm, _ref) { p = &OBJ_GET(T, _ref);}
    PyVar next(){
        if(index == p->size()) return nullptr;
        return p->operator[](index++); 
    }
};

class StringIter : public BaseIter {
    int index = 0;
    Str* str;
public:
    StringIter(VM* vm, PyVar _ref) : BaseIter(vm, _ref) {
        str = &OBJ_GET(Str, _ref);
    }

    PyVar next() {
        if(index == str->u8_length()) return nullptr;
        return VAR(str->u8_getitem(index++));
    }
};

PyVar Generator::next(){
    if(state == 2) return nullptr;
    vm->callstack.push(std::move(frame));
    PyVar ret = vm->_exec();
    if(ret == vm->_py_op_yield){
        frame = std::move(vm->callstack.top());
        vm->callstack.pop();
        state = 1;
        return frame->pop_value(vm);
    }else{
        state = 2;
        return nullptr;
    }
}

} // namespace pkpy


#include <type_traits>
#include <vector>

namespace pkpy {

template<typename Ret, typename... Params>
struct NativeProxyFunc {
    using T = Ret(*)(Params...);
    // using T = std::function<Ret(Params...)>;
    static constexpr int N = sizeof...(Params);
    T func;
    NativeProxyFunc(T func) : func(func) {}

    PyVar operator()(VM* vm, Args& args) {
        if (args.size() != N) {
            vm->TypeError("expected " + std::to_string(N) + " arguments, but got " + std::to_string(args.size()));
        }
        return call<Ret>(vm, args, std::make_index_sequence<N>());
    }

    template<typename __Ret, size_t... Is>
    std::enable_if_t<std::is_void_v<__Ret>, PyVar> call(VM* vm, Args& args, std::index_sequence<Is...>) {
        func(py_cast<Params>(vm, args[Is])...);
        return vm->None;
    }

    template<typename __Ret, size_t... Is>
    std::enable_if_t<!std::is_void_v<__Ret>, PyVar> call(VM* vm, Args& args, std::index_sequence<Is...>) {
        __Ret ret = func(py_cast<Params>(vm, args[Is])...);
        return VAR(std::move(ret));
    }
};

template<typename T>
constexpr int type_index() { return 0; }
template<> constexpr int type_index<void>() { return 1; }
template<> constexpr int type_index<char>() { return 2; }
template<> constexpr int type_index<short>() { return 3; }
template<> constexpr int type_index<int>() { return 4; }
template<> constexpr int type_index<long>() { return 5; }
template<> constexpr int type_index<long long>() { return 6; }
template<> constexpr int type_index<unsigned char>() { return 7; }
template<> constexpr int type_index<unsigned short>() { return 8; }
template<> constexpr int type_index<unsigned int>() { return 9; }
template<> constexpr int type_index<unsigned long>() { return 10; }
template<> constexpr int type_index<unsigned long long>() { return 11; }
template<> constexpr int type_index<float>() { return 12; }
template<> constexpr int type_index<double>() { return 13; }
template<> constexpr int type_index<bool>() { return 14; }

template<typename T>
struct TypeId{ inline static int id; };

struct TypeInfo;

struct MemberInfo{
    const TypeInfo* type;
    int offset;
};

struct TypeInfo{
    const char* name;
    int size;
    int index;
    std::map<StrName, MemberInfo> members;
};

struct Vec2 {
    float x, y;
};

struct TypeDB{
    std::vector<TypeInfo> _by_index;
    std::map<std::string_view, int> _by_name;

    template<typename T>
    void register_type(const char name[], std::map<StrName, MemberInfo>&& members){
        TypeInfo ti;
        ti.name = name;
        if constexpr(std::is_same_v<T, void>) ti.size = 1;
        else ti.size = sizeof(T);
        ti.members = std::move(members);
        TypeId<T>::id = ti.index = _by_index.size()+1;    // 0 is reserved for NULL
        _by_name[name] = ti.index;
        _by_index.push_back(ti);
    }

    const TypeInfo* get(int index) const {
        return index == 0 ? nullptr : &_by_index[index-1];
    }

    const TypeInfo* get(const char name[]) const {
        auto it = _by_name.find(name);
        if(it == _by_name.end()) return nullptr;
        return get(it->second);
    }

    const TypeInfo* get(const Str& s) const {
        return get(s.c_str());
    }

    template<typename T>
    const TypeInfo* get() const {
        return get(TypeId<std::decay_t<T>>::id);
    }
};

static TypeDB _type_db;


auto _ = [](){
    #define REGISTER_BASIC_TYPE(T) _type_db.register_type<T>(#T, {});
    _type_db.register_type<void>("void", {});
    REGISTER_BASIC_TYPE(char);
    REGISTER_BASIC_TYPE(short);
    REGISTER_BASIC_TYPE(int);
    REGISTER_BASIC_TYPE(long);
    REGISTER_BASIC_TYPE(long long);
    REGISTER_BASIC_TYPE(unsigned char);
    REGISTER_BASIC_TYPE(unsigned short);
    REGISTER_BASIC_TYPE(unsigned int);
    REGISTER_BASIC_TYPE(unsigned long);
    REGISTER_BASIC_TYPE(unsigned long long);
    REGISTER_BASIC_TYPE(float);
    REGISTER_BASIC_TYPE(double);
    REGISTER_BASIC_TYPE(bool);
    #undef REGISTER_BASIC_TYPE

    _type_db.register_type<Vec2>("Vec2", {
        {"x", { _type_db.get<float>(), offsetof(Vec2, x) }},
        {"y", { _type_db.get<float>(), offsetof(Vec2, y) }},
    });
    return 0;
}();

struct Pointer{
    PY_CLASS(Pointer, c, _ptr)

    const TypeInfo* ctype;      // this is immutable
    int level;                  // level of pointer
    char* ptr;

    i64 unit_size() const {
        return level == 1 ? ctype->size : sizeof(void*);
    }

    Pointer() : ctype(_type_db.get<void>()), level(1), ptr(nullptr) {}
    Pointer(const TypeInfo* ctype, int level, char* ptr): ctype(ctype), level(level), ptr(ptr) {}
    Pointer(const TypeInfo* ctype, char* ptr): ctype(ctype), level(1), ptr(ptr) {}

    Pointer operator+(i64 offset) const { 
        return Pointer(ctype, level, ptr+offset*unit_size());
    }

    Pointer operator-(i64 offset) const { 
        return Pointer(ctype, level, ptr-offset*unit_size());
    }

    static void _register(VM* vm, PyVar mod, PyVar type){
        vm->bind_static_method<-1>(type, "__new__", CPP_NOT_IMPLEMENTED());

        vm->bind_method<0>(type, "__repr__", [](VM* vm, Args& args) {
            Pointer& self = CAST(Pointer&, args[0]);
            StrStream ss;
            ss << "<" << self.ctype->name;
            for(int i=0; i<self.level; i++) ss << "*";
            ss << " at " << (i64)self.ptr << ">";
            return VAR(ss.str());
        });

        vm->bind_method<1>(type, "__add__", [](VM* vm, Args& args) {
            Pointer& self = CAST(Pointer&, args[0]);
            return VAR_T(Pointer, self + CAST(i64, args[1]));
        });

        vm->bind_method<1>(type, "__sub__", [](VM* vm, Args& args) {
            Pointer& self = CAST(Pointer&, args[0]);
            return VAR_T(Pointer, self - CAST(i64, args[1]));
        });

        vm->bind_method<1>(type, "__eq__", [](VM* vm, Args& args) {
            Pointer& self = CAST(Pointer&, args[0]);
            Pointer& other = CAST(Pointer&, args[1]);
            return VAR(self.ptr == other.ptr);
        });

        vm->bind_method<1>(type, "__ne__", [](VM* vm, Args& args) {
            Pointer& self = CAST(Pointer&, args[0]);
            Pointer& other = CAST(Pointer&, args[1]);
            return VAR(self.ptr != other.ptr);
        });

        vm->bind_method<1>(type, "__getitem__", [](VM* vm, Args& args) {
            Pointer& self = CAST(Pointer&, args[0]);
            i64 index = CAST(i64, args[1]);
            return (self+index).get(vm);
        });

        vm->bind_method<2>(type, "__setitem__", [](VM* vm, Args& args) {
            Pointer& self = CAST(Pointer&, args[0]);
            i64 index = CAST(i64, args[1]);
            (self+index).set(vm, args[2]);
            return vm->None;
        });

        vm->bind_method<1>(type, "__getattr__", [](VM* vm, Args& args) {
            Pointer& self = CAST(Pointer&, args[0]);
            const Str& name = CAST(Str&, args[1]);
            return VAR_T(Pointer, self._to(vm, name));
        });

        vm->bind_method<0>(type, "get", [](VM* vm, Args& args) {
            Pointer& self = CAST(Pointer&, args[0]);
            return self.get(vm);
        });

        vm->bind_method<1>(type, "set", [](VM* vm, Args& args) {
            Pointer& self = CAST(Pointer&, args[0]);
            self.set(vm, args[1]);
            return vm->None;
        });
    }

    template<typename T>
    inline T& ref() noexcept { return *reinterpret_cast<T*>(ptr); }

    PyVar get(VM* vm){
        if(level > 1) return VAR_T(Pointer, ctype, level-1, ref<char*>());
        switch(ctype->index){
#define CASE(T) case type_index<T>(): return VAR(ref<T>())
            case type_index<void>(): vm->ValueError("cannot get void*"); break;
            CASE(char);
            CASE(short);
            CASE(int);
            CASE(long);
            CASE(long long);
            CASE(unsigned char);
            CASE(unsigned short);
            CASE(unsigned int);
            CASE(unsigned long);
            CASE(unsigned long long);
            CASE(float);
            CASE(double);
            CASE(bool);
#undef CASE
        }
        return VAR_T(Pointer, *this);
    }

    void set(VM* vm, const PyVar& val){
        if(level > 1) {
            Pointer& p = CAST(Pointer&, val);
            ref<char*>() = p.ptr;   // We don't check the type, just copy the underlying address
            return;
        }
        switch(ctype->index){
#define CASE(T1, T2) case type_index<T1>(): ref<T1>() = CAST(T2, val); break
            case type_index<void>(): vm->ValueError("cannot set void*"); break;
            CASE(char, i64);
            CASE(short, i64);
            CASE(int, i64);
            CASE(long, i64);
            CASE(long long, i64);
            CASE(unsigned char, i64);
            CASE(unsigned short, i64);
            CASE(unsigned int, i64);
            CASE(unsigned long, i64);
            CASE(unsigned long long, i64);
            CASE(float, f64);
            CASE(double, f64);
            CASE(bool, bool);
#undef CASE
            default: UNREACHABLE();
        }
    }

    Pointer _to(VM* vm, StrName name){
        auto it = ctype->members.find(name);
        if(it == ctype->members.end()){
            vm->AttributeError(Str("struct '") + ctype->name + "' has no member " + name.str().escape(true));
        }
        const MemberInfo& info = it->second;
        return {info.type, level, ptr+info.offset};
    }
};


struct Value {
    PY_CLASS(Value, c, _value)

    char* data;
    Pointer head;

    const TypeInfo* ctype() const { return head.ctype; }

    Value(const TypeInfo* type) {
        data = new char[type->size];
        memset(data, 0, type->size);
        this->head = Pointer(type, data);
    }

    Value(const TypeInfo* type, void* src) {
        data = new char[type->size];
        memcpy(data, src, type->size);
        this->head = Pointer(type, data);
    }

    Value(Value&& other) noexcept {
        data = other.data;
        head = other.head;
        other.data = nullptr;
    }

    Value& operator=(Value&& other) noexcept = delete;
    Value& operator=(const Value& other) = delete;
    Value(const Value& other) = delete;
    
    static void _register(VM* vm, PyVar mod, PyVar type){
        vm->bind_static_method<-1>(type, "__new__", CPP_NOT_IMPLEMENTED());

        vm->bind_method<0>(type, "ptr", [](VM* vm, Args& args) {
            Value& self = CAST(Value&, args[0]);
            return VAR_T(Pointer, self.head);
        });

        vm->bind_method<1>(type, "__getattr__", [](VM* vm, Args& args) {
            Value& self = CAST(Value&, args[0]);
            const Str& name = CAST(Str&, args[1]);
            return self.head._to(vm, name).get(vm);
        });
    }

    ~Value(){
        delete[] data;
    }
};


struct CType{
    PY_CLASS(CType, c, ctype)

    const TypeInfo* type;

    CType() : type(_type_db.get<void>()) {}
    CType(const TypeInfo* type) : type(type) {}

    static void _register(VM* vm, PyVar mod, PyVar type){
        vm->bind_static_method<1>(type, "__new__", [](VM* vm, Args& args) {
            const Str& name = CAST(Str&, args[0]);
            const TypeInfo* type = _type_db.get(name);
            if(type == nullptr) vm->TypeError("unknown type: " + name.escape(true));
            return VAR_T(CType, type);
        });

        vm->bind_method<0>(type, "__call__", [](VM* vm, Args& args) {
            CType& self = CAST(CType&, args[0]);
            return VAR_T(Value, self.type);
        });
    }
};

void add_module_c(VM* vm){
    PyVar mod = vm->new_module("c");
    PyVar ptr_t = Pointer::register_class(vm, mod);
    Value::register_class(vm, mod);
    CType::register_class(vm, mod);

    vm->setattr(mod, "nullptr", VAR_T(Pointer));

    vm->bind_func<1>(mod, "malloc", [](VM* vm, Args& args) {
        i64 size = CAST(i64, args[0]);
        return VAR_T(Pointer, _type_db.get<void>(), (char*)malloc(size));
    });

    vm->bind_func<1>(mod, "free", [](VM* vm, Args& args) {
        Pointer& self = CAST(Pointer&, args[0]);
        free(self.ptr);
        return vm->None;
    });

    vm->bind_func<3>(mod, "memcpy", [](VM* vm, Args& args) {
        Pointer& dst = CAST(Pointer&, args[0]);
        Pointer& src = CAST(Pointer&, args[1]);
        i64 size = CAST(i64, args[2]);
        memcpy(dst.ptr, src.ptr, size);
        return vm->None;
    });

    vm->bind_func<2>(mod, "cast", [](VM* vm, Args& args) {
        Pointer& self = CAST(Pointer&, args[0]);
        const Str& name = CAST(Str&, args[1]);
        int level = 0;
        for(int i=name.size()-1; i>=0; i--){
            if(name[i] == '*') level++;
            else break;
        }
        if(level == 0) vm->TypeError("expect a pointer type, such as 'int*'");
        Str type_s = name.substr(0, name.size()-level);
        const TypeInfo* type = _type_db.get(type_s);
        if(type == nullptr) vm->TypeError("unknown type: " + type_s.escape(true));
        return VAR_T(Pointer, type, level, self.ptr);
    });

    vm->bind_func<1>(mod, "sizeof", [](VM* vm, Args& args) {
        const Str& name = CAST(Str&, args[0]);
        if(name.find('*') != Str::npos) return VAR(sizeof(void*));
        const TypeInfo* type = _type_db.get(name);
        if(type == nullptr) vm->TypeError("unknown type: " + name.escape(true));
        return VAR(type->size);
    });

    vm->bind_func<3>(mod, "memset", [](VM* vm, Args& args) {
        Pointer& dst = CAST(Pointer&, args[0]);
        i64 val = CAST(i64, args[1]);
        i64 size = CAST(i64, args[2]);
        memset(dst.ptr, (int)val, size);
        return vm->None;
    });
}

PyVar py_var(VM* vm, void* p){
    return VAR_T(Pointer, _type_db.get<void>(), (char*)p);
}

PyVar py_var(VM* vm, char* p){
    return VAR_T(Pointer, _type_db.get<char>(), (char*)p);
}

/***********************************************/

template<typename T>
struct _pointer {
    static constexpr int level = 0;
    using baseT = T;
};

template<typename T>
struct _pointer<T*> {
    static constexpr int level = _pointer<T>::level + 1;
    using baseT = typename _pointer<T>::baseT;
};

template<typename T>
struct pointer {
    static constexpr int level = _pointer<std::decay_t<T>>::level;
    using baseT = typename _pointer<std::decay_t<T>>::baseT;
};

template<typename T>
T py_pointer_cast(VM* vm, const PyVar& var){
    static_assert(std::is_pointer_v<T>);
    Pointer& p = CAST(Pointer&, var);
    const TypeInfo* type = _type_db.get<typename pointer<T>::baseT>();
    const int level = pointer<T>::level;
    if(p.ctype != type || p.level != level){
        vm->TypeError("invalid pointer cast");
    }
    return reinterpret_cast<T>(p.ptr);
}

template<typename T>
T py_value_cast(VM* vm, const PyVar& var){
    static_assert(std::is_pod_v<T>);
    Value& v = CAST(Value&, var);
    return *reinterpret_cast<T*>(v.data);
}

template<typename T>
std::enable_if_t<std::is_pointer_v<std::decay_t<T>>, PyVar>
py_var(VM* vm, T p){
    const TypeInfo* type = _type_db.get<typename pointer<T>::baseT>();
    if(type == nullptr) type = _type_db.get<void>();
    return VAR_T(Pointer, type, pointer<T>::level, (char*)p);
}

template<typename T>
std::enable_if_t<!std::is_pointer_v<std::decay_t<T>>, PyVar>
py_var(VM* vm, T p){
    const TypeInfo* type = _type_db.get<T>();
    return VAR_T(Value, type, &p);
}

}   // namespace pkpy


#if PK_ENABLE_FILEIO

#include <fstream>
#include <filesystem>

namespace pkpy{

struct FileIO {
    PY_CLASS(FileIO, io, FileIO)

    Str file;
    Str mode;
    std::fstream _fs;

    FileIO(VM* vm, Str file, Str mode): file(file), mode(mode) {
        if(mode == "rt" || mode == "r"){
            _fs.open(file, std::ios::in);
        }else if(mode == "wt" || mode == "w"){
            _fs.open(file, std::ios::out);
        }else if(mode == "at" || mode == "a"){
            _fs.open(file, std::ios::app);
        }
        if(!_fs.is_open()) vm->IOError(strerror(errno));
    }

    static void _register(VM* vm, PyVar mod, PyVar type){
        vm->bind_static_method<2>(type, "__new__", [](VM* vm, Args& args){
            return VAR_T(FileIO, 
                vm, CAST(Str, args[0]), CAST(Str, args[1])
            );
        });

        vm->bind_method<0>(type, "read", [](VM* vm, Args& args){
            FileIO& io = CAST(FileIO&, args[0]);
            std::string buffer;
            io._fs >> buffer;
            return VAR(buffer);
        });

        vm->bind_method<1>(type, "write", [](VM* vm, Args& args){
            FileIO& io = CAST(FileIO&, args[0]);
            io._fs << CAST(Str&, args[1]);
            return vm->None;
        });

        vm->bind_method<0>(type, "close", [](VM* vm, Args& args){
            FileIO& io = CAST(FileIO&, args[0]);
            io._fs.close();
            return vm->None;
        });

        vm->bind_method<0>(type, "__exit__", [](VM* vm, Args& args){
            FileIO& io = CAST(FileIO&, args[0]);
            io._fs.close();
            return vm->None;
        });

        vm->bind_method<0>(type, "__enter__", CPP_LAMBDA(vm->None));
    }
};

void add_module_io(VM* vm){
    PyVar mod = vm->new_module("io");
    PyVar type = FileIO::register_class(vm, mod);
    vm->bind_builtin_func<2>("open", [type](VM* vm, const Args& args){
        return vm->call(type, args);
    });
}

void add_module_os(VM* vm){
    PyVar mod = vm->new_module("os");
    vm->bind_func<0>(mod, "getcwd", [](VM* vm, const Args& args){
        return VAR(std::filesystem::current_path().string());
    });

    vm->bind_func<1>(mod, "listdir", [](VM* vm, const Args& args){
        std::filesystem::path path(CAST(Str&, args[0]).c_str());
        std::filesystem::directory_iterator di;
        try{
            di = std::filesystem::directory_iterator(path);
        }catch(std::filesystem::filesystem_error& e){
            Str msg = e.what();
            auto pos = msg.find_last_of(":");
            if(pos != Str::npos) msg = msg.substr(pos + 1);
            vm->IOError(msg.lstrip());
        }
        List ret;
        for(auto& p: di) ret.push_back(VAR(p.path().filename().string()));
        return VAR(ret);
    });

    vm->bind_func<1>(mod, "remove", [](VM* vm, const Args& args){
        std::filesystem::path path(CAST(Str&, args[0]).c_str());
        bool ok = std::filesystem::remove(path);
        if(!ok) vm->IOError("operation failed");
        return vm->None;
    });

    vm->bind_func<1>(mod, "mkdir", [](VM* vm, const Args& args){
        std::filesystem::path path(CAST(Str&, args[0]).c_str());
        bool ok = std::filesystem::create_directory(path);
        if(!ok) vm->IOError("operation failed");
        return vm->None;
    });

    vm->bind_func<1>(mod, "rmdir", [](VM* vm, const Args& args){
        std::filesystem::path path(CAST(Str&, args[0]).c_str());
        bool ok = std::filesystem::remove(path);
        if(!ok) vm->IOError("operation failed");
        return vm->None;
    });

    vm->bind_func<-1>(mod, "path_join", [](VM* vm, const Args& args){
        std::filesystem::path path;
        for(int i=0; i<args.size(); i++){
            path /= CAST(Str&, args[i]).c_str();
        }
        return VAR(path.string());
    });
}

} // namespace pkpy


#else

namespace pkpy{
void add_module_io(VM* vm){}
void add_module_os(VM* vm){}
} // namespace pkpy

#endif

// generated on 2023-03-16 21:41:33
#include <map>
#include <string>

namespace pkpy{
    std::map<std::string, const char*> kPythonLibs = {
        {"builtins", "\x64\x65\x66\x20\x70\x72\x69\x6e\x74\x28\x2a\x61\x72\x67\x73\x2c\x20\x73\x65\x70\x3d\x27\x20\x27\x2c\x20\x65\x6e\x64\x3d\x27\x5c\x6e\x27\x29\x3a\x0a\x20\x20\x20\x20\x73\x20\x3d\x20\x73\x65\x70\x2e\x6a\x6f\x69\x6e\x28\x5b\x73\x74\x72\x28\x69\x29\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x61\x72\x67\x73\x5d\x29\x0a\x20\x20\x20\x20\x5f\x5f\x73\x79\x73\x5f\x73\x74\x64\x6f\x75\x74\x5f\x77\x72\x69\x74\x65\x28\x73\x20\x2b\x20\x65\x6e\x64\x29\x0a\x0a\x64\x65\x66\x20\x72\x6f\x75\x6e\x64\x28\x78\x2c\x20\x6e\x64\x69\x67\x69\x74\x73\x3d\x30\x29\x3a\x0a\x20\x20\x20\x20\x61\x73\x73\x65\x72\x74\x20\x6e\x64\x69\x67\x69\x74\x73\x20\x3e\x3d\x20\x30\x0a\x20\x20\x20\x20\x69\x66\x20\x6e\x64\x69\x67\x69\x74\x73\x20\x3d\x3d\x20\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x78\x20\x3e\x3d\x20\x30\x20\x3f\x20\x69\x6e\x74\x28\x78\x20\x2b\x20\x30\x2e\x35\x29\x20\x3a\x20\x69\x6e\x74\x28\x78\x20\x2d\x20\x30\x2e\x35\x29\x0a\x20\x20\x20\x20\x69\x66\x20\x78\x20\x3e\x3d\x20\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x69\x6e\x74\x28\x78\x20\x2a\x20\x31\x30\x2a\x2a\x6e\x64\x69\x67\x69\x74\x73\x20\x2b\x20\x30\x2e\x35\x29\x20\x2f\x20\x31\x30\x2a\x2a\x6e\x64\x69\x67\x69\x74\x73\x0a\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x69\x6e\x74\x28\x78\x20\x2a\x20\x31\x30\x2a\x2a\x6e\x64\x69\x67\x69\x74\x73\x20\x2d\x20\x30\x2e\x35\x29\x20\x2f\x20\x31\x30\x2a\x2a\x6e\x64\x69\x67\x69\x74\x73\x0a\x0a\x64\x65\x66\x20\x69\x73\x69\x6e\x73\x74\x61\x6e\x63\x65\x28\x6f\x62\x6a\x2c\x20\x63\x6c\x73\x29\x3a\x0a\x20\x20\x20\x20\x61\x73\x73\x65\x72\x74\x20\x74\x79\x70\x65\x28\x63\x6c\x73\x29\x20\x69\x73\x20\x74\x79\x70\x65\x0a\x20\x20\x20\x20\x6f\x62\x6a\x5f\x74\x20\x3d\x20\x74\x79\x70\x65\x28\x6f\x62\x6a\x29\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x6f\x62\x6a\x5f\x74\x20\x69\x73\x20\x6e\x6f\x74\x20\x4e\x6f\x6e\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6f\x62\x6a\x5f\x74\x20\x69\x73\x20\x63\x6c\x73\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x54\x72\x75\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6f\x62\x6a\x5f\x74\x20\x3d\x20\x6f\x62\x6a\x5f\x74\x2e\x5f\x5f\x62\x61\x73\x65\x5f\x5f\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x46\x61\x6c\x73\x65\x0a\x0a\x64\x65\x66\x20\x61\x62\x73\x28\x78\x29\x3a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x78\x20\x3c\x20\x30\x20\x3f\x20\x2d\x78\x20\x3a\x20\x78\x0a\x0a\x64\x65\x66\x20\x6d\x61\x78\x28\x61\x2c\x20\x62\x29\x3a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x61\x20\x3e\x20\x62\x20\x3f\x20\x61\x20\x3a\x20\x62\x0a\x0a\x64\x65\x66\x20\x6d\x69\x6e\x28\x61\x2c\x20\x62\x29\x3a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x61\x20\x3c\x20\x62\x20\x3f\x20\x61\x20\x3a\x20\x62\x0a\x0a\x64\x65\x66\x20\x61\x6c\x6c\x28\x69\x74\x65\x72\x61\x62\x6c\x65\x29\x3a\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6e\x6f\x74\x20\x69\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x46\x61\x6c\x73\x65\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x54\x72\x75\x65\x0a\x0a\x64\x65\x66\x20\x61\x6e\x79\x28\x69\x74\x65\x72\x61\x62\x6c\x65\x29\x3a\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x69\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x54\x72\x75\x65\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x46\x61\x6c\x73\x65\x0a\x0a\x64\x65\x66\x20\x65\x6e\x75\x6d\x65\x72\x61\x74\x65\x28\x69\x74\x65\x72\x61\x62\x6c\x65\x2c\x20\x73\x74\x61\x72\x74\x3d\x30\x29\x3a\x0a\x20\x20\x20\x20\x6e\x20\x3d\x20\x73\x74\x61\x72\x74\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x65\x6c\x65\x6d\x20\x69\x6e\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x79\x69\x65\x6c\x64\x20\x6e\x2c\x20\x65\x6c\x65\x6d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6e\x20\x2b\x3d\x20\x31\x0a\x0a\x64\x65\x66\x20\x73\x75\x6d\x28\x69\x74\x65\x72\x61\x62\x6c\x65\x29\x3a\x0a\x20\x20\x20\x20\x72\x65\x73\x20\x3d\x20\x30\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x73\x20\x2b\x3d\x20\x69\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x73\x0a\x0a\x64\x65\x66\x20\x6d\x61\x70\x28\x66\x2c\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x29\x3a\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x79\x69\x65\x6c\x64\x20\x66\x28\x69\x29\x0a\x0a\x64\x65\x66\x20\x66\x69\x6c\x74\x65\x72\x28\x66\x2c\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x29\x3a\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x66\x28\x69\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x79\x69\x65\x6c\x64\x20\x69\x0a\x0a\x64\x65\x66\x20\x7a\x69\x70\x28\x61\x2c\x20\x62\x29\x3a\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x72\x61\x6e\x67\x65\x28\x6d\x69\x6e\x28\x6c\x65\x6e\x28\x61\x29\x2c\x20\x6c\x65\x6e\x28\x62\x29\x29\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x79\x69\x65\x6c\x64\x20\x28\x61\x5b\x69\x5d\x2c\x20\x62\x5b\x69\x5d\x29\x0a\x0a\x64\x65\x66\x20\x72\x65\x76\x65\x72\x73\x65\x64\x28\x69\x74\x65\x72\x61\x62\x6c\x65\x29\x3a\x0a\x20\x20\x20\x20\x61\x20\x3d\x20\x6c\x69\x73\x74\x28\x69\x74\x65\x72\x61\x62\x6c\x65\x29\x0a\x20\x20\x20\x20\x61\x2e\x72\x65\x76\x65\x72\x73\x65\x28\x29\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x61\x0a\x0a\x64\x65\x66\x20\x73\x6f\x72\x74\x65\x64\x28\x69\x74\x65\x72\x61\x62\x6c\x65\x2c\x20\x72\x65\x76\x65\x72\x73\x65\x3d\x46\x61\x6c\x73\x65\x29\x3a\x0a\x20\x20\x20\x20\x61\x20\x3d\x20\x6c\x69\x73\x74\x28\x69\x74\x65\x72\x61\x62\x6c\x65\x29\x0a\x20\x20\x20\x20\x61\x2e\x73\x6f\x72\x74\x28\x72\x65\x76\x65\x72\x73\x65\x3d\x72\x65\x76\x65\x72\x73\x65\x29\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x61\x0a\x0a\x23\x23\x23\x23\x23\x20\x73\x74\x72\x20\x23\x23\x23\x23\x23\x0a\x0a\x73\x74\x72\x2e\x5f\x5f\x6d\x75\x6c\x5f\x5f\x20\x3d\x20\x6c\x61\x6d\x62\x64\x61\x20\x73\x65\x6c\x66\x2c\x20\x6e\x3a\x20\x27\x27\x2e\x6a\x6f\x69\x6e\x28\x5b\x73\x65\x6c\x66\x20\x66\x6f\x72\x20\x5f\x20\x69\x6e\x20\x72\x61\x6e\x67\x65\x28\x6e\x29\x5d\x29\x0a\x0a\x64\x65\x66\x20\x73\x74\x72\x3a\x3a\x73\x70\x6c\x69\x74\x28\x73\x65\x6c\x66\x2c\x20\x73\x65\x70\x29\x3a\x0a\x20\x20\x20\x20\x69\x66\x20\x73\x65\x70\x20\x3d\x3d\x20\x22\x22\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x69\x73\x74\x28\x73\x65\x6c\x66\x29\x0a\x20\x20\x20\x20\x72\x65\x73\x20\x3d\x20\x5b\x5d\x0a\x20\x20\x20\x20\x69\x20\x3d\x20\x30\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x69\x20\x3c\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x73\x65\x6c\x66\x5b\x69\x3a\x69\x2b\x6c\x65\x6e\x28\x73\x65\x70\x29\x5d\x20\x3d\x3d\x20\x73\x65\x70\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x73\x2e\x61\x70\x70\x65\x6e\x64\x28\x73\x65\x6c\x66\x5b\x3a\x69\x5d\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x20\x3d\x20\x73\x65\x6c\x66\x5b\x69\x2b\x6c\x65\x6e\x28\x73\x65\x70\x29\x3a\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x20\x3d\x20\x30\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x20\x2b\x3d\x20\x31\x0a\x20\x20\x20\x20\x72\x65\x73\x2e\x61\x70\x70\x65\x6e\x64\x28\x73\x65\x6c\x66\x29\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x73\x0a\x0a\x64\x65\x66\x20\x73\x74\x72\x3a\x3a\x69\x6e\x64\x65\x78\x28\x73\x65\x6c\x66\x2c\x20\x73\x75\x62\x29\x3a\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x72\x61\x6e\x67\x65\x28\x6c\x65\x6e\x28\x73\x65\x6c\x66\x29\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x73\x65\x6c\x66\x5b\x69\x3a\x69\x2b\x6c\x65\x6e\x28\x73\x75\x62\x29\x5d\x20\x3d\x3d\x20\x73\x75\x62\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x69\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x2d\x31\x0a\x0a\x64\x65\x66\x20\x73\x74\x72\x3a\x3a\x73\x74\x72\x69\x70\x28\x73\x65\x6c\x66\x2c\x20\x63\x68\x61\x72\x73\x3d\x4e\x6f\x6e\x65\x29\x3a\x0a\x20\x20\x20\x20\x63\x68\x61\x72\x73\x20\x3d\x20\x63\x68\x61\x72\x73\x20\x6f\x72\x20\x27\x20\x5c\x74\x5c\x6e\x5c\x72\x27\x0a\x20\x20\x20\x20\x69\x20\x3d\x20\x30\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x69\x20\x3c\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x29\x20\x61\x6e\x64\x20\x73\x65\x6c\x66\x5b\x69\x5d\x20\x69\x6e\x20\x63\x68\x61\x72\x73\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x20\x2b\x3d\x20\x31\x0a\x20\x20\x20\x20\x6a\x20\x3d\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x29\x20\x2d\x20\x31\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x6a\x20\x3e\x3d\x20\x30\x20\x61\x6e\x64\x20\x73\x65\x6c\x66\x5b\x6a\x5d\x20\x69\x6e\x20\x63\x68\x61\x72\x73\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6a\x20\x2d\x3d\x20\x31\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x5b\x69\x3a\x6a\x2b\x31\x5d\x0a\x0a\x23\x23\x23\x23\x23\x20\x6c\x69\x73\x74\x20\x23\x23\x23\x23\x23\x0a\x0a\x6c\x69\x73\x74\x2e\x5f\x5f\x6e\x65\x77\x5f\x5f\x20\x3d\x20\x6c\x61\x6d\x62\x64\x61\x20\x6f\x62\x6a\x3a\x20\x5b\x69\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x6f\x62\x6a\x5d\x0a\x6c\x69\x73\x74\x2e\x5f\x5f\x72\x65\x70\x72\x5f\x5f\x20\x3d\x20\x6c\x61\x6d\x62\x64\x61\x20\x73\x65\x6c\x66\x3a\x20\x27\x5b\x27\x20\x2b\x20\x27\x2c\x20\x27\x2e\x6a\x6f\x69\x6e\x28\x5b\x72\x65\x70\x72\x28\x69\x29\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x73\x65\x6c\x66\x5d\x29\x20\x2b\x20\x27\x5d\x27\x0a\x74\x75\x70\x6c\x65\x2e\x5f\x5f\x72\x65\x70\x72\x5f\x5f\x20\x3d\x20\x6c\x61\x6d\x62\x64\x61\x20\x73\x65\x6c\x66\x3a\x20\x27\x28\x27\x20\x2b\x20\x27\x2c\x20\x27\x2e\x6a\x6f\x69\x6e\x28\x5b\x72\x65\x70\x72\x28\x69\x29\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x73\x65\x6c\x66\x5d\x29\x20\x2b\x20\x27\x29\x27\x0a\x6c\x69\x73\x74\x2e\x5f\x5f\x6a\x73\x6f\x6e\x5f\x5f\x20\x3d\x20\x6c\x61\x6d\x62\x64\x61\x20\x73\x65\x6c\x66\x3a\x20\x27\x5b\x27\x20\x2b\x20\x27\x2c\x20\x27\x2e\x6a\x6f\x69\x6e\x28\x5b\x69\x2e\x5f\x5f\x6a\x73\x6f\x6e\x5f\x5f\x28\x29\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x73\x65\x6c\x66\x5d\x29\x20\x2b\x20\x27\x5d\x27\x0a\x74\x75\x70\x6c\x65\x2e\x5f\x5f\x6a\x73\x6f\x6e\x5f\x5f\x20\x3d\x20\x6c\x61\x6d\x62\x64\x61\x20\x73\x65\x6c\x66\x3a\x20\x27\x5b\x27\x20\x2b\x20\x27\x2c\x20\x27\x2e\x6a\x6f\x69\x6e\x28\x5b\x69\x2e\x5f\x5f\x6a\x73\x6f\x6e\x5f\x5f\x28\x29\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x73\x65\x6c\x66\x5d\x29\x20\x2b\x20\x27\x5d\x27\x0a\x0a\x64\x65\x66\x20\x5f\x5f\x71\x73\x6f\x72\x74\x28\x61\x3a\x20\x6c\x69\x73\x74\x2c\x20\x4c\x3a\x20\x69\x6e\x74\x2c\x20\x52\x3a\x20\x69\x6e\x74\x29\x3a\x0a\x20\x20\x20\x20\x69\x66\x20\x4c\x20\x3e\x3d\x20\x52\x3a\x20\x72\x65\x74\x75\x72\x6e\x3b\x0a\x20\x20\x20\x20\x6d\x69\x64\x20\x3d\x20\x61\x5b\x28\x52\x2b\x4c\x29\x2f\x2f\x32\x5d\x3b\x0a\x20\x20\x20\x20\x69\x2c\x20\x6a\x20\x3d\x20\x4c\x2c\x20\x52\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x69\x3c\x3d\x6a\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x61\x5b\x69\x5d\x3c\x6d\x69\x64\x3a\x20\x69\x2b\x3d\x31\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x61\x5b\x6a\x5d\x3e\x6d\x69\x64\x3a\x20\x6a\x2d\x3d\x31\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x69\x3c\x3d\x6a\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x61\x5b\x69\x5d\x2c\x20\x61\x5b\x6a\x5d\x20\x3d\x20\x61\x5b\x6a\x5d\x2c\x20\x61\x5b\x69\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x2b\x3d\x31\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x6a\x2d\x3d\x31\x0a\x20\x20\x20\x20\x5f\x5f\x71\x73\x6f\x72\x74\x28\x61\x2c\x20\x4c\x2c\x20\x6a\x29\x0a\x20\x20\x20\x20\x5f\x5f\x71\x73\x6f\x72\x74\x28\x61\x2c\x20\x69\x2c\x20\x52\x29\x0a\x0a\x64\x65\x66\x20\x6c\x69\x73\x74\x3a\x3a\x73\x6f\x72\x74\x28\x73\x65\x6c\x66\x2c\x20\x72\x65\x76\x65\x72\x73\x65\x3d\x46\x61\x6c\x73\x65\x29\x3a\x0a\x20\x20\x20\x20\x5f\x5f\x71\x73\x6f\x72\x74\x28\x73\x65\x6c\x66\x2c\x20\x30\x2c\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x29\x2d\x31\x29\x0a\x20\x20\x20\x20\x69\x66\x20\x72\x65\x76\x65\x72\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x72\x65\x76\x65\x72\x73\x65\x28\x29\x0a\x0a\x64\x65\x66\x20\x6c\x69\x73\x74\x3a\x3a\x65\x78\x74\x65\x6e\x64\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x6f\x74\x68\x65\x72\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x61\x70\x70\x65\x6e\x64\x28\x69\x29\x0a\x0a\x64\x65\x66\x20\x6c\x69\x73\x74\x3a\x3a\x72\x65\x6d\x6f\x76\x65\x28\x73\x65\x6c\x66\x2c\x20\x76\x61\x6c\x75\x65\x29\x3a\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x72\x61\x6e\x67\x65\x28\x6c\x65\x6e\x28\x73\x65\x6c\x66\x29\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x73\x65\x6c\x66\x5b\x69\x5d\x20\x3d\x3d\x20\x76\x61\x6c\x75\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x64\x65\x6c\x20\x73\x65\x6c\x66\x5b\x69\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x54\x72\x75\x65\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x46\x61\x6c\x73\x65\x0a\x0a\x64\x65\x66\x20\x6c\x69\x73\x74\x3a\x3a\x69\x6e\x64\x65\x78\x28\x73\x65\x6c\x66\x2c\x20\x76\x61\x6c\x75\x65\x29\x3a\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x72\x61\x6e\x67\x65\x28\x6c\x65\x6e\x28\x73\x65\x6c\x66\x29\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x73\x65\x6c\x66\x5b\x69\x5d\x20\x3d\x3d\x20\x76\x61\x6c\x75\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x69\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x2d\x31\x0a\x0a\x64\x65\x66\x20\x6c\x69\x73\x74\x3a\x3a\x70\x6f\x70\x28\x73\x65\x6c\x66\x2c\x20\x69\x3d\x2d\x31\x29\x3a\x0a\x20\x20\x20\x20\x72\x65\x73\x20\x3d\x20\x73\x65\x6c\x66\x5b\x69\x5d\x0a\x20\x20\x20\x20\x64\x65\x6c\x20\x73\x65\x6c\x66\x5b\x69\x5d\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x73\x0a\x0a\x64\x65\x66\x20\x6c\x69\x73\x74\x3a\x3a\x5f\x5f\x65\x71\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x69\x66\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x29\x20\x21\x3d\x20\x6c\x65\x6e\x28\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x46\x61\x6c\x73\x65\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x72\x61\x6e\x67\x65\x28\x6c\x65\x6e\x28\x73\x65\x6c\x66\x29\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x73\x65\x6c\x66\x5b\x69\x5d\x20\x21\x3d\x20\x6f\x74\x68\x65\x72\x5b\x69\x5d\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x46\x61\x6c\x73\x65\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x54\x72\x75\x65\x0a\x74\x75\x70\x6c\x65\x2e\x5f\x5f\x65\x71\x5f\x5f\x20\x3d\x20\x6c\x69\x73\x74\x2e\x5f\x5f\x65\x71\x5f\x5f\x0a\x6c\x69\x73\x74\x2e\x5f\x5f\x6e\x65\x5f\x5f\x20\x3d\x20\x6c\x61\x6d\x62\x64\x61\x20\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x3a\x20\x6e\x6f\x74\x20\x73\x65\x6c\x66\x2e\x5f\x5f\x65\x71\x5f\x5f\x28\x6f\x74\x68\x65\x72\x29\x0a\x74\x75\x70\x6c\x65\x2e\x5f\x5f\x6e\x65\x5f\x5f\x20\x3d\x20\x6c\x61\x6d\x62\x64\x61\x20\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x3a\x20\x6e\x6f\x74\x20\x73\x65\x6c\x66\x2e\x5f\x5f\x65\x71\x5f\x5f\x28\x6f\x74\x68\x65\x72\x29\x0a\x0a\x64\x65\x66\x20\x6c\x69\x73\x74\x3a\x3a\x63\x6f\x75\x6e\x74\x28\x73\x65\x6c\x66\x2c\x20\x78\x29\x3a\x0a\x20\x20\x20\x20\x72\x65\x73\x20\x3d\x20\x30\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x73\x65\x6c\x66\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x69\x20\x3d\x3d\x20\x78\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x73\x20\x2b\x3d\x20\x31\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x73\x0a\x74\x75\x70\x6c\x65\x2e\x63\x6f\x75\x6e\x74\x20\x3d\x20\x6c\x69\x73\x74\x2e\x63\x6f\x75\x6e\x74\x0a\x0a\x64\x65\x66\x20\x6c\x69\x73\x74\x3a\x3a\x5f\x5f\x63\x6f\x6e\x74\x61\x69\x6e\x73\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x69\x74\x65\x6d\x29\x3a\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x73\x65\x6c\x66\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x69\x20\x3d\x3d\x20\x69\x74\x65\x6d\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x54\x72\x75\x65\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x46\x61\x6c\x73\x65\x0a\x74\x75\x70\x6c\x65\x2e\x5f\x5f\x63\x6f\x6e\x74\x61\x69\x6e\x73\x5f\x5f\x20\x3d\x20\x6c\x69\x73\x74\x2e\x5f\x5f\x63\x6f\x6e\x74\x61\x69\x6e\x73\x5f\x5f\x0a\x0a\x0a\x63\x6c\x61\x73\x73\x20\x70\x72\x6f\x70\x65\x72\x74\x79\x3a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x69\x6e\x69\x74\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x66\x67\x65\x74\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x66\x67\x65\x74\x20\x3d\x20\x66\x67\x65\x74\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x67\x65\x74\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x62\x6a\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x66\x67\x65\x74\x28\x6f\x62\x6a\x29"},
        {"dict", "\x63\x6c\x61\x73\x73\x20\x64\x69\x63\x74\x3a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x69\x6e\x69\x74\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x63\x61\x70\x61\x63\x69\x74\x79\x3d\x31\x33\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x5f\x63\x61\x70\x61\x63\x69\x74\x79\x20\x3d\x20\x63\x61\x70\x61\x63\x69\x74\x79\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x5f\x61\x20\x3d\x20\x5b\x4e\x6f\x6e\x65\x5d\x20\x2a\x20\x73\x65\x6c\x66\x2e\x5f\x63\x61\x70\x61\x63\x69\x74\x79\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x5f\x6c\x65\x6e\x20\x3d\x20\x30\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x6c\x65\x6e\x5f\x5f\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x6c\x65\x6e\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x70\x72\x6f\x62\x65\x28\x73\x65\x6c\x66\x2c\x20\x6b\x65\x79\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x20\x3d\x20\x68\x61\x73\x68\x28\x6b\x65\x79\x29\x20\x25\x20\x73\x65\x6c\x66\x2e\x5f\x63\x61\x70\x61\x63\x69\x74\x79\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x73\x65\x6c\x66\x2e\x5f\x61\x5b\x69\x5d\x20\x69\x73\x20\x6e\x6f\x74\x20\x4e\x6f\x6e\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x73\x65\x6c\x66\x2e\x5f\x61\x5b\x69\x5d\x5b\x30\x5d\x20\x3d\x3d\x20\x6b\x65\x79\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x54\x72\x75\x65\x2c\x20\x69\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x20\x3d\x20\x28\x69\x20\x2b\x20\x31\x29\x20\x25\x20\x73\x65\x6c\x66\x2e\x5f\x63\x61\x70\x61\x63\x69\x74\x79\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x46\x61\x6c\x73\x65\x2c\x20\x69\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x67\x65\x74\x69\x74\x65\x6d\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6b\x65\x79\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6f\x6b\x2c\x20\x69\x20\x3d\x20\x73\x65\x6c\x66\x2e\x5f\x5f\x70\x72\x6f\x62\x65\x28\x6b\x65\x79\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6e\x6f\x74\x20\x6f\x6b\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x61\x69\x73\x65\x20\x4b\x65\x79\x45\x72\x72\x6f\x72\x28\x72\x65\x70\x72\x28\x6b\x65\x79\x29\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x61\x5b\x69\x5d\x5b\x31\x5d\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x63\x6f\x6e\x74\x61\x69\x6e\x73\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6b\x65\x79\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6f\x6b\x2c\x20\x69\x20\x3d\x20\x73\x65\x6c\x66\x2e\x5f\x5f\x70\x72\x6f\x62\x65\x28\x6b\x65\x79\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6f\x6b\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x73\x65\x74\x69\x74\x65\x6d\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6b\x65\x79\x2c\x20\x76\x61\x6c\x75\x65\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6f\x6b\x2c\x20\x69\x20\x3d\x20\x73\x65\x6c\x66\x2e\x5f\x5f\x70\x72\x6f\x62\x65\x28\x6b\x65\x79\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6f\x6b\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x5f\x61\x5b\x69\x5d\x5b\x31\x5d\x20\x3d\x20\x76\x61\x6c\x75\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x5f\x61\x5b\x69\x5d\x20\x3d\x20\x5b\x6b\x65\x79\x2c\x20\x76\x61\x6c\x75\x65\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x5f\x6c\x65\x6e\x20\x2b\x3d\x20\x31\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x73\x65\x6c\x66\x2e\x5f\x6c\x65\x6e\x20\x3e\x20\x73\x65\x6c\x66\x2e\x5f\x63\x61\x70\x61\x63\x69\x74\x79\x20\x2a\x20\x30\x2e\x36\x37\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x5f\x63\x61\x70\x61\x63\x69\x74\x79\x20\x2a\x3d\x20\x32\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x5f\x5f\x72\x65\x68\x61\x73\x68\x28\x29\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x64\x65\x6c\x69\x74\x65\x6d\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6b\x65\x79\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6f\x6b\x2c\x20\x69\x20\x3d\x20\x73\x65\x6c\x66\x2e\x5f\x5f\x70\x72\x6f\x62\x65\x28\x6b\x65\x79\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6e\x6f\x74\x20\x6f\x6b\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x61\x69\x73\x65\x20\x4b\x65\x79\x45\x72\x72\x6f\x72\x28\x72\x65\x70\x72\x28\x6b\x65\x79\x29\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x5f\x61\x5b\x69\x5d\x20\x3d\x20\x4e\x6f\x6e\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x5f\x6c\x65\x6e\x20\x2d\x3d\x20\x31\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x72\x65\x68\x61\x73\x68\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6f\x6c\x64\x5f\x61\x20\x3d\x20\x73\x65\x6c\x66\x2e\x5f\x61\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x5f\x61\x20\x3d\x20\x5b\x4e\x6f\x6e\x65\x5d\x20\x2a\x20\x73\x65\x6c\x66\x2e\x5f\x63\x61\x70\x61\x63\x69\x74\x79\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x5f\x6c\x65\x6e\x20\x3d\x20\x30\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x6b\x76\x20\x69\x6e\x20\x6f\x6c\x64\x5f\x61\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6b\x76\x20\x69\x73\x20\x6e\x6f\x74\x20\x4e\x6f\x6e\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x5b\x6b\x76\x5b\x30\x5d\x5d\x20\x3d\x20\x6b\x76\x5b\x31\x5d\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x67\x65\x74\x28\x73\x65\x6c\x66\x2c\x20\x6b\x65\x79\x2c\x20\x64\x65\x66\x61\x75\x6c\x74\x3d\x4e\x6f\x6e\x65\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6f\x6b\x2c\x20\x69\x20\x3d\x20\x73\x65\x6c\x66\x2e\x5f\x5f\x70\x72\x6f\x62\x65\x28\x6b\x65\x79\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6f\x6b\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x61\x5b\x69\x5d\x5b\x31\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x64\x65\x66\x61\x75\x6c\x74\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x6b\x65\x79\x73\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x6b\x76\x20\x69\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x61\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6b\x76\x20\x69\x73\x20\x6e\x6f\x74\x20\x4e\x6f\x6e\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x79\x69\x65\x6c\x64\x20\x6b\x76\x5b\x30\x5d\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x76\x61\x6c\x75\x65\x73\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x6b\x76\x20\x69\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x61\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6b\x76\x20\x69\x73\x20\x6e\x6f\x74\x20\x4e\x6f\x6e\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x79\x69\x65\x6c\x64\x20\x6b\x76\x5b\x31\x5d\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x69\x74\x65\x6d\x73\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x6b\x76\x20\x69\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x61\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6b\x76\x20\x69\x73\x20\x6e\x6f\x74\x20\x4e\x6f\x6e\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x79\x69\x65\x6c\x64\x20\x6b\x76\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x63\x6c\x65\x61\x72\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x5f\x61\x20\x3d\x20\x5b\x4e\x6f\x6e\x65\x5d\x20\x2a\x20\x73\x65\x6c\x66\x2e\x5f\x63\x61\x70\x61\x63\x69\x74\x79\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x5f\x6c\x65\x6e\x20\x3d\x20\x30\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x75\x70\x64\x61\x74\x65\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x6b\x2c\x20\x76\x20\x69\x6e\x20\x6f\x74\x68\x65\x72\x2e\x69\x74\x65\x6d\x73\x28\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x5b\x6b\x5d\x20\x3d\x20\x76\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x63\x6f\x70\x79\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x64\x20\x3d\x20\x64\x69\x63\x74\x28\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x6b\x76\x20\x69\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x61\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6b\x76\x20\x69\x73\x20\x6e\x6f\x74\x20\x4e\x6f\x6e\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x64\x5b\x6b\x76\x5b\x30\x5d\x5d\x20\x3d\x20\x6b\x76\x5b\x31\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x64\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x72\x65\x70\x72\x5f\x5f\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x61\x20\x3d\x20\x5b\x72\x65\x70\x72\x28\x6b\x29\x2b\x27\x3a\x20\x27\x2b\x72\x65\x70\x72\x28\x76\x29\x20\x66\x6f\x72\x20\x6b\x2c\x76\x20\x69\x6e\x20\x73\x65\x6c\x66\x2e\x69\x74\x65\x6d\x73\x28\x29\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x27\x7b\x27\x2b\x20\x27\x2c\x20\x27\x2e\x6a\x6f\x69\x6e\x28\x61\x29\x20\x2b\x20\x27\x7d\x27\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x6a\x73\x6f\x6e\x5f\x5f\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x61\x20\x3d\x20\x5b\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x6b\x2c\x76\x20\x69\x6e\x20\x73\x65\x6c\x66\x2e\x69\x74\x65\x6d\x73\x28\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6b\x29\x20\x69\x73\x20\x6e\x6f\x74\x20\x73\x74\x72\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x61\x69\x73\x65\x20\x54\x79\x70\x65\x45\x72\x72\x6f\x72\x28\x27\x6a\x73\x6f\x6e\x20\x6b\x65\x79\x73\x20\x6d\x75\x73\x74\x20\x62\x65\x20\x73\x74\x72\x69\x6e\x67\x73\x2c\x20\x67\x6f\x74\x20\x27\x20\x2b\x20\x72\x65\x70\x72\x28\x6b\x29\x20\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x61\x2e\x61\x70\x70\x65\x6e\x64\x28\x6b\x2e\x5f\x5f\x6a\x73\x6f\x6e\x5f\x5f\x28\x29\x2b\x27\x3a\x20\x27\x2b\x76\x2e\x5f\x5f\x6a\x73\x6f\x6e\x5f\x5f\x28\x29\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x27\x7b\x27\x2b\x20\x27\x2c\x20\x27\x2e\x6a\x6f\x69\x6e\x28\x61\x29\x20\x2b\x20\x27\x7d\x27"},
        {"functools", "\x64\x65\x66\x20\x63\x61\x63\x68\x65\x28\x66\x29\x3a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x77\x72\x61\x70\x70\x65\x72\x28\x2a\x61\x72\x67\x73\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6e\x6f\x74\x20\x68\x61\x73\x61\x74\x74\x72\x28\x66\x2c\x20\x27\x63\x61\x63\x68\x65\x27\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x66\x2e\x63\x61\x63\x68\x65\x20\x3d\x20\x7b\x7d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6b\x65\x79\x20\x3d\x20\x61\x72\x67\x73\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6b\x65\x79\x20\x6e\x6f\x74\x20\x69\x6e\x20\x66\x2e\x63\x61\x63\x68\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x66\x2e\x63\x61\x63\x68\x65\x5b\x6b\x65\x79\x5d\x20\x3d\x20\x66\x28\x2a\x61\x72\x67\x73\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x66\x2e\x63\x61\x63\x68\x65\x5b\x6b\x65\x79\x5d\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x77\x72\x61\x70\x70\x65\x72"},
        {"random", "\x64\x65\x66\x20\x73\x68\x75\x66\x66\x6c\x65\x28\x4c\x29\x3a\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x72\x61\x6e\x67\x65\x28\x6c\x65\x6e\x28\x4c\x29\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6a\x20\x3d\x20\x72\x61\x6e\x64\x69\x6e\x74\x28\x69\x2c\x20\x6c\x65\x6e\x28\x4c\x29\x20\x2d\x20\x31\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x4c\x5b\x69\x5d\x2c\x20\x4c\x5b\x6a\x5d\x20\x3d\x20\x4c\x5b\x6a\x5d\x2c\x20\x4c\x5b\x69\x5d\x0a\x0a\x64\x65\x66\x20\x63\x68\x6f\x69\x63\x65\x28\x4c\x29\x3a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x4c\x5b\x72\x61\x6e\x64\x69\x6e\x74\x28\x30\x2c\x20\x6c\x65\x6e\x28\x4c\x29\x20\x2d\x20\x31\x29\x5d"},
        {"set", "\x63\x6c\x61\x73\x73\x20\x73\x65\x74\x3a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x69\x6e\x69\x74\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x3d\x4e\x6f\x6e\x65\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x20\x3d\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x20\x6f\x72\x20\x5b\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x5f\x61\x20\x3d\x20\x7b\x7d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x74\x65\x6d\x20\x69\x6e\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x61\x64\x64\x28\x69\x74\x65\x6d\x29\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x61\x64\x64\x28\x73\x65\x6c\x66\x2c\x20\x65\x6c\x65\x6d\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x5f\x61\x5b\x65\x6c\x65\x6d\x5d\x20\x3d\x20\x4e\x6f\x6e\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x64\x69\x73\x63\x61\x72\x64\x28\x73\x65\x6c\x66\x2c\x20\x65\x6c\x65\x6d\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x65\x6c\x65\x6d\x20\x69\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x61\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x64\x65\x6c\x20\x73\x65\x6c\x66\x2e\x5f\x61\x5b\x65\x6c\x65\x6d\x5d\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x72\x65\x6d\x6f\x76\x65\x28\x73\x65\x6c\x66\x2c\x20\x65\x6c\x65\x6d\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x64\x65\x6c\x20\x73\x65\x6c\x66\x2e\x5f\x61\x5b\x65\x6c\x65\x6d\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x63\x6c\x65\x61\x72\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x5f\x61\x2e\x63\x6c\x65\x61\x72\x28\x29\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x75\x70\x64\x61\x74\x65\x28\x73\x65\x6c\x66\x2c\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x65\x6c\x65\x6d\x20\x69\x6e\x20\x6f\x74\x68\x65\x72\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x61\x64\x64\x28\x65\x6c\x65\x6d\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x6c\x65\x6e\x5f\x5f\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x2e\x5f\x61\x29\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x63\x6f\x70\x79\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x74\x28\x73\x65\x6c\x66\x2e\x5f\x61\x2e\x6b\x65\x79\x73\x28\x29\x29\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x61\x6e\x64\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x20\x3d\x20\x73\x65\x74\x28\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x65\x6c\x65\x6d\x20\x69\x6e\x20\x73\x65\x6c\x66\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x65\x6c\x65\x6d\x20\x69\x6e\x20\x6f\x74\x68\x65\x72\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x2e\x61\x64\x64\x28\x65\x6c\x65\x6d\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x74\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x6f\x72\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x20\x3d\x20\x73\x65\x6c\x66\x2e\x63\x6f\x70\x79\x28\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x65\x6c\x65\x6d\x20\x69\x6e\x20\x6f\x74\x68\x65\x72\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x2e\x61\x64\x64\x28\x65\x6c\x65\x6d\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x74\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x73\x75\x62\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x20\x3d\x20\x73\x65\x74\x28\x29\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x65\x6c\x65\x6d\x20\x69\x6e\x20\x73\x65\x6c\x66\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x65\x6c\x65\x6d\x20\x6e\x6f\x74\x20\x69\x6e\x20\x6f\x74\x68\x65\x72\x3a\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x2e\x61\x64\x64\x28\x65\x6c\x65\x6d\x29\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x74\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x78\x6f\x72\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x20\x3d\x20\x73\x65\x74\x28\x29\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x65\x6c\x65\x6d\x20\x69\x6e\x20\x73\x65\x6c\x66\x3a\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x65\x6c\x65\x6d\x20\x6e\x6f\x74\x20\x69\x6e\x20\x6f\x74\x68\x65\x72\x3a\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x2e\x61\x64\x64\x28\x65\x6c\x65\x6d\x29\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x65\x6c\x65\x6d\x20\x69\x6e\x20\x6f\x74\x68\x65\x72\x3a\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x65\x6c\x65\x6d\x20\x6e\x6f\x74\x20\x69\x6e\x20\x73\x65\x6c\x66\x3a\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x2e\x61\x64\x64\x28\x65\x6c\x65\x6d\x29\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x74\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x75\x6e\x69\x6f\x6e\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x20\x7c\x20\x6f\x74\x68\x65\x72\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x69\x6e\x74\x65\x72\x73\x65\x63\x74\x69\x6f\x6e\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x20\x26\x20\x6f\x74\x68\x65\x72\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x64\x69\x66\x66\x65\x72\x65\x6e\x63\x65\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x20\x2d\x20\x6f\x74\x68\x65\x72\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x73\x79\x6d\x6d\x65\x74\x72\x69\x63\x5f\x64\x69\x66\x66\x65\x72\x65\x6e\x63\x65\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x20\x5e\x20\x6f\x74\x68\x65\x72\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x65\x71\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x5f\x78\x6f\x72\x5f\x5f\x28\x6f\x74\x68\x65\x72\x29\x2e\x5f\x5f\x6c\x65\x6e\x5f\x5f\x28\x29\x20\x3d\x3d\x20\x30\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x6e\x65\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x5f\x78\x6f\x72\x5f\x5f\x28\x6f\x74\x68\x65\x72\x29\x2e\x5f\x5f\x6c\x65\x6e\x5f\x5f\x28\x29\x20\x21\x3d\x20\x30\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x69\x73\x64\x69\x73\x6a\x6f\x69\x6e\x74\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x5f\x61\x6e\x64\x5f\x5f\x28\x6f\x74\x68\x65\x72\x29\x2e\x5f\x5f\x6c\x65\x6e\x5f\x5f\x28\x29\x20\x3d\x3d\x20\x30\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x69\x73\x73\x75\x62\x73\x65\x74\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x5f\x73\x75\x62\x5f\x5f\x28\x6f\x74\x68\x65\x72\x29\x2e\x5f\x5f\x6c\x65\x6e\x5f\x5f\x28\x29\x20\x3d\x3d\x20\x30\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x69\x73\x73\x75\x70\x65\x72\x73\x65\x74\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6f\x74\x68\x65\x72\x2e\x5f\x5f\x73\x75\x62\x5f\x5f\x28\x73\x65\x6c\x66\x29\x2e\x5f\x5f\x6c\x65\x6e\x5f\x5f\x28\x29\x20\x3d\x3d\x20\x30\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x63\x6f\x6e\x74\x61\x69\x6e\x73\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x65\x6c\x65\x6d\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x65\x6c\x65\x6d\x20\x69\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x61\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x72\x65\x70\x72\x5f\x5f\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x29\x20\x3d\x3d\x20\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x27\x73\x65\x74\x28\x29\x27\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x27\x7b\x27\x2b\x20\x27\x2c\x20\x27\x2e\x6a\x6f\x69\x6e\x28\x5b\x72\x65\x70\x72\x28\x69\x29\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x61\x2e\x6b\x65\x79\x73\x28\x29\x5d\x29\x20\x2b\x20\x27\x7d\x27\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x69\x74\x65\x72\x5f\x5f\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x61\x2e\x6b\x65\x79\x73\x28\x29"},

    };
}   // namespace pkpy



namespace pkpy {

CodeObject_ VM::compile(Str source, Str filename, CompileMode mode) {
    Compiler compiler(this, source.c_str(), filename, mode);
    try{
        return compiler.compile();
    }catch(Exception& e){
        // std::cout << e.summary() << std::endl;
        _error(e);
        return nullptr;
    }
}

#define BIND_NUM_ARITH_OPT(name, op)                                                                    \
    _vm->_bind_methods<1>({"int","float"}, #name, [](VM* vm, Args& args){                         \
        if(is_both_int(args[0], args[1])){                                                              \
            return VAR(_CAST(i64, args[0]) op _CAST(i64, args[1]));                     \
        }else{                                                                                          \
            return VAR(vm->num_to_float(args[0]) op vm->num_to_float(args[1]));                 \
        }                                                                                               \
    });

#define BIND_NUM_LOGICAL_OPT(name, op, is_eq)                                                           \
    _vm->_bind_methods<1>({"int","float"}, #name, [](VM* vm, Args& args){                         \
        if(!is_both_int_or_float(args[0], args[1])){                                                    \
            if constexpr(is_eq) return VAR(args[0] op args[1]);                                  \
            vm->TypeError("unsupported operand type(s) for " #op );                                     \
        }                                                                                               \
        if(is_both_int(args[0], args[1]))                                                               \
            return VAR(_CAST(i64, args[0]) op _CAST(i64, args[1]));                    \
        return VAR(vm->num_to_float(args[0]) op vm->num_to_float(args[1]));                      \
    });
    

void init_builtins(VM* _vm) {
    BIND_NUM_ARITH_OPT(__add__, +)
    BIND_NUM_ARITH_OPT(__sub__, -)
    BIND_NUM_ARITH_OPT(__mul__, *)

    BIND_NUM_LOGICAL_OPT(__lt__, <, false)
    BIND_NUM_LOGICAL_OPT(__le__, <=, false)
    BIND_NUM_LOGICAL_OPT(__gt__, >, false)
    BIND_NUM_LOGICAL_OPT(__ge__, >=, false)
    BIND_NUM_LOGICAL_OPT(__eq__, ==, true)
    BIND_NUM_LOGICAL_OPT(__ne__, !=, true)

#undef BIND_NUM_ARITH_OPT
#undef BIND_NUM_LOGICAL_OPT

    _vm->bind_builtin_func<1>("__sys_stdout_write", [](VM* vm, Args& args) {
        (*vm->_stdout) << CAST(Str&, args[0]);
        return vm->None;
    });

    _vm->bind_builtin_func<0>("super", [](VM* vm, Args& args) {
        const PyVar* self = vm->top_frame()->f_locals().try_get(m_self);
        if(self == nullptr) vm->TypeError("super() can only be called in a class");
        return vm->new_object(vm->tp_super, *self);
    });

    _vm->bind_builtin_func<1>("id", [](VM* vm, Args& args) {
        const PyVar& obj = args[0];
        if(obj.is_tagged()) return VAR((i64)0);
        return VAR(obj.bits);
    });

    _vm->bind_builtin_func<1>("eval", [](VM* vm, Args& args) {
        CodeObject_ code = vm->compile(CAST(Str&, args[0]), "<eval>", EVAL_MODE);
        return vm->_exec(code, vm->top_frame()->_module, vm->top_frame()->_locals);
    });

    _vm->bind_builtin_func<1>("exec", [](VM* vm, Args& args) {
        CodeObject_ code = vm->compile(CAST(Str&, args[0]), "<exec>", EXEC_MODE);
        vm->_exec(code, vm->top_frame()->_module, vm->top_frame()->_locals);
        return vm->None;
    });

    _vm->bind_builtin_func<-1>("exit", [](VM* vm, Args& args) {
        if(args.size() == 0) std::exit(0);
        else if(args.size() == 1) std::exit(CAST(int, args[0]));
        else vm->TypeError("exit() takes at most 1 argument");
        return vm->None;
    });

    _vm->bind_builtin_func<1>("repr", CPP_LAMBDA(vm->asRepr(args[0])));
    _vm->bind_builtin_func<1>("len", CPP_LAMBDA(vm->call(args[0], __len__, no_arg())));

    _vm->bind_builtin_func<1>("hash", [](VM* vm, Args& args){
        i64 value = vm->hash(args[0]);
        if(((value << 2) >> 2) != value) value >>= 2;
        return VAR(value);
    });

    _vm->bind_builtin_func<1>("chr", [](VM* vm, Args& args) {
        i64 i = CAST(i64, args[0]);
        if (i < 0 || i > 128) vm->ValueError("chr() arg not in range(128)");
        return VAR(std::string(1, (char)i));
    });

    _vm->bind_builtin_func<1>("ord", [](VM* vm, Args& args) {
        const Str& s = CAST(Str&, args[0]);
        if (s.size() != 1) vm->TypeError("ord() expected an ASCII character");
        return VAR((i64)(s.c_str()[0]));
    });

    _vm->bind_builtin_func<2>("hasattr", [](VM* vm, Args& args) {
        return VAR(vm->getattr(args[0], CAST(Str&, args[1]), false) != nullptr);
    });

    _vm->bind_builtin_func<3>("setattr", [](VM* vm, Args& args) {
        vm->setattr(args[0], CAST(Str&, args[1]), args[2]);
        return vm->None;
    });

    _vm->bind_builtin_func<2>("getattr", [](VM* vm, Args& args) {
        const Str& name = CAST(Str&, args[1]);
        return vm->getattr(args[0], name);
    });

    _vm->bind_builtin_func<1>("hex", [](VM* vm, Args& args) {
        std::stringstream ss;
        ss << std::hex << CAST(i64, args[0]);
        return VAR("0x" + ss.str());
    });

    _vm->bind_builtin_func<1>("dir", [](VM* vm, Args& args) {
        std::set<StrName> names;
        if(args[0]->is_attr_valid()){
            std::vector<StrName> keys = args[0]->attr().keys();
            names.insert(keys.begin(), keys.end());
        }
        const NameDict& t_attr = vm->_t(args[0])->attr();
        std::vector<StrName> keys = t_attr.keys();
        names.insert(keys.begin(), keys.end());
        List ret;
        for (StrName name : names) ret.push_back(VAR(name.str()));
        return VAR(std::move(ret));
    });

    _vm->bind_method<0>("object", "__repr__", [](VM* vm, Args& args) {
        PyVar self = args[0];
        std::uintptr_t addr = self.is_tagged() ? 0 : (uintptr_t)self.get();
        StrStream ss;
        ss << std::hex << addr;
        Str s = "<" + OBJ_NAME(vm->_t(self)) + " object at 0x" + ss.str() + ">";
        return VAR(s);
    });

    _vm->bind_method<1>("object", "__eq__", CPP_LAMBDA(VAR(args[0] == args[1])));
    _vm->bind_method<1>("object", "__ne__", CPP_LAMBDA(VAR(args[0] != args[1])));

    _vm->bind_static_method<1>("type", "__new__", CPP_LAMBDA(vm->_t(args[0])));
    _vm->bind_method<0>("type", "__repr__", CPP_LAMBDA(VAR("<class '" + OBJ_GET(Str, args[0]->attr(__name__)) + "'>")));

    _vm->bind_static_method<-1>("range", "__new__", [](VM* vm, Args& args) {
        Range r;
        switch (args.size()) {
            case 1: r.stop = CAST(i64, args[0]); break;
            case 2: r.start = CAST(i64, args[0]); r.stop = CAST(i64, args[1]); break;
            case 3: r.start = CAST(i64, args[0]); r.stop = CAST(i64, args[1]); r.step = CAST(i64, args[2]); break;
            default: vm->TypeError("expected 1-3 arguments, but got " + std::to_string(args.size()));
        }
        return VAR(r);
    });

    _vm->bind_method<0>("range", "__iter__", CPP_LAMBDA(
        vm->PyIter(RangeIter(vm, args[0]))
    ));

    _vm->bind_method<0>("NoneType", "__repr__", CPP_LAMBDA(VAR("None")));
    _vm->bind_method<0>("NoneType", "__json__", CPP_LAMBDA(VAR("null")));

    _vm->_bind_methods<1>({"int", "float"}, "__truediv__", [](VM* vm, Args& args) {
        f64 rhs = vm->num_to_float(args[1]);
        if (rhs == 0) vm->ZeroDivisionError();
        return VAR(vm->num_to_float(args[0]) / rhs);
    });

    _vm->_bind_methods<1>({"int", "float"}, "__pow__", [](VM* vm, Args& args) {
        if(is_both_int(args[0], args[1])){
            i64 lhs = _CAST(i64, args[0]);
            i64 rhs = _CAST(i64, args[1]);
            bool flag = false;
            if(rhs < 0) {flag = true; rhs = -rhs;}
            i64 ret = 1;
            while(rhs){
                if(rhs & 1) ret *= lhs;
                lhs *= lhs;
                rhs >>= 1;
            }
            if(flag) return VAR((f64)(1.0 / ret));
            return VAR(ret);
        }else{
            return VAR((f64)std::pow(vm->num_to_float(args[0]), vm->num_to_float(args[1])));
        }
    });

    /************ PyInt ************/
    _vm->bind_static_method<1>("int", "__new__", [](VM* vm, Args& args) {
        if (is_type(args[0], vm->tp_int)) return args[0];
        if (is_type(args[0], vm->tp_float)) return VAR((i64)CAST(f64, args[0]));
        if (is_type(args[0], vm->tp_bool)) return VAR(_CAST(bool, args[0]) ? 1 : 0);
        if (is_type(args[0], vm->tp_str)) {
            const Str& s = CAST(Str&, args[0]);
            try{
                size_t parsed = 0;
                i64 val = S_TO_INT(s, &parsed, 10);
                if(parsed != s.size()) throw std::invalid_argument("<?>");
                return VAR(val);
            }catch(std::invalid_argument&){
                vm->ValueError("invalid literal for int(): " + s.escape(true));
            }
        }
        vm->TypeError("int() argument must be a int, float, bool or str");
        return vm->None;
    });

    _vm->bind_method<1>("int", "__floordiv__", [](VM* vm, Args& args) {
        i64 rhs = CAST(i64, args[1]);
        if(rhs == 0) vm->ZeroDivisionError();
        return VAR(CAST(i64, args[0]) / rhs);
    });

    _vm->bind_method<1>("int", "__mod__", [](VM* vm, Args& args) {
        i64 rhs = CAST(i64, args[1]);
        if(rhs == 0) vm->ZeroDivisionError();
        return VAR(CAST(i64, args[0]) % rhs);
    });

    _vm->bind_method<0>("int", "__repr__", CPP_LAMBDA(VAR(std::to_string(CAST(i64, args[0])))));
    _vm->bind_method<0>("int", "__json__", CPP_LAMBDA(VAR(std::to_string(CAST(i64, args[0])))));

#define INT_BITWISE_OP(name,op) \
    _vm->bind_method<1>("int", #name, CPP_LAMBDA(VAR(CAST(i64, args[0]) op CAST(i64, args[1]))));

    INT_BITWISE_OP(__lshift__, <<)
    INT_BITWISE_OP(__rshift__, >>)
    INT_BITWISE_OP(__and__, &)
    INT_BITWISE_OP(__or__, |)
    INT_BITWISE_OP(__xor__, ^)

#undef INT_BITWISE_OP

    /************ PyFloat ************/
    _vm->bind_static_method<1>("float", "__new__", [](VM* vm, Args& args) {
        if (is_type(args[0], vm->tp_int)) return VAR((f64)CAST(i64, args[0]));
        if (is_type(args[0], vm->tp_float)) return args[0];
        if (is_type(args[0], vm->tp_bool)) return VAR(_CAST(bool, args[0]) ? 1.0 : 0.0);
        if (is_type(args[0], vm->tp_str)) {
            const Str& s = CAST(Str&, args[0]);
            if(s == "inf") return VAR(INFINITY);
            if(s == "-inf") return VAR(-INFINITY);
            try{
                f64 val = S_TO_FLOAT(s);
                return VAR(val);
            }catch(std::invalid_argument&){
                vm->ValueError("invalid literal for float(): '" + s + "'");
            }
        }
        vm->TypeError("float() argument must be a int, float, bool or str");
        return vm->None;
    });

    _vm->bind_method<0>("float", "__repr__", [](VM* vm, Args& args) {
        f64 val = CAST(f64, args[0]);
        if(std::isinf(val) || std::isnan(val)) return VAR(std::to_string(val));
        StrStream ss;
        ss << std::setprecision(std::numeric_limits<f64>::max_digits10-1-2) << val;
        std::string s = ss.str();
        if(std::all_of(s.begin()+1, s.end(), isdigit)) s += ".0";
        return VAR(s);
    });

    _vm->bind_method<0>("float", "__json__", [](VM* vm, Args& args) {
        f64 val = CAST(f64, args[0]);
        if(std::isinf(val) || std::isnan(val)) vm->ValueError("cannot jsonify 'nan' or 'inf'");
        return VAR(std::to_string(val));
    });

    /************ PyString ************/
    _vm->bind_static_method<1>("str", "__new__", CPP_LAMBDA(vm->asStr(args[0])));

    _vm->bind_method<1>("str", "__add__", [](VM* vm, Args& args) {
        const Str& lhs = CAST(Str&, args[0]);
        const Str& rhs = CAST(Str&, args[1]);
        return VAR(lhs + rhs);
    });

    _vm->bind_method<0>("str", "__len__", [](VM* vm, Args& args) {
        const Str& self = CAST(Str&, args[0]);
        return VAR(self.u8_length());
    });

    _vm->bind_method<1>("str", "__contains__", [](VM* vm, Args& args) {
        const Str& self = CAST(Str&, args[0]);
        const Str& other = CAST(Str&, args[1]);
        return VAR(self.find(other) != Str::npos);
    });

    _vm->bind_method<0>("str", "__str__", CPP_LAMBDA(args[0]));
    _vm->bind_method<0>("str", "__iter__", CPP_LAMBDA(vm->PyIter(StringIter(vm, args[0]))));

    _vm->bind_method<0>("str", "__repr__", [](VM* vm, Args& args) {
        const Str& _self = CAST(Str&, args[0]);
        return VAR(_self.escape(true));
    });

    _vm->bind_method<0>("str", "__json__", [](VM* vm, Args& args) {
        const Str& self = CAST(Str&, args[0]);
        return VAR(self.escape(false));
    });

    _vm->bind_method<1>("str", "__eq__", [](VM* vm, Args& args) {
        if(is_type(args[0], vm->tp_str) && is_type(args[1], vm->tp_str))
            return VAR(CAST(Str&, args[0]) == CAST(Str&, args[1]));
        return VAR(args[0] == args[1]);
    });

    _vm->bind_method<1>("str", "__ne__", [](VM* vm, Args& args) {
        if(is_type(args[0], vm->tp_str) && is_type(args[1], vm->tp_str))
            return VAR(CAST(Str&, args[0]) != CAST(Str&, args[1]));
        return VAR(args[0] != args[1]);
    });

    _vm->bind_method<1>("str", "__getitem__", [](VM* vm, Args& args) {
        const Str& self (CAST(Str&, args[0]));

        if(is_type(args[1], vm->tp_slice)){
            Slice s = _CAST(Slice, args[1]);
            s.normalize(self.u8_length());
            return VAR(self.u8_substr(s.start, s.stop));
        }

        int index = CAST(int, args[1]);
        index = vm->normalized_index(index, self.u8_length());
        return VAR(self.u8_getitem(index));
    });

    _vm->bind_method<1>("str", "__gt__", [](VM* vm, Args& args) {
        const Str& self (CAST(Str&, args[0]));
        const Str& obj (CAST(Str&, args[1]));
        return VAR(self > obj);
    });

    _vm->bind_method<1>("str", "__lt__", [](VM* vm, Args& args) {
        const Str& self (CAST(Str&, args[0]));
        const Str& obj (CAST(Str&, args[1]));
        return VAR(self < obj);
    });

    _vm->bind_method<2>("str", "replace", [](VM* vm, Args& args) {
        const Str& _self = CAST(Str&, args[0]);
        const Str& _old = CAST(Str&, args[1]);
        const Str& _new = CAST(Str&, args[2]);
        Str _copy = _self;
        size_t pos = 0;
        while ((pos = _copy.find(_old, pos)) != std::string::npos) {
            _copy.replace(pos, _old.length(), _new);
            pos += _new.length();
        }
        return VAR(_copy);
    });

    _vm->bind_method<1>("str", "startswith", [](VM* vm, Args& args) {
        const Str& self = CAST(Str&, args[0]);
        const Str& prefix = CAST(Str&, args[1]);
        return VAR(self.find(prefix) == 0);
    });

    _vm->bind_method<1>("str", "endswith", [](VM* vm, Args& args) {
        const Str& self = CAST(Str&, args[0]);
        const Str& suffix = CAST(Str&, args[1]);
        return VAR(self.rfind(suffix) == self.length() - suffix.length());
    });

    _vm->bind_method<1>("str", "join", [](VM* vm, Args& args) {
        const Str& self = CAST(Str&, args[0]);
        StrStream ss;
        PyVar obj = vm->asList(args[1]);
        const List& list = CAST(List&, obj);
        for (int i = 0; i < list.size(); ++i) {
            if (i > 0) ss << self;
            ss << CAST(Str&, list[i]);
        }
        return VAR(ss.str());
    });

    /************ PyList ************/
    _vm->bind_method<1>("list", "append", [](VM* vm, Args& args) {
        List& self = CAST(List&, args[0]);
        self.push_back(args[1]);
        return vm->None;
    });

    _vm->bind_method<0>("list", "reverse", [](VM* vm, Args& args) {
        List& self = CAST(List&, args[0]);
        std::reverse(self.begin(), self.end());
        return vm->None;
    });

    _vm->bind_method<1>("list", "__mul__", [](VM* vm, Args& args) {
        const List& self = CAST(List&, args[0]);
        int n = CAST(int, args[1]);
        List result;
        result.reserve(self.size() * n);
        for(int i = 0; i < n; i++) result.insert(result.end(), self.begin(), self.end());
        return VAR(std::move(result));
    });

    _vm->bind_method<2>("list", "insert", [](VM* vm, Args& args) {
        List& self = CAST(List&, args[0]);
        int index = CAST(int, args[1]);
        if(index < 0) index += self.size();
        if(index < 0) index = 0;
        if(index > self.size()) index = self.size();
        self.insert(self.begin() + index, args[2]);
        return vm->None;
    });

    _vm->bind_method<0>("list", "clear", [](VM* vm, Args& args) {
        CAST(List&, args[0]).clear();
        return vm->None;
    });

    _vm->bind_method<0>("list", "copy", CPP_LAMBDA(VAR(CAST(List, args[0]))));

    _vm->bind_method<1>("list", "__add__", [](VM* vm, Args& args) {
        const List& self = CAST(List&, args[0]);
        const List& obj = CAST(List&, args[1]);
        List new_list = self;
        new_list.insert(new_list.end(), obj.begin(), obj.end());
        return VAR(new_list);
    });

    _vm->bind_method<0>("list", "__len__", [](VM* vm, Args& args) {
        const List& self = CAST(List&, args[0]);
        return VAR(self.size());
    });

    _vm->bind_method<0>("list", "__iter__", [](VM* vm, Args& args) {
        return vm->PyIter(ArrayIter<List>(vm, args[0]));
    });

    _vm->bind_method<1>("list", "__getitem__", [](VM* vm, Args& args) {
        const List& self = CAST(List&, args[0]);

        if(is_type(args[1], vm->tp_slice)){
            Slice s = _CAST(Slice, args[1]);
            s.normalize(self.size());
            List new_list;
            for(size_t i = s.start; i < s.stop; i++) new_list.push_back(self[i]);
            return VAR(std::move(new_list));
        }

        int index = CAST(int, args[1]);
        index = vm->normalized_index(index, self.size());
        return self[index];
    });

    _vm->bind_method<2>("list", "__setitem__", [](VM* vm, Args& args) {
        List& self = CAST(List&, args[0]);
        int index = CAST(int, args[1]);
        index = vm->normalized_index(index, self.size());
        self[index] = args[2];
        return vm->None;
    });

    _vm->bind_method<1>("list", "__delitem__", [](VM* vm, Args& args) {
        List& self = CAST(List&, args[0]);
        int index = CAST(int, args[1]);
        index = vm->normalized_index(index, self.size());
        self.erase(self.begin() + index);
        return vm->None;
    });

    /************ PyTuple ************/
    _vm->bind_static_method<1>("tuple", "__new__", [](VM* vm, Args& args) {
        List list = CAST(List, vm->asList(args[0]));
        return VAR(std::move(list));
    });

    _vm->bind_method<0>("tuple", "__iter__", [](VM* vm, Args& args) {
        return vm->PyIter(ArrayIter<Args>(vm, args[0]));
    });

    _vm->bind_method<1>("tuple", "__getitem__", [](VM* vm, Args& args) {
        const Tuple& self = CAST(Tuple&, args[0]);

        if(is_type(args[1], vm->tp_slice)){
            Slice s = _CAST(Slice, args[1]);
            s.normalize(self.size());
            List new_list;
            for(size_t i = s.start; i < s.stop; i++) new_list.push_back(self[i]);
            return VAR(std::move(new_list));
        }

        int index = CAST(int, args[1]);
        index = vm->normalized_index(index, self.size());
        return self[index];
    });

    _vm->bind_method<0>("tuple", "__len__", [](VM* vm, Args& args) {
        const Tuple& self = CAST(Tuple&, args[0]);
        return VAR(self.size());
    });

    /************ PyBool ************/
    _vm->bind_static_method<1>("bool", "__new__", CPP_LAMBDA(vm->asBool(args[0])));

    _vm->bind_method<0>("bool", "__repr__", [](VM* vm, Args& args) {
        bool val = CAST(bool, args[0]);
        return VAR(val ? "True" : "False");
    });

    _vm->bind_method<0>("bool", "__json__", [](VM* vm, Args& args) {
        bool val = CAST(bool, args[0]);
        return VAR(val ? "true" : "false");
    });

    _vm->bind_method<1>("bool", "__xor__", [](VM* vm, Args& args) {
        bool self = CAST(bool, args[0]);
        bool other = CAST(bool, args[1]);
        return VAR(self ^ other);
    });

    _vm->bind_method<0>("ellipsis", "__repr__", CPP_LAMBDA(VAR("Ellipsis")));
}

#ifdef _WIN32
#define __EXPORT __declspec(dllexport)
#elif __APPLE__
#define __EXPORT __attribute__((visibility("default"))) __attribute__((used))
#elif __EMSCRIPTEN__
#include <emscripten.h>
#define __EXPORT EMSCRIPTEN_KEEPALIVE
#else
#define __EXPORT
#endif

void add_module_time(VM* vm){
    PyVar mod = vm->new_module("time");
    vm->bind_func<0>(mod, "time", [](VM* vm, Args& args) {
        auto now = std::chrono::high_resolution_clock::now();
        return VAR(std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count() / 1000000.0);
    });
}

void add_module_sys(VM* vm){
    PyVar mod = vm->new_module("sys");
    vm->setattr(mod, "version", VAR(PK_VERSION));

    vm->bind_func<1>(mod, "getrefcount", CPP_LAMBDA(VAR(args[0].use_count())));
    vm->bind_func<0>(mod, "getrecursionlimit", CPP_LAMBDA(VAR(vm->recursionlimit)));

    vm->bind_func<1>(mod, "setrecursionlimit", [](VM* vm, Args& args) {
        vm->recursionlimit = CAST(int, args[0]);
        return vm->None;
    });
}

void add_module_json(VM* vm){
    PyVar mod = vm->new_module("json");
    vm->bind_func<1>(mod, "loads", [](VM* vm, Args& args) {
        const Str& expr = CAST(Str&, args[0]);
        CodeObject_ code = vm->compile(expr, "<json>", JSON_MODE);
        return vm->_exec(code, vm->top_frame()->_module, vm->top_frame()->_locals);
    });

    vm->bind_func<1>(mod, "dumps", CPP_LAMBDA(vm->call(args[0], __json__)));
}

void add_module_math(VM* vm){
    PyVar mod = vm->new_module("math");
    vm->setattr(mod, "pi", VAR(3.1415926535897932384));
    vm->setattr(mod, "e" , VAR(2.7182818284590452354));

    vm->bind_func<1>(mod, "log", CPP_LAMBDA(VAR(std::log(vm->num_to_float(args[0])))));
    vm->bind_func<1>(mod, "log10", CPP_LAMBDA(VAR(std::log10(vm->num_to_float(args[0])))));
    vm->bind_func<1>(mod, "log2", CPP_LAMBDA(VAR(std::log2(vm->num_to_float(args[0])))));
    vm->bind_func<1>(mod, "sin", CPP_LAMBDA(VAR(std::sin(vm->num_to_float(args[0])))));
    vm->bind_func<1>(mod, "cos", CPP_LAMBDA(VAR(std::cos(vm->num_to_float(args[0])))));
    vm->bind_func<1>(mod, "tan", CPP_LAMBDA(VAR(std::tan(vm->num_to_float(args[0])))));
    vm->bind_func<1>(mod, "isnan", CPP_LAMBDA(VAR(std::isnan(vm->num_to_float(args[0])))));
    vm->bind_func<1>(mod, "isinf", CPP_LAMBDA(VAR(std::isinf(vm->num_to_float(args[0])))));
    vm->bind_func<1>(mod, "fabs", CPP_LAMBDA(VAR(std::fabs(vm->num_to_float(args[0])))));
    vm->bind_func<1>(mod, "floor", CPP_LAMBDA(VAR((i64)std::floor(vm->num_to_float(args[0])))));
    vm->bind_func<1>(mod, "ceil", CPP_LAMBDA(VAR((i64)std::ceil(vm->num_to_float(args[0])))));
    vm->bind_func<1>(mod, "sqrt", CPP_LAMBDA(VAR(std::sqrt(vm->num_to_float(args[0])))));
}

void add_module_dis(VM* vm){
    PyVar mod = vm->new_module("dis");
    vm->bind_func<1>(mod, "dis", [](VM* vm, Args& args) {
        PyVar f = args[0];
        if(is_type(f, vm->tp_bound_method)) f = CAST(BoundMethod, args[0]).method;
        CodeObject_ code = CAST(Function, f).code;
        (*vm->_stdout) << vm->disassemble(code);
        return vm->None;
    });
}

struct ReMatch {
    PY_CLASS(ReMatch, re, Match)

    i64 start;
    i64 end;
    std::smatch m;
    ReMatch(i64 start, i64 end, std::smatch m) : start(start), end(end), m(m) {}

    static void _register(VM* vm, PyVar mod, PyVar type){
        vm->bind_method<-1>(type, "__init__", CPP_NOT_IMPLEMENTED());
        vm->bind_method<0>(type, "start", CPP_LAMBDA(VAR(CAST(ReMatch&, args[0]).start)));
        vm->bind_method<0>(type, "end", CPP_LAMBDA(VAR(CAST(ReMatch&, args[0]).end)));

        vm->bind_method<0>(type, "span", [](VM* vm, Args& args) {
            auto& self = CAST(ReMatch&, args[0]);
            return VAR(two_args(VAR(self.start), VAR(self.end)));
        });

        vm->bind_method<1>(type, "group", [](VM* vm, Args& args) {
            auto& self = CAST(ReMatch&, args[0]);
            int index = CAST(int, args[1]);
            index = vm->normalized_index(index, self.m.size());
            return VAR(self.m[index].str());
        });
    }
};

PyVar _regex_search(const Str& pattern, const Str& string, bool fromStart, VM* vm){
    std::regex re(pattern);
    std::smatch m;
    if(std::regex_search(string, m, re)){
        if(fromStart && m.position() != 0) return vm->None;
        i64 start = string._to_u8_index(m.position());
        i64 end = string._to_u8_index(m.position() + m.length());
        return VAR_T(ReMatch, start, end, m);
    }
    return vm->None;
};

void add_module_re(VM* vm){
    PyVar mod = vm->new_module("re");
    ReMatch::register_class(vm, mod);

    vm->bind_func<2>(mod, "match", [](VM* vm, Args& args) {
        const Str& pattern = CAST(Str&, args[0]);
        const Str& string = CAST(Str&, args[1]);
        return _regex_search(pattern, string, true, vm);
    });

    vm->bind_func<2>(mod, "search", [](VM* vm, Args& args) {
        const Str& pattern = CAST(Str&, args[0]);
        const Str& string = CAST(Str&, args[1]);
        return _regex_search(pattern, string, false, vm);
    });

    vm->bind_func<3>(mod, "sub", [](VM* vm, Args& args) {
        const Str& pattern = CAST(Str&, args[0]);
        const Str& repl = CAST(Str&, args[1]);
        const Str& string = CAST(Str&, args[2]);
        std::regex re(pattern);
        return VAR(std::regex_replace(string, re, repl));
    });

    vm->bind_func<2>(mod, "split", [](VM* vm, Args& args) {
        const Str& pattern = CAST(Str&, args[0]);
        const Str& string = CAST(Str&, args[1]);
        std::regex re(pattern);
        std::sregex_token_iterator it(string.begin(), string.end(), re, -1);
        std::sregex_token_iterator end;
        List vec;
        for(; it != end; ++it){
            vec.push_back(VAR(it->str()));
        }
        return VAR(vec);
    });
}

void add_module_random(VM* vm){
    PyVar mod = vm->new_module("random");
    std::srand(std::time(0));
    vm->bind_func<1>(mod, "seed", [](VM* vm, Args& args) {
        std::srand((unsigned int)CAST(i64, args[0]));
        return vm->None;
    });

    vm->bind_func<0>(mod, "random", CPP_LAMBDA(VAR(std::rand() / (f64)RAND_MAX)));
    vm->bind_func<2>(mod, "randint", [](VM* vm, Args& args) {
        i64 a = CAST(i64, args[0]);
        i64 b = CAST(i64, args[1]);
        if(a > b) std::swap(a, b);
        return VAR(a + std::rand() % (b - a + 1));
    });

    vm->bind_func<2>(mod, "uniform", [](VM* vm, Args& args) {
        f64 a = CAST(f64, args[0]);
        f64 b = CAST(f64, args[1]);
        if(a > b) std::swap(a, b);
        return VAR(a + (b - a) * std::rand() / (f64)RAND_MAX);
    });

    CodeObject_ code = vm->compile(kPythonLibs["random"], "random.py", EXEC_MODE);
    vm->_exec(code, mod);
}

void add_module_functools(VM* vm){
    PyVar mod = vm->new_module("functools");
    CodeObject_ code = vm->compile(kPythonLibs["functools"], "functools.py", EXEC_MODE);
    vm->_exec(code, mod);
}

void VM::post_init(){
    init_builtins(this);
    add_module_sys(this);
    add_module_time(this);
    add_module_json(this);
    add_module_math(this);
    add_module_re(this);
    add_module_dis(this);
    add_module_random(this);
    add_module_io(this);
    add_module_os(this);
    add_module_functools(this);
    add_module_c(this);

    CodeObject_ code = compile(kPythonLibs["builtins"], "<builtins>", EXEC_MODE);
    this->_exec(code, this->builtins);
    code = compile(kPythonLibs["dict"], "<builtins>", EXEC_MODE);
    this->_exec(code, this->builtins);
    code = compile(kPythonLibs["set"], "<builtins>", EXEC_MODE);
    this->_exec(code, this->builtins);
}

}   // namespace pkpy

/*************************GLOBAL NAMESPACE*************************/

class PkExportedBase{
public:
    virtual ~PkExportedBase() = default;
    virtual void* get() = 0;
};

static std::vector<PkExportedBase*> _pk_lookup_table;
template<typename T>
class PkExported : public PkExportedBase{
    T* _ptr;
public:
    template<typename... Args>
    PkExported(Args&&... args) {
        _ptr = new T(std::forward<Args>(args)...);
        _pk_lookup_table.push_back(this);
    }
    
    ~PkExported() override { delete _ptr; }
    void* get() override { return _ptr; }
    operator T*() { return _ptr; }
};

#define PKPY_ALLOCATE(T, ...) *(new PkExported<T>(__VA_ARGS__))

extern "C" {
    __EXPORT
    /// Delete a pointer allocated by `pkpy_xxx_xxx`.
    /// It can be `VM*`, `REPL*`, `char*`, etc.
    /// 
    /// !!!
    /// If the pointer is not allocated by `pkpy_xxx_xxx`, the behavior is undefined.
    /// !!!
    void pkpy_delete(void* p){
        for(int i = 0; i < _pk_lookup_table.size(); i++){
            if(_pk_lookup_table[i]->get() == p){
                delete _pk_lookup_table[i];
                _pk_lookup_table.erase(_pk_lookup_table.begin() + i);
                return;
            }
        }
        free(p);
    }

    __EXPORT
    /// Run a given source on a virtual machine.
    void pkpy_vm_exec(pkpy::VM* vm, const char* source){
        vm->exec(source, "main.py", pkpy::EXEC_MODE);
    }

    __EXPORT
    /// Get a global variable of a virtual machine.
    /// 
    /// Return `__repr__` of the result.
    /// If the variable is not found, return `nullptr`.
    char* pkpy_vm_get_global(pkpy::VM* vm, const char* name){
        pkpy::PyVar* val = vm->_main->attr().try_get(name);
        if(val == nullptr) return nullptr;
        try{
            pkpy::Str repr = pkpy::CAST(pkpy::Str, vm->asRepr(*val));
            return strdup(repr.c_str());
        }catch(...){
            return nullptr;
        }
    }

    __EXPORT
    /// Evaluate an expression.
    /// 
    /// Return `__repr__` of the result.
    /// If there is any error, return `nullptr`.
    char* pkpy_vm_eval(pkpy::VM* vm, const char* source){
        pkpy::PyVarOrNull ret = vm->exec(source, "<eval>", pkpy::EVAL_MODE);
        if(ret == nullptr) return nullptr;
        try{
            pkpy::Str repr = pkpy::CAST(pkpy::Str, vm->asRepr(ret));
            return strdup(repr.c_str());
        }catch(...){
            return nullptr;
        }
    }

    __EXPORT
    /// Create a REPL, using the given virtual machine as the backend.
    pkpy::REPL* pkpy_new_repl(pkpy::VM* vm){
        return PKPY_ALLOCATE(pkpy::REPL, vm);
    }

    __EXPORT
    /// Input a source line to an interactive console. Return true if need more lines.
    bool pkpy_repl_input(pkpy::REPL* r, const char* line){
        return r->input(line);
    }

    __EXPORT
    /// Add a source module into a virtual machine.
    void pkpy_vm_add_module(pkpy::VM* vm, const char* name, const char* source){
        vm->_lazy_modules[name] = source;
    }

    __EXPORT
    /// Create a virtual machine.
    pkpy::VM* pkpy_new_vm(bool use_stdio){
        return PKPY_ALLOCATE(pkpy::VM, use_stdio);
    }

    __EXPORT
    /// Read the standard output and standard error as string of a virtual machine.
    /// The `vm->use_stdio` should be `false`.
    /// After this operation, both stream will be cleared.
    ///
    /// Return a json representing the result.
    char* pkpy_vm_read_output(pkpy::VM* vm){
        if(vm->use_stdio) return nullptr;
        pkpy::StrStream* s_out = (pkpy::StrStream*)(vm->_stdout);
        pkpy::StrStream* s_err = (pkpy::StrStream*)(vm->_stderr);
        pkpy::Str _stdout = s_out->str();
        pkpy::Str _stderr = s_err->str();
        pkpy::StrStream ss;
        ss << '{' << "\"stdout\": " << _stdout.escape(false);
        ss << ", " << "\"stderr\": " << _stderr.escape(false) << '}';
        s_out->str(""); s_err->str("");
        return strdup(ss.str().c_str());
    }

    typedef i64 (*f_int_t)(char*);
    typedef f64 (*f_float_t)(char*);
    typedef bool (*f_bool_t)(char*);
    typedef char* (*f_str_t)(char*);
    typedef void (*f_None_t)(char*);

    static f_int_t f_int = nullptr;
    static f_float_t f_float = nullptr;
    static f_bool_t f_bool = nullptr;
    static f_str_t f_str = nullptr;
    static f_None_t f_None = nullptr;

    __EXPORT
    /// Setup the callback functions.
    void pkpy_setup_callbacks(f_int_t _f_int, f_float_t _f_float, f_bool_t _f_bool, f_str_t _f_str, f_None_t _f_None){
        f_int = _f_int;
        f_float = _f_float;
        f_bool = _f_bool;
        f_str = _f_str;
        f_None = _f_None;
    }

    __EXPORT
    /// Bind a function to a virtual machine.
    char* pkpy_vm_bind(pkpy::VM* vm, const char* mod, const char* name, int ret_code){
        if(!f_int || !f_float || !f_bool || !f_str || !f_None) return nullptr;
        static int kGlobalBindId = 0;
        for(int i=0; mod[i]; i++) if(mod[i] == ' ') return nullptr;
        for(int i=0; name[i]; i++) if(name[i] == ' ') return nullptr;
        std::string f_header = std::string(mod) + '.' + name + '#' + std::to_string(kGlobalBindId++);
        pkpy::PyVar obj = vm->_modules.contains(mod) ? vm->_modules[mod] : vm->new_module(mod);
        vm->bind_func<-1>(obj, name, [ret_code, f_header](pkpy::VM* vm, const pkpy::Args& args){
            pkpy::StrStream ss;
            ss << f_header;
            for(int i=0; i<args.size(); i++){
                ss << ' ';
                pkpy::PyVar x = vm->call(args[i], pkpy::__json__);
                ss << pkpy::CAST(pkpy::Str&, x);
            }
            char* packet = strdup(ss.str().c_str());
            switch(ret_code){
                case 'i': return VAR(f_int(packet));
                case 'f': return VAR(f_float(packet));
                case 'b': return VAR(f_bool(packet));
                case 's': {
                    char* p = f_str(packet);
                    if(p == nullptr) return vm->None;
                    return VAR(p); // no need to free(p)
                }
                case 'N': f_None(packet); return vm->None;
            }
            free(packet);
            UNREACHABLE();
            return vm->None;
        });
        return strdup(f_header.c_str());
    }
}

#endif // POCKETPY_H