#include "pocketpy/common/str.h"
#include "pocketpy/common/vector.h"
#include "pocketpy/common/utils.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

int pkpy_utils__u8_header(unsigned char c, bool suppress) {
    if((c & 0b10000000) == 0) return 1;
    if((c & 0b11100000) == 0b11000000) return 2;
    if((c & 0b11110000) == 0b11100000) return 3;
    if((c & 0b11111000) == 0b11110000) return 4;
    if((c & 0b11111100) == 0b11111000) return 5;
    if((c & 0b11111110) == 0b11111100) return 6;
    if(!suppress) PK_FATAL_ERROR("invalid utf8 char\n")
    return 0;
}

void pkpy_Str__ctor(pkpy_Str *self, const char *data){
    pkpy_Str__ctor2(self, data, strlen(data));
}

void pkpy_Str__ctor2(pkpy_Str *self, const char *data, int size){
    self->size = size;
    self->is_ascii = true;
    self->is_sso = size < sizeof(self->_inlined);
    char* p;
    if(self->is_sso){
        p = self->_inlined;
    }else{
        self->_ptr = (char*)malloc(size + 1);
        p = self->_ptr;
    }
    memcpy(p, data, size);
    p[size] = '\0';
    // check is_ascii
    for(int i = 0; i < size; i++){
        if(!isascii(p[i])){
            self->is_ascii = false;
            break;
        }
    }
}

void pkpy_Str__dtor(pkpy_Str *self){
    if(!self->is_sso){
        free(self->_ptr);
        self->is_sso = true;
        self->size = 0;
    }
}

pkpy_Str pkpy_Str__copy(const pkpy_Str *self){
    pkpy_Str retval = *self;
    if(!self->is_sso){
        retval._ptr = (char*)malloc(self->size + 1);
        memcpy(retval._ptr, self->_ptr, self->size + 1);
        retval._ptr[retval.size] = '\0';
    }
    return retval;
}

pkpy_Str pkpy_Str__concat(const pkpy_Str *self, const pkpy_Str *other){
    pkpy_Str retval = {
        .size = self->size + other->size,
        .is_ascii = self->is_ascii && other->is_ascii,
        .is_sso = self->size + other->size < sizeof(retval._inlined),
    };
    char* p;
    if(retval.is_sso){
        p = retval._inlined;
    }else{
        retval._ptr = (char*)malloc(retval.size + 1);
        p = retval._ptr;
    }
    memcpy(p, pkpy_Str__data(self), self->size);
    memcpy(p + self->size, pkpy_Str__data(other), other->size);
    p[retval.size] = '\0';
    return retval;
}

pkpy_Str pkpy_Str__concat2(const pkpy_Str *self, const char *other, int size){
    pkpy_Str tmp;
    pkpy_Str__ctor2(&tmp, other, size);
    pkpy_Str retval = pkpy_Str__concat(self, &tmp);
    pkpy_Str__dtor(&tmp);
    return retval;
}

pkpy_Str pkpy_Str__slice(const pkpy_Str *self, int start){
    return pkpy_Str__slice2(self, start, self->size);
}

pkpy_Str pkpy_Str__slice2(const pkpy_Str *self, int start, int stop){
    pkpy_Str retval;
    if(stop < start) stop = start;
    pkpy_Str__ctor2(&retval, pkpy_Str__data(self) + start, stop - start);
    return retval;
}

pkpy_Str pkpy_Str__lower(const pkpy_Str *self){
    pkpy_Str retval = pkpy_Str__copy(self);
    char* p = (char*)pkpy_Str__data(&retval);
    for(int i = 0; i < retval.size; i++){
        if('A' <= p[i] && p[i] <= 'Z') p[i] += 32;
    }
    return retval;
}

pkpy_Str pkpy_Str__upper(const pkpy_Str *self){
    pkpy_Str retval = pkpy_Str__copy(self);
    char* p = (char*)pkpy_Str__data(&retval);
    for(int i = 0; i < retval.size; i++){
        if('a' <= p[i] && p[i] <= 'z') p[i] -= 32;
    }
    return retval;
}

pkpy_Str pkpy_Str__escape(const pkpy_Str* self, char quote){
    assert(quote == '"' || quote == '\'');
    c11_vector buffer;
    c11_vector__ctor(&buffer, sizeof(char));
    c11_vector__reserve(&buffer, self->size);
    c11_vector__push(char, &buffer, quote);
    const char* data = pkpy_Str__data(self);
    for(int i = 0; i < self->size; i++) {
        char c = data[i];
        switch(c) {
            case '"': case '\'':
                if(c == quote) c11_vector__push(char, &buffer, '\\');
                c11_vector__push(char, &buffer, c);
                break;
            case '\\':
                c11_vector__push(char, &buffer, '\\');
                c11_vector__push(char, &buffer, '\\');
                break;
            case '\n':
                c11_vector__push(char, &buffer, '\\');
                c11_vector__push(char, &buffer, 'n');
                break;
            case '\r':
                c11_vector__push(char, &buffer, '\\');
                c11_vector__push(char, &buffer, 'r');
                break;
            case '\t':
                c11_vector__push(char, &buffer, '\\');
                c11_vector__push(char, &buffer, 't');
                break;
            case '\b':
                c11_vector__push(char, &buffer, '\\');
                c11_vector__push(char, &buffer, 'b');
                break;
            default:
                if('\x00' <= c && c <= '\x1f') {
                    c11_vector__push(char, &buffer, '\\');
                    c11_vector__push(char, &buffer, 'x');
                    c11_vector__push(char, &buffer, PK_HEX_TABLE[c >> 4]);
                    c11_vector__push(char, &buffer, PK_HEX_TABLE[c & 0xf]);
                } else {
                    c11_vector__push(char, &buffer, c);
                }
        }
    }
    c11_vector__push(char, &buffer, quote);
    c11_vector__push(char, &buffer, '\0');
    pkpy_Str retval = {
        .size = buffer.count - 1,
        .is_ascii = self->is_ascii,
        .is_sso = false,
        ._ptr = (char*)buffer.data,
    };
    return retval;
}

pkpy_Str pkpy_Str__strip(const pkpy_Str *self, bool left, bool right){
    const char* data = pkpy_Str__data(self);
    if(self->is_ascii) {
        int L = 0;
        int R = self->size;
        if(left) {
            while(L < R && (data[L] == ' ' || data[L] == '\t' || data[L] == '\n' || data[L] == '\r'))
                L++;
        }
        if(right) {
            while(L < R && (data[R - 1] == ' ' || data[R - 1] == '\t' || data[R - 1] == '\n' || data[R - 1] == '\r'))
                R--;
        }
        return pkpy_Str__slice2(self, L, R);
    } else {
        pkpy_Str tmp;
        pkpy_Str__ctor(&tmp, " \t\n\r");
        pkpy_Str retval = pkpy_Str__strip2(self, left, right, &tmp);
        pkpy_Str__dtor(&tmp);
        return retval;
    }
}

pkpy_Str pkpy_Str__strip2(const pkpy_Str *self, bool left, bool right, const pkpy_Str *chars){
    int L = 0;
    int R = pkpy_Str__u8_length(self);
    if(left) {
        while(L < R){
            pkpy_Str tmp = pkpy_Str__u8_getitem(self, L);
            bool found = pkpy_Str__index(chars, &tmp, 0) != -1;
            pkpy_Str__dtor(&tmp);
            if(!found) break;
            L++;
        }
    }
    if(right) {
        while(L < R){
            pkpy_Str tmp = pkpy_Str__u8_getitem(self, R - 1);
            bool found = pkpy_Str__index(chars, &tmp, 0) != -1;
            pkpy_Str__dtor(&tmp);
            if(!found) break;
            R--;
        }
    }
    return pkpy_Str__u8_slice(self, L, R, 1);
}

pkpy_Str pkpy_Str__replace(const pkpy_Str *self, char old, char new_){
    pkpy_Str retval = pkpy_Str__copy(self);
    char* p = (char*)pkpy_Str__data(&retval);
    for(int i = 0; i < retval.size; i++){
        if(p[i] == old) p[i] = new_;
    }
    return retval;
}

pkpy_Str pkpy_Str__replace2(const pkpy_Str *self, const pkpy_Str *old, const pkpy_Str *new_){
    c11_vector buffer;
    c11_vector__ctor(&buffer, sizeof(char));
    int start = 0;
    while(true) {
        int i = pkpy_Str__index(self, old, start);
        if(i == -1) break;
        pkpy_Str tmp = pkpy_Str__slice2(self, start, i);
        c11_vector__extend(char, &buffer, pkpy_Str__data(&tmp), tmp.size);
        pkpy_Str__dtor(&tmp);
        c11_vector__extend(char, &buffer, pkpy_Str__data(new_), new_->size);
        start = i + old->size;
    }
    pkpy_Str tmp = pkpy_Str__slice2(self, start, self->size);
    c11_vector__extend(char, &buffer, pkpy_Str__data(&tmp), tmp.size);
    pkpy_Str__dtor(&tmp);
    c11_vector__push(char, &buffer, '\0');
    pkpy_Str retval = {
        .size = buffer.count - 1,
        .is_ascii = self->is_ascii && old->is_ascii && new_->is_ascii,
        .is_sso = false,
        ._ptr = (char*)buffer.data,
    };
    return retval;
}

int pkpy_Str__cmp(const pkpy_Str *self, const pkpy_Str *other){
    return pkpy_Str__cmp2(self, pkpy_Str__data(other), other->size);
}

int pkpy_Str__cmp2(const pkpy_Str *self, const char *other, int size){
    int res = strncmp(pkpy_Str__data(self), other, PK_MIN(self->size, size));
    if(res != 0) return res;
    return self->size - size;
}

pkpy_Str pkpy_Str__u8_getitem(const pkpy_Str *self, int i){
    i = pkpy_Str__unicode_index_to_byte(self, i);
    int size = pkpy_utils__u8_header(pkpy_Str__data(self)[i], false);
    return pkpy_Str__slice2(self, i, i + size);
}

pkpy_Str pkpy_Str__u8_slice(const pkpy_Str *self, int start, int stop, int step){
    c11_vector buffer;
    c11_vector__ctor(&buffer, sizeof(char));
    assert(step != 0);
    if(self->is_ascii){
        const char* p = pkpy_Str__data(self);
        for (int i=start; step>0 ? i<stop : i>stop; i+=step) {
            c11_vector__push(char, &buffer, p[i]);
        }
    }else{
        for (int i=start; step>0 ? i<stop : i>stop; i+=step) {
            pkpy_Str unicode = pkpy_Str__u8_getitem(self, i);
            const char* p = pkpy_Str__data(&unicode);
            for(int j = 0; j < unicode.size; j++){
                c11_vector__push(char, &buffer, p[j]);
            }
            pkpy_Str__dtor(&unicode);
        }
    }
    c11_vector__push(char, &buffer, '\0');
    pkpy_Str retval = {
        .size = buffer.count - 1,
        .is_ascii = self->is_ascii,
        .is_sso = false,
        ._ptr = (char*)buffer.data,
    };
    return retval;
}

int pkpy_Str__u8_length(const pkpy_Str *self){
    return pkpy_Str__byte_index_to_unicode(self, self->size);
}

int pkpy_Str__unicode_index_to_byte(const pkpy_Str* self, int i) {
    if(self->is_ascii) return i;
    const char* p = pkpy_Str__data(self);
    int j = 0;
    while(i > 0) {
        j += pkpy_utils__u8_header(p[j], false);
        i--;
    }
    return j;
}

int pkpy_Str__byte_index_to_unicode(const pkpy_Str* self, int n) {
    if(self->is_ascii) return n;
    const char* p = pkpy_Str__data(self);
    int cnt = 0;
    for(int i = 0; i < n; i++) {
        if((p[i] & 0xC0) != 0x80) cnt++;
    }
    return cnt;
}

int pkpy_Str__index(const pkpy_Str *self, const pkpy_Str *sub, int start){
    if(sub->size == 0) return start;
    int max_end = self->size - sub->size;
    const char* self_data = pkpy_Str__data(self);
    const char* sub_data = pkpy_Str__data(sub);
    for(int i=start; i<=max_end; i++){
        int res = memcmp(self_data + i, sub_data, sub->size);
        if(res == 0) return i;
    }
    return -1;
}

int pkpy_Str__count(const pkpy_Str *self, const pkpy_Str *sub){
    if(sub->size == 0) return self->size + 1;
    int cnt = 0;
    int start = 0;
    while(true) {
        int i = pkpy_Str__index(self, sub, start);
        if(i == -1) break;
        cnt++;
        start = i + sub->size;
    }
    return cnt;
}

