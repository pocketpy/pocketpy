#include "pocketpy/common/sstream.h"
#include "pocketpy/common/config.h"
#include "pocketpy/common/utils.h"

#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <math.h>

void pk_SStream__ctor(pk_SStream* self) {
    c11_vector__ctor(&self->data, sizeof(char));
}

void pk_SStream__ctor2(pk_SStream* self, int capacity) {
    c11_vector__ctor(&self->data, sizeof(char));
    c11_vector__reserve(&self->data, capacity);
}

void pk_SStream__dtor(pk_SStream* self) {
    c11_vector__dtor(&self->data);
}

void pk_SStream__write_char(pk_SStream* self, char c) {
    c11_vector__push(char, &self->data, c);
}

void pk_SStream__write_int(pk_SStream* self, int i) {
    char buf[12]; // sign + 10 digits + null terminator
    snprintf(buf, sizeof(buf), "%d", i);
    pk_SStream__write_cstr(self, buf);
}

void pk_SStream__write_i64(pk_SStream* self, int64_t val) {
    // sign + 21 digits + null terminator
    // str(-2**64).__len__() == 21
    c11_vector__reserve(&self->data, self->data.count + 23);
    if(val == 0){
        pk_SStream__write_char(self, '0');
        return;
    }
    if(val < 0){
        pk_SStream__write_char(self, '-');
        val = -val;
    }
    int start = self->data.count;
    while(val){
        c11_vector__push(char, &self->data, '0' + val % 10);
        val /= 10;
    }
    int end = self->data.count - 1;
    c11_vector__reverse(char, &self->data, start, end);
}

void pk_SStream__write_float(pk_SStream* self, float val, int precision){
    pk_SStream__write_double(self, val, precision);
}

void pk_SStream__write_double(pk_SStream* self, double val, int precision){
    if(isinf(val)) {
        pk_SStream__write_cstr(self, val > 0 ? "inf" : "-inf");
        return;
    }
    if(isnan(val)) {
        pk_SStream__write_cstr(self, "nan");
        return;
    }
    char b[32];
    int size;
    if(precision < 0) {
        int prec = 17 - 1;  // std::numeric_limits<double>::max_digits10 == 17
        size = snprintf(b, sizeof(b), "%.*g", prec, val);
    } else {
        int prec = precision;
        size = snprintf(b, sizeof(b), "%.*f", prec, val);
    }
    pk_SStream__write_cstr(self, b);
    bool all_is_digit = true;
    for(int i = 1; i < size; i++){
        if(!isdigit(b[i])){ all_is_digit = false; break; }
    }
    if(all_is_digit) pk_SStream__write_cstr(self, ".0");
}

void pk_SStream__write_Str(pk_SStream* self, const pkpy_Str* str) {
    pk_SStream__write_cstrn(self, pkpy_Str__data(str), str->size);
}

void pk_SStream__write_sv(pk_SStream* self, c11_string sv) {
    pk_SStream__write_cstrn(self, sv.data, sv.size);
}

void pk_SStream__write_cstr(pk_SStream* self, const char* str) {
    pk_SStream__write_cstrn(self, str, strlen(str));
}

void pk_SStream__write_cstrn(pk_SStream* self, const char* str, int n) {
    c11_vector__extend(char, &self->data, str, n);
}

void pk_SStream__write_hex(pk_SStream* self, unsigned char c, bool non_zero) {
    unsigned char high = c >> 4;
    unsigned char low = c & 0xf;
    if(non_zero) {
        if(high) pk_SStream__write_char(self, PK_HEX_TABLE[high]);
        if(high || low) pk_SStream__write_char(self, PK_HEX_TABLE[low]);
    } else {
        pk_SStream__write_char(self, PK_HEX_TABLE[high]);
        pk_SStream__write_char(self, PK_HEX_TABLE[low]);
    }
}

void pk_SStream__write_ptr(pk_SStream* self, void* p) {
    if(p == NULL) {
        pk_SStream__write_cstr(self, "0x0");
        return;
    }
    pk_SStream__write_cstr(self, "0x");
    uintptr_t p_t = (uintptr_t)(p);
    bool non_zero = true;
    for(int i = sizeof(void*) - 1; i >= 0; i--) {
        unsigned char cpnt = (p_t >> (i * 8)) & 0xff;
        pk_SStream__write_hex(self, cpnt, non_zero);
        if(cpnt != 0) non_zero = false;
    }
}

void pk_SStream__write_any(pk_SStream* self, const char* fmt, const pk_AnyStr* args, int n){
    int i = 0;
    while(*fmt){
        if(*fmt == '{' && fmt[1] == '}'){
            assert(i < n);
            switch(args[i].type){
                case 1: pk_SStream__write_int(self, args[i]._int); break;
                case 2: pk_SStream__write_i64(self, args[i]._i64); break;
                case 3: pk_SStream__write_float(self, args[i]._float, -1); break;
                case 4: pk_SStream__write_double(self, args[i]._double, -1); break;
                case 5: pk_SStream__write_char(self, args[i]._char); break;
                case 6: pk_SStream__write_Str(self, args[i]._str); break;
                case 7: pk_SStream__write_sv(self, args[i]._sv); break;
                case 8: pk_SStream__write_cstr(self, args[i]._cstr); break;
                case 9: pk_SStream__write_ptr(self, args[i]._ptr); break;
                default: assert(0); break;
            }
            fmt += 2;
            i++;
        }else{
            pk_SStream__write_char(self, *fmt);
            fmt++;
        }
    }
}

pkpy_Str pk_SStream__submit(pk_SStream* self) {
    c11_vector__push(char, &self->data, '\0');
    pkpy_Str retval = {
        .size = self->data.count - 1,
        .is_ascii = c11__isascii((char*)self->data.data, self->data.count),
        .is_sso = false,
        ._ptr = (char*)self->data.data
    };
    return retval;
}

const char* pk_format_any(const char* fmt, const pk_AnyStr* args, int n){
    PK_THREAD_LOCAL pk_SStream ss;
    if(ss.data.elem_size == 0){
        pk_SStream__ctor2(&ss, 128);
    }else{
        c11_vector__clear(&ss.data);
    }
    pk_SStream__write_any(&ss, fmt, args, n);
    pk_SStream__write_char(&ss, '\0');
    return (const char*)ss.data.data;
}
