#pragma once

#include <vector>
#include <string>
#include <sstream>

typedef std::stringstream _StrStream;


class _Str {
private:
    mutable bool utf8_initialized = false;
    mutable std::vector<uint16_t> _u8_index;    // max_len is 65535

    std::string _s;

    mutable bool hash_initialized = false;
    mutable size_t _hash;

    void utf8_lazy_init() const{
        if(utf8_initialized) return;
        for(uint16_t i = 0; i < size(); i++){
            // https://stackoverflow.com/questions/3911536/utf-8-unicode-whats-with-0xc0-and-0x80
            if((_s[i] & 0xC0) != 0x80)
                _u8_index.push_back(i);
        }
        utf8_initialized = true;
    }
public:
    _Str(const char* s): _s(s) {}
    _Str(const char* s, size_t len): _s(s, len) {}
    _Str(int n, char fill = ' '): _s(n, fill) {}
    _Str(const std::string& s): _s(s) {}
    _Str(std::string&& s): _s(std::move(s)) {}
    _Str(const _StrStream& ss): _s(ss.str()) {}
    _Str(){}
    
    size_t hash() const{
        if(!hash_initialized){
            _hash = std::hash<std::string>()(_s);
            hash_initialized = true;
        }
        return _hash;
    }

    int u8_length() const {
        utf8_lazy_init();
        return _u8_index.size();
    }

    _Str u8_getitem(int i) const{
        return u8_substr(i, i+1);
    }

    _Str u8_substr(int start, int end) const{
        utf8_lazy_init();
        if(start >= end) return _Str();
        int c_end = end >= _u8_index.size() ? size() : _u8_index[end];
        return _s.substr(_u8_index.at(start), c_end - _u8_index.at(start));
    }

    int size() const {
        return _s.size();
    }

    bool empty() const {
        return _s.empty();
    }

    bool operator==(const _Str& other) const {
        return _s == other._s;
    }

    bool operator!=(const _Str& other) const {
        return _s != other._s;
    }

    bool operator<(const _Str& other) const {
        return _s < other._s;
    }

    bool operator>(const _Str& other) const {
        return _s > other._s;
    }

    char operator[](int i) const {
        return _s[i];
    }

    friend std::ostream& operator<<(std::ostream& os, const _Str& s){
        os << s._s;
        return os;
    }

    _Str operator+(const _Str& other) const {
        return _Str(_s + other._s);
    }

    _Str operator+(const char* other) const {
        return _Str(_s + other);
    }

    _Str operator+(const std::string& other) const {
        return _Str(_s + other);
    }

    friend _Str operator+(const char* other, const _Str& s){
        return _Str(other + s._s);
    }

    friend _Str operator+(const std::string& other, const _Str& s){
        return _Str(other + s._s);
    }

    const std::string& str() const {
        return _s;
    }

    static const std::size_t npos = std::string::npos;

    operator const char*() const {
        return _s.c_str();
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

const _Str& __class__ = _Str("__class__");
const _Str& __base__ = _Str("__base__");
const _Str& __new__ = _Str("__new__");
const _Str& __iter__ = _Str("__iter__");
const _Str& __str__ = _Str("__str__");
const _Str& __neg__ = _Str("__neg__");
const _Str& __getitem__ = _Str("__getitem__");
const _Str& __setitem__ = _Str("__setitem__");
const _Str& __delitem__ = _Str("__delitem__");
const _Str& __contains__ = _Str("__contains__");
const _Str& __init__ = _Str("__init__");

const _Str CMP_SPECIAL_METHODS[] = {
    "__lt__", "__le__", "__eq__", "__ne__", "__gt__", "__ge__"
};  // __ne__ should not be used

const _Str BIN_SPECIAL_METHODS[] = {
    "__add__", "__sub__", "__mul__", "__truediv__", "__floordiv__", "__mod__", "__pow__"
};