#pragma once

#include "common.h"
#include "memory.h"

namespace pkpy{

inline int utf8len(unsigned char c){
    if((c & 0b10000000) == 0) return 1;
    if((c & 0b11100000) == 0b11000000) return 2;
    if((c & 0b11110000) == 0b11100000) return 3;
    if((c & 0b11111000) == 0b11110000) return 4;
    if((c & 0b11111100) == 0b11111000) return 5;
    if((c & 0b11111110) == 0b11111100) return 6;
    return 0;
}

struct String{
    int size;
    bool is_ascii;
    char* data;

    String(): size(0), is_ascii(true), data((char*)pool64.alloc(0)) {}

    String(int size, bool is_ascii): size(size), is_ascii(is_ascii) {
        data = (char*)pool64.alloc(size);
    }

    String(const char* str): size(strlen(str)), is_ascii(true) {
        data = (char*)pool64.alloc(size);
        for(int i=0; i<size; i++){
            data[i] = str[i];
            if(!isascii(str[i])) is_ascii = false;
        }
    }

    String(const String& other): size(other.size), is_ascii(other.is_ascii) {
        data = (char*)pool64.alloc(size);
        memcpy(data, other.data, size);
    }

    String(String&& other): size(other.size), is_ascii(other.is_ascii), data(other.data) {
        other.data = nullptr;
        other.size = 0;
    }

    String& operator=(const String& other){
        if(data!=nullptr) pool64.dealloc(data);
        size = other.size;
        is_ascii = other.is_ascii;
        data = (char*)pool64.alloc(size);
        memcpy(data, other.data, size);
        return *this;
    }

    String& operator=(String&& other){
        if(data!=nullptr) pool64.dealloc(data);
        size = other.size;
        is_ascii = other.is_ascii;
        data = other.data;
        other.data = nullptr;
        return *this;
    }

    ~String(){
        if(data!=nullptr) pool64.dealloc(data);
    }

    char operator[](int idx) const {
        return data[idx];
    }

    int length() const {
        return size;
    }

    String operator+(const String& other) const {
        String ret(size + other.size, is_ascii && other.is_ascii);
        memcpy(ret.data, data, size);
        memcpy(ret.data + size, other.data, other.size);
        return ret;
    }

    friend std::ostream& operator<<(std::ostream& os, const String& str){
        os.write(str.data, str.size);
        return os;
    }

    bool operator==(const String& other) const {
        if(size != other.size) return false;
        return memcmp(data, other.data, size) == 0;
    }

    bool operator!=(const String& other) const {
        if(size != other.size) return true;
        return memcmp(data, other.data, size) != 0;
    }

    bool operator<(const String& other) const {
        int ret = strncmp(data, other.data, std::min(size, other.size));
        if(ret != 0) return ret < 0;
        return size < other.size;
    }

    bool operator>(const String& other) const {
        int ret = strncmp(data, other.data, std::min(size, other.size));
        if(ret != 0) return ret > 0;
        return size > other.size;
    }

    bool operator<=(const String& other) const {
        int ret = strncmp(data, other.data, std::min(size, other.size));
        if(ret != 0) return ret < 0;
        return size <= other.size;
    }

    bool operator>=(const String& other) const {
        int ret = strncmp(data, other.data, std::min(size, other.size));
        if(ret != 0) return ret > 0;
        return size >= other.size;
    }

    String substr(int start, int len) const {
        String ret(len, is_ascii);
        memcpy(ret.data, data + start, len);
        return ret;
    }

    char* dup_c_str() const {
        char* p = (char*)malloc(size + 1);
        memcpy(p, data, size);
        p[size] = 0;
        return p;
    }

    std::string_view view() const {
        return std::string_view(data, size);
    }

    std::string str() const {
        return std::string(data, size);
    }

    String lstrip() const {
        std::string copy = str();
        copy.erase(copy.begin(), std::find_if(copy.begin(), copy.end(), [](char c) {
            // std::isspace(c) does not working on windows (Debug)
            return c != ' ' && c != '\t' && c != '\r' && c != '\n';
        }));
        return String(copy.c_str());
    }

    /*************unicode*************/

    int _u8_index(int i) const{
        if(is_ascii) return i;
        int j = 0;
        while(i > 0){
            j += utf8len(data[j]);
            i--;
        }
        return j;
    }

    String u8_getitem(int i) const {
        i = _u8_index(i);
        return substr(i, utf8len(data[i]));
    }

    String u8_slice(int start, int end) const{
        start = _u8_index(start);
        end = _u8_index(end);
        return substr(start, end - start);
    }
};

}   // namespace pkpy