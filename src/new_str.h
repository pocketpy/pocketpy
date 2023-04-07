#pragma once

#include "common.h"
#include "memory.h"
#include <string_view>

namespace pkpy{

struct String{
    char* data;
    int size;

    String(): data((char*)pool64.alloc(0)), size(0) {}
    String(int size): data((char*)pool64.alloc(size)), size(size) {}
    String(const char* str) {
        size = strlen(str);
        data = (char*)pool64.alloc(size);
        memcpy(data, str, size);
    }

    String(const String& other): data((char*)pool64.alloc(other.size)), size(other.size) {
        memcpy(data, other.data, size);
    }

    String(String&& other): data(other.data), size(other.size) {
        other.data = nullptr;
    }

    String& operator=(const String& other){
        if(data!=nullptr) pool64.dealloc(data);
        size = other.size;
        data = (char*)pool64.alloc(size);
        memcpy(data, other.data, size);
        return *this;
    }

    String& operator=(String&& other){
        if(data!=nullptr) pool64.dealloc(data);
        size = other.size;
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
        String ret(size + other.size);
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
        String ret(len);
        memcpy(ret.data, data + start, len);
        return ret;
    }

    String substr(int start) const {
        return substr(start, size - start);
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
};

struct UnicodeString: String{

};


}   // namespace pkpy