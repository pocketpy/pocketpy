#pragma once

#include "common.h"
#include "memory.h"
#include "vector.h"

namespace pkpy {

int utf8len(unsigned char c, bool suppress=false);
struct SStream;

struct Str{
    int size;
    bool is_ascii;
    char* data;
    char _inlined[16];

    bool is_inlined() const { return data == _inlined; }

    Str();
    Str(int size, bool is_ascii);
    Str(const std::string& s);
    Str(std::string_view s);
    Str(const char* s);
    Str(const char* s, int len);
    Str(std::pair<char *, int>);
    Str(const Str& other);
    Str(Str&& other);

    operator std::string_view() const { return sv(); }

    const char* begin() const { return data; }
    const char* end() const { return data + size; }
    char operator[](int idx) const { return data[idx]; }
    int length() const { return size; }
    bool empty() const { return size == 0; }
    size_t hash() const{ return std::hash<std::string_view>()(sv()); }

    Str& operator=(const Str&);
    Str operator+(const Str&) const;
    Str operator+(const char*) const;
    friend Str operator+(const char*, const Str&);

    bool operator==(const std::string_view other) const;
    bool operator!=(const std::string_view other) const;
    bool operator<(const std::string_view other) const;
    friend bool operator<(const std::string_view other, const Str& str);

    bool operator==(const char* p) const;
    bool operator!=(const char* p) const;

    bool operator==(const Str& other) const;
    bool operator!=(const Str& other) const;
    bool operator<(const Str& other) const;
    bool operator>(const Str& other) const;
    bool operator<=(const Str& other) const;
    bool operator>=(const Str& other) const;

    ~Str();

    friend std::ostream& operator<<(std::ostream& os, const Str& str);

    const char* c_str() const { return data; }
    std::string_view sv() const { return std::string_view(data, size); }
    std::string str() const { return std::string(data, size); }

    Str substr(int start, int len) const;
    Str substr(int start) const;
    Str strip(bool left, bool right, const Str& chars) const;
    Str strip(bool left=true, bool right=true) const;
    Str lstrip() const { return strip(true, false); }
    Str rstrip() const { return strip(false, true); }
    Str lower() const;
    Str upper() const;
    Str escape(bool single_quote=true) const;
    void escape_(SStream& ss, bool single_quote=true) const;
    int index(const Str& sub, int start=0) const;
    Str replace(char old, char new_) const;
    Str replace(const Str& old, const Str& new_, int count=-1) const;
    pod_vector<std::string_view> split(const Str& sep) const;
    pod_vector<std::string_view> split(char sep) const;
    int count(const Str& sub) const;

    /*************unicode*************/
    int _unicode_index_to_byte(int i) const;
    int _byte_index_to_unicode(int n) const;
    Str u8_getitem(int i) const;
    Str u8_slice(int start, int stop, int step) const;
    int u8_length() const;
};

struct StrName {
    uint16_t index;

    StrName(): index(0) {}
    explicit StrName(uint16_t index): index(index) {}
    StrName(const char* s): index(get(s).index) {}
    StrName(const Str& s): index(get(s.sv()).index) {}

    std::string_view sv() const { return _r_interned()[index];}
    const char* c_str() const { return _r_interned()[index].c_str(); }

    bool empty() const { return index == 0; }
    Str escape() const { return Str(sv()).escape(); }

    bool operator==(const StrName& other) const noexcept {
        return this->index == other.index;
    }

    bool operator!=(const StrName& other) const noexcept {
        return this->index != other.index;
    }

    bool operator<(const StrName& other) const noexcept {
        return sv() < other.sv();
    }

    bool operator>(const StrName& other) const noexcept {
        return sv() > other.sv();
    }

    static bool is_valid(int index);
    static StrName get(std::string_view s);
    static std::map<std::string, uint16_t, std::less<>>& _interned();
    static std::map<uint16_t, std::string>& _r_interned();
    static uint32_t _pesudo_random_index;
};

struct SStream{
    PK_ALWAYS_PASS_BY_POINTER(SStream)
    // pod_vector<T> is allocated by pool64 so the buffer can be moved into Str without a copy
    pod_vector<char> buffer;
    int _precision = -1;

    bool empty() const { return buffer.empty(); }
    void setprecision(int precision) { _precision = precision; }

    SStream(){}
    SStream(int guess_size){ buffer.reserve(guess_size); }

    Str str();

    SStream& operator<<(const Str&);
    SStream& operator<<(const char*);
    SStream& operator<<(int);
    SStream& operator<<(size_t);
    SStream& operator<<(i64);
    SStream& operator<<(f64);
    SStream& operator<<(const std::string&);
    SStream& operator<<(std::string_view);
    SStream& operator<<(char);
    SStream& operator<<(StrName);

    void write_hex(unsigned char, bool non_zero=false);
    void write_hex(void*);
    void write_hex(i64);
};

#ifdef _S
#undef _S
#endif

template<typename... Args>
Str _S(Args&&... args) {
    SStream ss;
    (ss << ... << args);
    return ss.str();
}

struct CString{
	const char* ptr;
	CString(const char* ptr): ptr(ptr) {}
    operator const char*() const { return ptr; }
};

// unary operators
const StrName __repr__ = StrName::get("__repr__");
const StrName __str__ = StrName::get("__str__");
const StrName __hash__ = StrName::get("__hash__");
const StrName __len__ = StrName::get("__len__");
const StrName __iter__ = StrName::get("__iter__");
const StrName __next__ = StrName::get("__next__");
const StrName __neg__ = StrName::get("__neg__");
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

// specials
const StrName __new__ = StrName::get("__new__");
const StrName __init__ = StrName::get("__init__");
const StrName __call__ = StrName::get("__call__");
const StrName __divmod__ = StrName::get("__divmod__");
const StrName __enter__ = StrName::get("__enter__");
const StrName __exit__ = StrName::get("__exit__");
const StrName __name__ = StrName::get("__name__");
const StrName __all__ = StrName::get("__all__");
const StrName __package__ = StrName::get("__package__");
const StrName __path__ = StrName::get("__path__");
const StrName __class__ = StrName::get("__class__");
const StrName __missing__ = StrName::get("__missing__");

const StrName pk_id_add = StrName::get("add");
const StrName pk_id_set = StrName::get("set");
const StrName pk_id_long = StrName::get("long");
const StrName pk_id_complex = StrName::get("complex");

#define DEF_SNAME(name) const static StrName name(#name)

} // namespace pkpy