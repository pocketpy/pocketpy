#pragma once

#include "pocketpy/common/utils.h"
#include "pocketpy/common/memorypool.hpp"
#include "pocketpy/common/vector.hpp"
#include "pocketpy/common/str.h"

#include <string_view>
#include <ostream>

namespace pkpy {

struct SStream;

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

    Str(pair<char*, int>);      // take ownership

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

    vector<std::string_view> split(const Str& sep) const;
    vector<std::string_view> split(char sep) const;

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

    explicit StrName(uint16_t index) : index(index) {}

    StrName(const char* s) : index(get(s).index) {}

    StrName(const Str& s) : index(get(s.sv()).index) {}

    std::string_view sv() const;
    const char* c_str() const;

    bool empty() const { return index == 0; }

    Str escape() const { return Str(sv()).escape(); }

    bool operator== (const StrName& other) const noexcept { return this->index == other.index; }

    bool operator!= (const StrName& other) const noexcept { return this->index != other.index; }

    bool operator< (const StrName& other) const noexcept { return sv() < other.sv(); }

    bool operator> (const StrName& other) const noexcept { return sv() > other.sv(); }

    static StrName get(std::string_view s);
    static uint32_t _pesudo_random_index;
};

struct SStream {
    PK_ALWAYS_PASS_BY_POINTER(SStream)

    vector<char> buffer;
    int _precision = -1;

    bool empty() const { return buffer.empty(); }

    void setprecision(int precision) { _precision = precision; }

    SStream() {}

    SStream(int guess_size) { buffer.reserve(guess_size); }

    Str str();

    SStream& operator<< (const Str&);
    SStream& operator<< (const char*);
    SStream& operator<< (int);
    SStream& operator<< (size_t);
    SStream& operator<< (i64);
    SStream& operator<< (f64);
    SStream& operator<< (const std::string&);
    SStream& operator<< (std::string_view);
    SStream& operator<< (char);
    SStream& operator<< (StrName);

    void write_hex(unsigned char, bool non_zero = false);
    void write_hex(void*);
    void write_hex(i64);
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

// unary operators
extern const StrName __repr__;
extern const StrName __str__;
extern const StrName __hash__;
extern const StrName __len__;
extern const StrName __iter__;
extern const StrName __next__;
extern const StrName __neg__;
// logical operators
extern const StrName __eq__;
extern const StrName __lt__;
extern const StrName __le__;
extern const StrName __gt__;
extern const StrName __ge__;
extern const StrName __contains__;
// binary operators
extern const StrName __add__;
extern const StrName __radd__;
extern const StrName __sub__;
extern const StrName __rsub__;
extern const StrName __mul__;
extern const StrName __rmul__;
extern const StrName __truediv__;
extern const StrName __floordiv__;
extern const StrName __mod__;
extern const StrName __pow__;
extern const StrName __matmul__;
extern const StrName __lshift__;
extern const StrName __rshift__;
extern const StrName __and__;
extern const StrName __or__;
extern const StrName __xor__;
extern const StrName __invert__;
// indexer
extern const StrName __getitem__;
extern const StrName __setitem__;
extern const StrName __delitem__;

// specials
extern const StrName __new__;
extern const StrName __init__;
extern const StrName __call__;
extern const StrName __divmod__;
extern const StrName __enter__;
extern const StrName __exit__;
extern const StrName __name__;
extern const StrName __all__;
extern const StrName __package__;
extern const StrName __path__;
extern const StrName __class__;
extern const StrName __missing__;

extern const StrName pk_id_add;
extern const StrName pk_id_set;
extern const StrName pk_id_long;
extern const StrName pk_id_complex;

#define DEF_SNAME(name) const static StrName name(#name)

}  // namespace pkpy
