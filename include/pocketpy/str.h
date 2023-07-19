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