#include "pocketpy/common/sstream.h"
#include "pocketpy/common/utils.h"

#include <stdio.h>
#include <assert.h>

void pkpy_SStream__ctor(pkpy_SStream* self) {
    c11_vector__ctor(&self->data, sizeof(char));
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

void pkpy_SStream__write_int64(pkpy_SStream* self, int64_t i) {
    char buf[23]; // sign + 21 digits + null terminator
    snprintf(buf, sizeof(buf), "%lld", i);
    pkpy_SStream__write_cstr(self, buf);
}

void pkpy_SStream__write_Str(pkpy_SStream* self, const pkpy_Str* str) {
    pkpy_SStream__write_cstr(self, pkpy_Str__data(str));
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

void pkpy_SStream__write_any(pkpy_SStream* self, const char* fmt, const pkpy_AnyStr* args, int n){
    int i = 0;
    while(*fmt){
        if(*fmt == '{' && fmt[1] == '}'){
            assert(i < n);
            switch(args[i].type){
                case 1: pkpy_SStream__write_int(self, args[i]._int); break;
                case 2: pkpy_SStream__write_int64(self, args[i]._int64); break;
                case 3: assert(0); break;
                case 4: assert(0); break;
                case 5: pkpy_SStream__write_char(self, args[i]._char); break;
                case 6: assert(0); break;
                case 7: pkpy_SStream__write_Str(self, args[i]._str); break;
                case 8: pkpy_SStream__write_sv(self, args[i]._sv); break;
                case 9: pkpy_SStream__write_cstr(self, args[i]._cstr); break;
                case 10: assert(0); break;
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
