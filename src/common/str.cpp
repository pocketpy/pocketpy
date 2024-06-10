#include "pocketpy/common/str.hpp"
#include "pocketpy/common/gil.hpp"

#include <cassert>
#include <ostream>
#include <algorithm>
#include <cmath>
#include <map>

namespace pkpy {

Str::Str(pair<char*, int> detached) {
    this->size = detached.second;
    this->is_ascii = true;
    this->is_sso = false;
    this->_ptr = detached.first;
    for(int i = 0; i < size; i++) {
        if(!isascii(_ptr[i])) {
            is_ascii = false;
            break;
        }
    }
    assert(_ptr[size] == '\0');
}

Str Str::strip(bool left, bool right, const Str& chars) const {
    int L = 0;
    int R = u8_length();
    if(left) {
        while(L < R && chars.index(u8_getitem(L)) != -1)
            L++;
    }
    if(right) {
        while(L < R && chars.index(u8_getitem(R - 1)) != -1)
            R--;
    }
    return u8_slice(L, R, 1);
}

Str Str::strip(bool left, bool right) const {
    const char* data = pkpy_Str__data(this);
    if(is_ascii) {
        int L = 0;
        int R = size;
        if(left) {
            while(L < R && (data[L] == ' ' || data[L] == '\t' || data[L] == '\n' || data[L] == '\r'))
                L++;
        }
        if(right) {
            while(L < R && (data[R - 1] == ' ' || data[R - 1] == '\t' || data[R - 1] == '\n' || data[R - 1] == '\r'))
                R--;
        }
        return substr(L, R - L);
    } else {
        return strip(left, right, " \t\n\r");
    }
}

Str Str::escape(bool single_quote) const {
    SStream ss;
    escape_(ss, single_quote);
    return ss.str();
}

void Str::escape_(SStream& ss, bool single_quote) const {
    ss << (single_quote ? '\'' : '"');
    for(int i = 0; i < size; i++) {
        char c = this->operator[] (i);
        switch(c) {
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
            case '\b': ss << "\\b"; break;
            default:
                if('\x00' <= c && c <= '\x1f') {
                    ss << "\\x";  // << std::hex << std::setw(2) << std::setfill('0') << (int)c;
                    ss << PK_HEX_TABLE[c >> 4];
                    ss << PK_HEX_TABLE[c & 0xf];
                } else {
                    ss << c;
                }
        }
    }
    ss << (single_quote ? '\'' : '"');
}

vector<std::string_view> Str::split(const Str& sep) const {
    vector<std::string_view> result;
    std::string_view tmp;
    int start = 0;
    while(true) {
        int i = index(sep, start);
        if(i == -1) break;
        tmp = sv().substr(start, i - start);
        if(!tmp.empty()) result.push_back(tmp);
        start = i + sep.size;
    }
    tmp = sv().substr(start, size - start);
    if(!tmp.empty()) result.push_back(tmp);
    return result;
}

vector<std::string_view> Str::split(char sep) const {
    vector<std::string_view> result;
    const char* data = pkpy_Str__data(this);
    int i = 0;
    for(int j = 0; j < size; j++) {
        if(data[j] == sep) {
            if(j > i) result.emplace_back(data + i, j - i);
            i = j + 1;
            continue;
        }
    }
    if(size > i) result.emplace_back(data + i, size - i);
    return result;
}

static std::map<std::string_view, uint16_t>& _interned() {
    static std::map<std::string_view, uint16_t> interned;
    return interned;
}

static std::map<uint16_t, std::string>& _r_interned() {
    static std::map<uint16_t, std::string> r_interned;
    return r_interned;
}

std::string_view StrName::sv() const { return _r_interned()[index]; }
const char* StrName::c_str() const { return _r_interned()[index].c_str(); }

uint32_t StrName::_pesudo_random_index = 0;

StrName StrName::get(std::string_view s) {
    PK_GLOBAL_SCOPE_LOCK()
    auto it = _interned().find(s);
    if(it != _interned().end()) return StrName(it->second);
    // generate new index
    // https://github.com/python/cpython/blob/3.12/Objects/dictobject.c#L175
    uint16_t index = ((_pesudo_random_index * 5) + 1) & 65535;
    if(index == 0) PK_FATAL_ERROR("StrName index overflow\n")
    auto res = _r_interned().emplace(index, s);
    assert(res.second);
    s = std::string_view(res.first->second);
    _interned()[s] = index;
    _pesudo_random_index = index;
    return StrName(index);
}

Str SStream::str() {
    // after this call, the buffer is no longer valid
    buffer.push_back('\0');
    auto detached = buffer.detach();
    detached.second--;  // remove the last '\0'
    return Str(detached);
}

SStream& SStream::operator<< (const Str& s) {
    for(char c: s)
        buffer.push_back(c);
    return *this;
}

SStream& SStream::operator<< (const char* s) {
    while(*s)
        buffer.push_back(*s++);
    return *this;
}

SStream& SStream::operator<< (const std::string& s) {
    for(char c: s)
        buffer.push_back(c);
    return *this;
}

SStream& SStream::operator<< (std::string_view s) {
    for(char c: s)
        buffer.push_back(c);
    return *this;
}

SStream& SStream::operator<< (char c) {
    buffer.push_back(c);
    return *this;
}

SStream& SStream::operator<< (StrName sn) { return *this << sn.sv(); }

SStream& SStream::operator<< (size_t val) {
    // size_t could be out of range of `i64`, use `std::to_string` instead
    return (*this) << std::to_string(val);
}

SStream& SStream::operator<< (int val) { return (*this) << static_cast<i64>(val); }

SStream& SStream::operator<< (i64 val) {
    // str(-2**64).__len__() == 21
    buffer.reserve(buffer.size() + 24);
    if(val == 0) {
        buffer.push_back('0');
        return *this;
    }
    if(val < 0) {
        buffer.push_back('-');
        val = -val;
    }
    auto begin = buffer.end();
    while(val) {
        buffer.push_back('0' + val % 10);
        val /= 10;
    }
    std::reverse(begin, buffer.end());
    return *this;
}

SStream& SStream::operator<< (f64 val) {
    if(std::isinf(val)) { return (*this) << (val > 0 ? "inf" : "-inf"); }
    if(std::isnan(val)) { return (*this) << "nan"; }
    char b[32];
    if(_precision == -1) {
        int prec = std::numeric_limits<f64>::max_digits10 - 1;
        snprintf(b, sizeof(b), "%.*g", prec, val);
    } else {
        int prec = _precision;
        snprintf(b, sizeof(b), "%.*f", prec, val);
    }
    (*this) << b;
    if(std::all_of(b + 1, b + strlen(b), isdigit)) (*this) << ".0";
    return *this;
}

void SStream::write_hex(unsigned char c, bool non_zero) {
    unsigned char high = c >> 4;
    unsigned char low = c & 0xf;
    if(non_zero) {
        if(high) (*this) << PK_HEX_TABLE[high];
        if(high || low) (*this) << PK_HEX_TABLE[low];
    } else {
        (*this) << PK_HEX_TABLE[high];
        (*this) << PK_HEX_TABLE[low];
    }
}

void SStream::write_hex(void* p) {
    if(p == nullptr) {
        (*this) << "0x0";
        return;
    }
    (*this) << "0x";
    uintptr_t p_t = reinterpret_cast<uintptr_t>(p);
    bool non_zero = true;
    for(int i = sizeof(void*) - 1; i >= 0; i--) {
        unsigned char cpnt = (p_t >> (i * 8)) & 0xff;
        write_hex(cpnt, non_zero);
        if(cpnt != 0) non_zero = false;
    }
}

void SStream::write_hex(i64 val) {
    if(val == 0) {
        (*this) << "0x0";
        return;
    }
    if(val < 0) {
        (*this) << "-";
        val = -val;
    }
    (*this) << "0x";
    bool non_zero = true;
    for(int i = 56; i >= 0; i -= 8) {
        unsigned char cpnt = (val >> i) & 0xff;
        write_hex(cpnt, non_zero);
        if(cpnt != 0) non_zero = false;
    }
}

#undef PK_STR_ALLOCATE
#undef PK_STR_COPY_INIT

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

}  // namespace pkpy
