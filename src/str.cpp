#include "pocketpy/str.h"

namespace pkpy {

int utf8len(unsigned char c, bool suppress){
    if((c & 0b10000000) == 0) return 1;
    if((c & 0b11100000) == 0b11000000) return 2;
    if((c & 0b11110000) == 0b11100000) return 3;
    if((c & 0b11111000) == 0b11110000) return 4;
    if((c & 0b11111100) == 0b11111000) return 5;
    if((c & 0b11111110) == 0b11111100) return 6;
    if(!suppress) throw std::runtime_error("invalid utf8 char: " + std::to_string(c));
    return 0;
}

    Str::Str(int size, bool is_ascii): size(size), is_ascii(is_ascii) {
        _alloc();
    }

#define STR_INIT()                                  \
        _alloc();                                   \
        for(int i=0; i<size; i++){                  \
            data[i] = s[i];                         \
            if(!isascii(s[i])) is_ascii = false;    \
        }

    Str::Str(const std::string& s): size(s.size()), is_ascii(true) {
        STR_INIT()
    }

    Str::Str(std::string_view s): size(s.size()), is_ascii(true) {
        STR_INIT()
    }

    Str::Str(const char* s): size(strlen(s)), is_ascii(true) {
        STR_INIT()
    }

    Str::Str(const char* s, int len): size(len), is_ascii(true) {
        STR_INIT()
    }

#undef STR_INIT

    Str::Str(const Str& other): size(other.size), is_ascii(other.is_ascii) {
        _alloc();
        memcpy(data, other.data, size);
    }

    Str::Str(Str&& other): size(other.size), is_ascii(other.is_ascii) {
        if(other.is_inlined()){
            data = _inlined;
            for(int i=0; i<size; i++) _inlined[i] = other._inlined[i];
        }else{
            data = other.data;
            other.data = other._inlined;
            other.size = 0;
        }
    }

    Str operator+(const char* p, const Str& str){
        Str other(p);
        return other + str;
    }

    std::ostream& operator<<(std::ostream& os, const Str& str){
        return os << str.sv();
    }

    bool operator<(const std::string_view other, const Str& str){
        return other < str.sv();
    }

    void Str::_alloc(){
        if(size <= 16){
            this->data = _inlined;
        }else{
            this->data = (char*)pool64_alloc(size);
        }
    }

    Str& Str::operator=(const Str& other){
        if(!is_inlined()) pool64_dealloc(data);
        size = other.size;
        is_ascii = other.is_ascii;
        _cached_c_str = nullptr;
        _alloc();
        memcpy(data, other.data, size);
        return *this;
    }

    Str Str::operator+(const Str& other) const {
        Str ret(size + other.size, is_ascii && other.is_ascii);
        memcpy(ret.data, data, size);
        memcpy(ret.data + size, other.data, other.size);
        return ret;
    }

    Str Str::operator+(const char* p) const {
        Str other(p);
        return *this + other;
    }

    bool Str::operator==(const Str& other) const {
        if(size != other.size) return false;
        return memcmp(data, other.data, size) == 0;
    }

    bool Str::operator!=(const Str& other) const {
        if(size != other.size) return true;
        return memcmp(data, other.data, size) != 0;
    }

    bool Str::operator==(const std::string_view other) const {
        if(size != (int)other.size()) return false;
        return memcmp(data, other.data(), size) == 0;
    }

    bool Str::operator!=(const std::string_view other) const {
        if(size != (int)other.size()) return true;
        return memcmp(data, other.data(), size) != 0;
    }

    bool Str::operator==(const char* p) const {
        return *this == std::string_view(p);
    }

    bool Str::operator!=(const char* p) const {
        return *this != std::string_view(p);
    }

    bool Str::operator<(const Str& other) const {
        return this->sv() < other.sv();
    }

    bool Str::operator<(const std::string_view other) const {
        return this->sv() < other;
    }

    bool Str::operator>(const Str& other) const {
        return this->sv() > other.sv();
    }

    bool Str::operator<=(const Str& other) const {
        return this->sv() <= other.sv();
    }

    bool Str::operator>=(const Str& other) const {
        return this->sv() >= other.sv();
    }

    Str::~Str(){
        if(!is_inlined()) pool64_dealloc(data);
        if(_cached_c_str != nullptr) free((void*)_cached_c_str);
    }

    Str Str::substr(int start, int len) const {
        Str ret(len, is_ascii);
        memcpy(ret.data, data + start, len);
        return ret;
    }

    Str Str::substr(int start) const {
        return substr(start, size - start);
    }

    char* Str::c_str_dup() const {
        char* p = (char*)malloc(size + 1);
        memcpy(p, data, size);
        p[size] = 0;
        return p;
    }

    const char* Str::c_str() const{
        if(_cached_c_str == nullptr){
            _cached_c_str = c_str_dup();
        }
        return _cached_c_str;
    }

    std::string_view Str::sv() const {
        return std::string_view(data, size);
    }

    std::string Str::str() const {
        return std::string(data, size);
    }

    Str Str::lstrip() const {
        std::string copy(data, size);
        copy.erase(copy.begin(), std::find_if(copy.begin(), copy.end(), [](char c) {
            // std::isspace(c) does not working on windows (Debug)
            return c != ' ' && c != '\t' && c != '\r' && c != '\n';
        }));
        return Str(copy);
    }

    Str Str::strip() const {
        std::string copy(data, size);
        copy.erase(copy.begin(), std::find_if(copy.begin(), copy.end(), [](char c) {
            return c != ' ' && c != '\t' && c != '\r' && c != '\n';
        }));
        copy.erase(std::find_if(copy.rbegin(), copy.rend(), [](char c) {
            return c != ' ' && c != '\t' && c != '\r' && c != '\n';
        }).base(), copy.end());
        return Str(copy);
    }

    Str Str::lower() const{
        std::string copy(data, size);
        std::transform(copy.begin(), copy.end(), copy.begin(), [](unsigned char c){
            if('A' <= c && c <= 'Z') return c + ('a' - 'A');
            return (int)c;
        });
        return Str(copy);
    }

    Str Str::upper() const{
        std::string copy(data, size);
        std::transform(copy.begin(), copy.end(), copy.begin(), [](unsigned char c){
            if('a' <= c && c <= 'z') return c - ('a' - 'A');
            return (int)c;
        });
        return Str(copy);
    }

    Str Str::escape(bool single_quote) const{
        SStream ss;
        escape_(ss, single_quote);
        return ss.str();
    }

    void Str::escape_(SStream& ss, bool single_quote) const {
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
                        ss << "\\x"; // << std::hex << std::setw(2) << std::setfill('0') << (int)c;
                        ss << "0123456789abcdef"[c >> 4];
                        ss << "0123456789abcdef"[c & 0xf];
                    } else {
                        ss << c;
                    }
            }
        }
        ss << (single_quote ? '\'' : '"');
    }

    int Str::index(const Str& sub, int start) const {
        auto p = std::search(data + start, data + size, sub.data, sub.data + sub.size);
        if(p == data + size) return -1;
        return p - data;
    }

    Str Str::replace(char old, char new_) const{
        Str copied = *this;
        for(int i=0; i<copied.size; i++){
            if(copied.data[i] == old) copied.data[i] = new_;
        }
        return copied;
    }

    Str Str::replace(const Str& old, const Str& new_, int count) const {
        SStream ss;
        int start = 0;
        while(true){
            int i = index(old, start);
            if(i == -1) break;
            ss << substr(start, i - start);
            ss << new_;
            start = i + old.size;
            if(count != -1 && --count == 0) break;
        }
        ss << substr(start, size - start);
        return ss.str();
    }


    int Str::_unicode_index_to_byte(int i) const{
        if(is_ascii) return i;
        int j = 0;
        while(i > 0){
            j += utf8len(data[j]);
            i--;
        }
        return j;
    }

    int Str::_byte_index_to_unicode(int n) const{
        if(is_ascii) return n;
        int cnt = 0;
        for(int i=0; i<n; i++){
            if((data[i] & 0xC0) != 0x80) cnt++;
        }
        return cnt;
    }

    Str Str::u8_getitem(int i) const{
        i = _unicode_index_to_byte(i);
        return substr(i, utf8len(data[i]));
    }

    Str Str::u8_slice(int start, int stop, int step) const{
        SStream ss;
        if(is_ascii){
            for(int i=start; step>0?i<stop:i>stop; i+=step) ss << data[i];
        }else{
            for(int i=start; step>0?i<stop:i>stop; i+=step) ss << u8_getitem(i);
        }
        return ss.str();
    }

    int Str::u8_length() const {
        return _byte_index_to_unicode(size);
    }

    std::vector<std::string_view> Str::split(const Str& sep) const{
        std::vector<std::string_view> result;
        std::string_view tmp;
        int start = 0;
        while(true){
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

    std::vector<std::string_view> Str::split(char sep) const{
        std::vector<std::string_view> result;
        int i = 0;
        for(int j = 0; j < size; j++){
            if(data[j] == sep){
                if(j > i) result.emplace_back(data+i, j-i);
                i = j + 1;
                continue;
            }
        }
        if(size > i) result.emplace_back(data+i, size-i);
        return result;
    }

    int Str::count(const Str& sub) const{
        if(sub.empty()) return size + 1;
        int cnt = 0;
        int start = 0;
        while(true){
            int i = index(sub, start);
            if(i == -1) break;
            cnt++;
            start = i + sub.size;
        }
        return cnt;
    }

    std::ostream& operator<<(std::ostream& os, const StrName& sn){
        return os << sn.sv();
    }

    std::map<std::string, uint16_t, std::less<>>& StrName::_interned(){
        static std::map<std::string, uint16_t, std::less<>> interned;
        return interned;
    }

    std::map<uint16_t, std::string>& StrName::_r_interned(){
        static std::map<uint16_t, std::string> r_interned;
        return r_interned;
    }

    uint32_t StrName::_pesudo_random_index = 0;

    StrName StrName::get(std::string_view s){
        auto it = _interned().find(s);
        if(it != _interned().end()) return StrName(it->second);
        // generate new index
        // https://github.com/python/cpython/blob/3.12/Objects/dictobject.c#L175
        uint16_t index = ((_pesudo_random_index*5) + 1) & 65535;
        if(index == 0) throw std::runtime_error("StrName index overflow");
        _interned()[std::string(s)] = index;
        if(is_valid(index)) throw std::runtime_error("StrName index conflict");
        _r_interned()[index] = std::string(s);
        _pesudo_random_index = index;
        return StrName(index);
    }

    Str StrName::escape() const {
        return Str(sv()).escape();
    }

    bool StrName::is_valid(int index) {
        return _r_interned().find(index) != _r_interned().end();
    }

    StrName::StrName(): index(0) {}
    StrName::StrName(uint16_t index): index(index) {}
    StrName::StrName(const char* s): index(get(s).index) {}
    StrName::StrName(const Str& s){
        index = get(s.sv()).index;
    }

    std::string_view StrName::sv() const {
        const std::string& str = _r_interned()[index];
        return std::string_view(str);
    }
} // namespace pkpy