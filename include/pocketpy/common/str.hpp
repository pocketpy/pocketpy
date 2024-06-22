#pragma once

#include "pocketpy/common/sstream.h"
#include "pocketpy/common/utils.h"
#include "pocketpy/common/memorypool.h"
#include "pocketpy/common/vector.h"
#include "pocketpy/common/vector.hpp"
#include "pocketpy/common/str.h"
#include "pocketpy/common/strname.h"

#include <cassert>
#include <string_view>
#include <string>
#include <ostream>

namespace pkpy {

struct Str: pkpy_Str {
    bool is_inlined() const { return is_sso; }

    Str(){
        pkpy_Str__ctor2(this, "", 0);
    }

    Str(pkpy_Str&& s){
        std::memcpy(this, &s, sizeof(pkpy_Str));
    }

    Str(const std::string& s){
        pkpy_Str__ctor2(this, s.data(), s.size());
    }

    Str(std::string_view s){
        pkpy_Str__ctor2(this, s.data(), s.size());
    }

    Str(const char* s){
        pkpy_Str__ctor2(this, s, strlen(s));
    }

    Str(const char* s, int len){
        pkpy_Str__ctor2(this, s, len);
    }

    Str(pair<char*, int> detached) {
        this->size = detached.second;
        this->is_ascii = c11__isascii(detached.first, detached.second);
        this->is_sso = false;
        this->_ptr = detached.first;
        assert(_ptr[size] == '\0');
    }

    Str(const Str& other){
        pkpy_Str__ctor2(this, pkpy_Str__data(&other), other.size);
    }

    Str(Str&& other){
        std::memcpy(this, &other, sizeof(pkpy_Str));
        other.size = 0;
        other.is_sso = true;
    }

    operator std::string_view () const { return sv(); }
    const char* begin() const { return pkpy_Str__data(this); }
    const char* end() const { return pkpy_Str__data(this) + size; }
    int length() const { return size; }
    char operator[] (int idx) const { return pkpy_Str__data(this)[idx]; }
    bool empty() const { return size == 0; }
    size_t hash() const { return std::hash<std::string_view>()(sv()); }

    Str& operator= (const Str& other){
        pkpy_Str__dtor(this);
        pkpy_Str__ctor2(this, pkpy_Str__data(&other), other.size);
        return *this;
    }

    Str operator+ (const Str& other) const{
        return pkpy_Str__concat(this, &other);
    }

    Str operator+ (const char* other) const{
        return pkpy_Str__concat2(this, other, strlen(other));
    }

    friend Str operator+ (const char* self, const Str& other){
        pkpy_Str tmp;
        pkpy_Str__ctor2(&tmp, self, strlen(self));
        pkpy_Str retval = pkpy_Str__concat(&tmp, &other);
        pkpy_Str__dtor(&tmp);
        return retval;
    }

    bool operator== (const std::string_view other) const{
        int res = pkpy_Str__cmp2(this, other.data(), other.size());
        return res == 0;
    }

    bool operator!= (const std::string_view other) const{
        int res = pkpy_Str__cmp2(this, other.data(), other.size());
        return res != 0;
    }

    bool operator< (const std::string_view other) const{
        int res = pkpy_Str__cmp2(this, other.data(), other.size());
        return res < 0;
    }

    friend bool operator< (const std::string_view other, const Str& str){
        int res = pkpy_Str__cmp2(&str, other.data(), other.size());
        return res > 0;
    }

    bool operator== (const char* p) const{
        int res = pkpy_Str__cmp2(this, p, strlen(p));
        return res == 0;
    }

    bool operator!= (const char* p) const{
        int res = pkpy_Str__cmp2(this, p, strlen(p));
        return res != 0;
    }

    bool operator== (const Str& other) const{
        return pkpy_Str__cmp(this, &other) == 0;
    }
    bool operator!= (const Str& other) const{
        return pkpy_Str__cmp(this, &other) != 0;
    }
    bool operator< (const Str& other) const{
        return pkpy_Str__cmp(this, &other) < 0;
    }
    bool operator> (const Str& other) const{
        return pkpy_Str__cmp(this, &other) > 0;
    }
    bool operator<= (const Str& other) const{
        return pkpy_Str__cmp(this, &other) <= 0;
    }
    bool operator>= (const Str& other) const{
        return pkpy_Str__cmp(this, &other) >= 0;
    }

    ~Str(){
        pkpy_Str__dtor(this);
    }

    friend std::ostream& operator<< (std::ostream& os, const Str& self){
        os.write(pkpy_Str__data(&self), self.size);
        return os;
    }

    const char* c_str() const { return pkpy_Str__data(this); }

    std::string_view sv() const {
        return std::string_view(pkpy_Str__data(this), size);
    }

    std::string str() const {
        return std::string(pkpy_Str__data(this), size);
    }

    Str slice(int start, int stop) const{
        return pkpy_Str__slice2(this, start, stop);
    }

    Str slice(int start) const{
        return pkpy_Str__slice(this, start);
    }

    Str substr(int start) const{
        return pkpy_Str__slice(this, start);
    }

    Str strip(bool left, bool right, const Str& chars) const{
        return pkpy_Str__strip2(this, left, right, &chars);
    }

    Str strip(bool left = true, bool right = true) const{
        return pkpy_Str__strip(this, left, right);
    }

    Str lstrip() const { return strip(true, false); }

    Str rstrip() const { return strip(false, true); }

    Str lower() const{
        return pkpy_Str__lower(this);
    }
    Str upper() const{
        return pkpy_Str__upper(this);
    }
    Str replace(char old, char new_) const{
        return pkpy_Str__replace(this, old, new_);
    }
    Str replace(const Str& old, const Str& new_) const{
        return pkpy_Str__replace2(this, &old, &new_);
    }

    Str escape(char quote='\'') const{
        return pkpy_Str__escape(this, quote);
    }

    vector<std::string_view> split(const Str& sep) const{
        c11_vector/* T=c11_string */ res = pkpy_Str__split2(this, &sep);
        vector<std::string_view> retval(res.count);
        for(int i = 0; i < res.count; i++){
            c11_string tmp = c11__getitem(c11_string, &res, i);
            retval[i] = std::string_view(tmp.data, tmp.size);
        }
        c11_vector__dtor(&res);
        return retval;
    }

    vector<std::string_view> split(char sep) const{
        c11_vector/* T=c11_string */ res = pkpy_Str__split(this, sep);
        vector<std::string_view> retval(res.count);
        for(int i = 0; i < res.count; i++){
            c11_string tmp = c11__getitem(c11_string, &res, i);
            retval[i] = std::string_view(tmp.data, tmp.size);
        }
        c11_vector__dtor(&res);
        return retval;
    }

    int index(const Str& sub, int start = 0) const{
        return pkpy_Str__index(this, &sub, start);
    }

    int count(const Str& sub) const{
        return pkpy_Str__count(this, &sub);
    }

    /*************unicode*************/
    int _unicode_index_to_byte(int i) const{
        return pkpy_Str__unicode_index_to_byte(this, i);
    }

    int _byte_index_to_unicode(int n) const{
        return pkpy_Str__byte_index_to_unicode(this, n);
    }

    Str u8_getitem(int i) const{
        return pkpy_Str__u8_getitem(this, i);
    }

    Str u8_slice(int start, int stop, int step) const{
        return pkpy_Str__u8_slice(this, start, stop, step);
    }

    int u8_length() const{
        return pkpy_Str__u8_length(this);
    }
};

struct StrName {
    uint16_t index;

    StrName() : index(0) {}

    StrName(uint16_t index) : index(index) {}

    StrName(const char* s) : index(get(s).index) {}

    StrName(const Str& s) : index(get(s.sv()).index) {}

    bool empty() const { return index == 0; }

    Str escape() const { return Str(sv()).escape(); }

    bool operator== (const StrName& other) const noexcept { return this->index == other.index; }

    bool operator!= (const StrName& other) const noexcept { return this->index != other.index; }

    bool operator< (const StrName& other) const noexcept { return sv() < other.sv(); }

    bool operator> (const StrName& other) const noexcept { return sv() > other.sv(); }

    inline static StrName get(std::string_view s){
        uint16_t index = pkpy_StrName__map2({s.data(), (int)s.size()});
        return StrName(index);
    }

    std::string_view sv() const{
        return pkpy_StrName__rmap(index);
    }

    const char* c_str() const{
        return pkpy_StrName__rmap(index);
    }
};

struct SStream: pk_SStream {
    PK_ALWAYS_PASS_BY_POINTER(SStream)

    int _precision = -1;
    bool _submited = false;
    bool empty() const { return data.count == 0; }

    void setprecision(int precision) { _precision = precision; }

    SStream() {
        pk_SStream__ctor(this);
    }

    SStream(int guess_size) { c11_vector__reserve(&data, guess_size); }

    ~SStream() {
        // in case of error
        if(!_submited) pk_SStream__dtor(this);
    }

    Str str(){
        assert(!_submited);
        _submited = true;
        return pk_SStream__submit(this);
    }

    SStream& operator<< (const Str& val){
        pk_SStream__write_Str(this, &val);
        return *this;
    }

    SStream& operator<< (const char* val){
        pk_SStream__write_cstr(this, val);
        return *this;
    }

    SStream& operator<< (int val){
        pk_SStream__write_int(this, val);
        return *this;
    }

    SStream& operator<< (size_t val){
        // size_t could overflow int64, but nevermind...
        pk_SStream__write_i64(this, val);
        return *this;
    }

    SStream& operator<< (i64 val){
        pk_SStream__write_i64(this, val);
        return *this;
    }

    SStream& operator<< (f64 val){
        pk_SStream__write_double(this, val, _precision);
        return *this;
    }

    SStream& operator<< (const std::string& val){
        pk_SStream__write_cstrn(this, val.data(), val.size());
        return *this;
    }

    SStream& operator<< (std::string_view val){
        pk_SStream__write_cstrn(this, val.data(), val.size());
        return *this;
    }

    SStream& operator<< (c11_string val){
        pk_SStream__write_cstrn(this, val.data, val.size);
        return *this;
    }

    SStream& operator<< (char val){
        pk_SStream__write_char(this, val);
        return *this;
    }

    SStream& operator<< (StrName name){
        std::string_view sv = name.sv();
        pk_SStream__write_cstrn(this, sv.data(), sv.size());
        return *this;
    }

    void write_hex(unsigned char val, bool non_zero = false){
        pk_SStream__write_hex(this, val, non_zero);
    }

    void write_ptr(void* p){
        pk_SStream__write_ptr(this, p);
    }
};

#ifdef _S
#undef _S
#endif

template <typename... Args>
Str _S(Args&&... args) {
    SStream ss;
    (ss << ... << args);
    return ss.str();
}

struct CString {
    const char* ptr;

    CString(const char* ptr) : ptr(ptr) {}

    operator const char* () const { return ptr; }
};

#define DEF_SNAME(name) const static StrName name(#name)

}  // namespace pkpy
