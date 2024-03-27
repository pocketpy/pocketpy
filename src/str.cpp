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

#define PK_STR_ALLOCATE()                                   \
        if(this->size < (int)sizeof(this->_inlined)){       \
            this->data = this->_inlined;                    \
        }else{                                              \
            this->data = (char*)pool64_alloc(this->size+1); \
        }

#define PK_STR_COPY_INIT(__s)  \
        for(int i=0; i<this->size; i++){                    \
            this->data[i] = __s[i];                         \
            if(!isascii(__s[i])) is_ascii = false;          \
        }                                                   \
        this->data[this->size] = '\0';

    Str::Str(): size(0), is_ascii(true), data(_inlined) {
        _inlined[0] = '\0';
    }

    Str::Str(int size, bool is_ascii): size(size), is_ascii(is_ascii) {
        PK_STR_ALLOCATE()
    }

    Str::Str(const std::string& s): size(s.size()), is_ascii(true) {
        PK_STR_ALLOCATE()
        PK_STR_COPY_INIT(s)
    }

    Str::Str(std::string_view s): size(s.size()), is_ascii(true) {
        PK_STR_ALLOCATE()
        PK_STR_COPY_INIT(s)
    }

    Str::Str(const char* s): size(strlen(s)), is_ascii(true) {
        PK_STR_ALLOCATE()
        PK_STR_COPY_INIT(s)
    }

    Str::Str(const char* s, int len): size(len), is_ascii(true) {
        PK_STR_ALLOCATE()
        PK_STR_COPY_INIT(s)
    }

    Str::Str(std::pair<char *, int> detached): size(detached.second), is_ascii(true) {
        this->data = detached.first;
        for(int i=0; i<size; i++){
            if(!isascii(data[i])){ is_ascii = false; break; }
        }
        PK_ASSERT(data[size] == '\0');
    }

    Str::Str(const Str& other): size(other.size), is_ascii(other.is_ascii) {
        PK_STR_ALLOCATE()
        memcpy(data, other.data, size);
        data[size] = '\0';
    }

    Str::Str(Str&& other): size(other.size), is_ascii(other.is_ascii) {
        if(other.is_inlined()){
            data = _inlined;
            for(int i=0; i<size; i++) _inlined[i] = other._inlined[i];
            data[size] = '\0';
        }else{
            data = other.data;
            // zero out `other`
            other.data = other._inlined;
            other.data[0] = '\0';
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

    Str& Str::operator=(const Str& other){
        if(!is_inlined()) pool64_dealloc(data);
        size = other.size;
        is_ascii = other.is_ascii;
        PK_STR_ALLOCATE()
        memcpy(data, other.data, size);
        data[size] = '\0';
        return *this;
    }

    Str Str::operator+(const Str& other) const {
        Str ret(size + other.size, is_ascii && other.is_ascii);
        memcpy(ret.data, data, size);
        memcpy(ret.data + size, other.data, other.size);
        ret.data[ret.size] = '\0';
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
    }

    Str Str::substr(int start, int len) const {
        Str ret(len, is_ascii);
        memcpy(ret.data, data + start, len);
        ret.data[len] = '\0';
        return ret;
    }

    Str Str::substr(int start) const {
        return substr(start, size - start);
    }

    Str Str::strip(bool left, bool right, const Str& chars) const {
        int L = 0;
        int R = u8_length();
        if(left){
            while(L < R && chars.index(u8_getitem(L)) != -1) L++;
        }
        if(right){
            while(L < R && chars.index(u8_getitem(R-1)) != -1) R--;
        }
        return u8_slice(L, R, 1);
    }

    Str Str::strip(bool left, bool right) const {
        if(is_ascii){
            int L = 0;
            int R = size;
            if(left){
                while(L < R && (data[L] == ' ' || data[L] == '\t' || data[L] == '\n' || data[L] == '\r')) L++;
            }
            if(right){
                while(L < R && (data[R-1] == ' ' || data[R-1] == '\t' || data[R-1] == '\n' || data[R-1] == '\r')) R--;
            }
            return substr(L, R - L);
        }else{
            return strip(left, right, " \t\n\r");
        }
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
                case '\b': ss << "\\b"; break;
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
            PK_SLICE_LOOP(i, start, stop, step) ss << data[i];
        }else{
            PK_SLICE_LOOP(i, start, stop, step) ss << u8_getitem(i);
        }
        return ss.str();
    }

    int Str::u8_length() const {
        return _byte_index_to_unicode(size);
    }

    pod_vector<std::string_view> Str::split(const Str& sep) const{
        pod_vector<std::string_view> result;
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

    pod_vector<std::string_view> Str::split(char sep) const{
        pod_vector<std::string_view> result;
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

    bool StrName::is_valid(int index) {
        return _r_interned().find(index) != _r_interned().end();
    }

    Str SStream::str(){
        // after this call, the buffer is no longer valid
        buffer.reserve(buffer.size() + 1);  // allocate one more byte for '\0'
        buffer[buffer.size()] = '\0';       // set '\0'
        return Str(buffer.detach());
    }

    SStream& SStream::operator<<(const Str& s){
        buffer.extend(s.begin(), s.end());
        return *this;
    }

    SStream& SStream::operator<<(const char* s){
        buffer.extend(s, s + strlen(s));
        return *this;
    }

    SStream& SStream::operator<<(const std::string& s){
        buffer.extend(s.data(), s.data() + s.size());
        return *this;
    }

    SStream& SStream::operator<<(std::string_view s){
        buffer.extend(s.data(), s.data() + s.size());
        return *this;
    }

    SStream& SStream::operator<<(char c){
        buffer.push_back(c);
        return *this;
    }

    SStream& SStream::operator<<(StrName sn){
        return *this << sn.sv();
    }

    SStream& SStream::operator<<(size_t val){
        // size_t could be out of range of `i64`, use `std::to_string` instead
        return (*this) << std::to_string(val);
    }

    SStream& SStream::operator<<(int val){
        return (*this) << static_cast<i64>(val);
    }

    SStream& SStream::operator<<(i64 val){
        // str(-2**64).__len__() == 21
        buffer.reserve(buffer.size() + 24);
        if(val == 0){
            buffer.push_back('0');
            return *this;
        }
        if(val < 0){
            buffer.push_back('-');
            val = -val;
        }
        char* begin = buffer.end();
        while(val){
            buffer.push_back('0' + val % 10);
            val /= 10;
        }
        std::reverse(begin, buffer.end());
        return *this;
    }

    SStream& SStream::operator<<(f64 val){
        if(std::isinf(val)){
            return (*this) << (val > 0 ? "inf" : "-inf");
        }
        if(std::isnan(val)){
            return (*this) << "nan";
        }
        char b[32];
        if(_precision == -1){
            int prec = std::numeric_limits<f64>::max_digits10-1;
            snprintf(b, sizeof(b), "%.*g", prec, val);
        }else{
            int prec = _precision;
            snprintf(b, sizeof(b), "%.*f", prec, val);
        }
        (*this) << b;
        if(std::all_of(b+1, b+strlen(b), isdigit)) (*this) << ".0";
        return *this;
    }

    void SStream::write_hex(unsigned char c, bool non_zero){
        unsigned char high = c >> 4;
        unsigned char low = c & 0xf;
        if(non_zero){
            if(high) (*this) << "0123456789abcdef"[high];
            if(high || low) (*this) << "0123456789abcdef"[low];
        }else{
            (*this) << "0123456789abcdef"[high];
            (*this) << "0123456789abcdef"[low];
        }
    }

    void SStream::write_hex(void* p){
        if(p == nullptr){
            (*this) << "0x0";
            return;
        }
        (*this) << "0x";
        uintptr_t p_t = reinterpret_cast<uintptr_t>(p);
        bool non_zero = true;
        for(int i=sizeof(void*)-1; i>=0; i--){
            unsigned char cpnt = (p_t >> (i * 8)) & 0xff;
            write_hex(cpnt, non_zero);
            if(cpnt != 0) non_zero = false;
        }
    }

    void SStream::write_hex(i64 val){
        if(val == 0){
            (*this) << "0x0";
            return;
        }
        if(val < 0){
            (*this) << "-";
            val = -val;
        }
        (*this) << "0x";
        bool non_zero = true;
        for(int i=56; i>=0; i-=8){
            unsigned char cpnt = (val >> i) & 0xff;
            write_hex(cpnt, non_zero);
            if(cpnt != 0) non_zero = false;
        }
    }

#undef PK_STR_ALLOCATE
#undef PK_STR_COPY_INIT

} // namespace pkpy