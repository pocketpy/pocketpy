#include "pocketpy/common/sstream.h"
#include "pocketpy/common/utils.h"

#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <math.h>

void pkpy_SStream__ctor(pkpy_SStream* self) {
    c11_vector__ctor(&self->data, sizeof(char));
}

void pkpy_SStream__ctor2(pkpy_SStream* self, int capacity) {
    c11_vector__ctor(&self->data, sizeof(char));
    c11_vector__reserve(&self->data, capacity);
}

void pkpy_SStream__dtor(pkpy_SStream* self) {
    c11_vector__dtor(&self->data);
}

void pkpy_SStream__write_char(pkpy_SStream* self, char c) {
    c11_vector__push(char, &self->data, c);
}

void pkpy_SStream__write_int(pkpy_SStream* self, int i) {
    char buf[12]; // sign + 10 digits + null terminator
    snprintf(buf, sizeof(buf), "%d", i);
    pkpy_SStream__write_cstr(self, buf);
}

void pkpy_SStream__write_i64(pkpy_SStream* self, int64_t val) {
    // sign + 21 digits + null terminator
    // str(-2**64).__len__() == 21
    c11_vector__reserve(&self->data, self->data.count + 23);
    if(val == 0){
        pkpy_SStream__write_char(self, '0');
        return;
    }
    if(val < 0){
        pkpy_SStream__write_char(self, '-');
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

void pkpy_SStream__write_float(pkpy_SStream* self, float val, int precision){
    pkpy_SStream__write_double(self, val, precision);
}

void pkpy_SStream__write_double(pkpy_SStream* self, double val, int precision){
    if(isinf(val)) {
        pkpy_SStream__write_cstr(self, val > 0 ? "inf" : "-inf");
        return;
    }
    if(isnan(val)) {
        pkpy_SStream__write_cstr(self, "nan");
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
    pkpy_SStream__write_cstr(self, b);
    bool all_is_digit = true;
    for(int i = 1; i < size; i++){
        if(!isdigit(b[i])){ all_is_digit = false; break; }
    }
    if(all_is_digit) pkpy_SStream__write_cstr(self, ".0");
}

void pkpy_SStream__write_Str(pkpy_SStream* self, const pkpy_Str* str) {
    pkpy_SStream__write_cstrn(self, pkpy_Str__data(str), str->size);
}

void pkpy_SStream__write_sv(pkpy_SStream* self, c11_string sv) {
    pkpy_SStream__write_cstrn(self, sv.data, sv.size);
}

void pkpy_SStream__write_cstr(pkpy_SStream* self, const char* str) {
    pkpy_SStream__write_cstrn(self, str, strlen(str));
}

void pkpy_SStream__write_cstrn(pkpy_SStream* self, const char* str, int n) {
    c11_vector__extend(char, &self->data, str, n);
}

void pkpy_SStream__write_hex(pkpy_SStream* self, unsigned char c, bool non_zero) {
    unsigned char high = c >> 4;
    unsigned char low = c & 0xf;
    if(non_zero) {
        if(high) pkpy_SStream__write_char(self, PK_HEX_TABLE[high]);
        if(high || low) pkpy_SStream__write_char(self, PK_HEX_TABLE[low]);
    } else {
        pkpy_SStream__write_char(self, PK_HEX_TABLE[high]);
        pkpy_SStream__write_char(self, PK_HEX_TABLE[low]);
    }
}

void pkpy_SStream__write_ptr(pkpy_SStream* self, void* p) {
    if(p == NULL) {
        pkpy_SStream__write_cstr(self, "0x0");
        return;
    }
    pkpy_SStream__write_cstr(self, "0x");
    uintptr_t p_t = (uintptr_t)(p);
    bool non_zero = true;
    for(int i = sizeof(void*) - 1; i >= 0; i--) {
        unsigned char cpnt = (p_t >> (i * 8)) & 0xff;
        pkpy_SStream__write_hex(self, cpnt, non_zero);
        if(cpnt != 0) non_zero = false;
    }
}

void pkpy_SStream__write_any(pkpy_SStream* self, const char* fmt, const pkpy_AnyStr* args, int n){
    int i = 0;
    while(*fmt){
        if(*fmt == '{' && fmt[1] == '}'){
            assert(i < n);
            switch(args[i].type){
                case 1: pkpy_SStream__write_int(self, args[i]._int); break;
                case 2: pkpy_SStream__write_i64(self, args[i]._i64); break;
                case 3: pkpy_SStream__write_float(self, args[i]._float, -1); break;
                case 4: pkpy_SStream__write_double(self, args[i]._double, -1); break;
                case 5: pkpy_SStream__write_char(self, args[i]._char); break;
                case 6: pkpy_SStream__write_Str(self, args[i]._str); break;
                case 7: pkpy_SStream__write_sv(self, args[i]._sv); break;
                case 8: pkpy_SStream__write_cstr(self, args[i]._cstr); break;
                case 9: assert(0); break;
                default: assert(0); break;
            }
            fmt += 2;
            i++;
        }else{
            pkpy_SStream__write_char(self, *fmt);
            fmt++;
        }
    }
}

pkpy_Str pkpy_SStream__submit(pkpy_SStream* self) {
    c11_vector__push(char, &self->data, '\0');
    pkpy_Str retval = {
        .size = self->data.count - 1,
        .is_ascii = false,  // need to check
        .is_sso = false,
        ._ptr = (char*)self->data.data
    };
    return retval;
}
