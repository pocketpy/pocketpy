#include "pocketpy/common/sstream.h"
#include <stdio.h>

void pkpy_SStream__ctor(pkpy_SStream* self) {
    c11_vector__ctor(&self->data, sizeof(char));
}

void pkpy_SStream__dtor(pkpy_SStream* self) {
    c11_vector__dtor(&self->data);
}

void pkpy_SStream__append_cstr(pkpy_SStream* self, const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        c11_vector__push_back(char, &self->data, str[i]);
    }
}

void pkpy_SStream__append_cstrn(pkpy_SStream* self, const char* str, int n) {
    for (int i = 0; i < n; i++) {
        c11_vector__push_back(char, &self->data, str[i]);
    }
}

void pkpy_SStream__append_Str(pkpy_SStream* self, const pkpy_Str* str) {
    pkpy_SStream__append_cstr(self, pkpy_Str__data(str));
}

void pkpy_SStream__append_char(pkpy_SStream* self, char c) {
    c11_vector__push_back(char, &self->data, c);
}

void pkpy_SStream__append_int(pkpy_SStream* self, int i) {
    char str[12]; // sign + 10 digits + null terminator
    sprintf(str, "%d", i);
    pkpy_SStream__append_cstr(self, str);
}

void pkpy_SStream__append_int64(pkpy_SStream* self, int64_t i) {
    char str[23]; // sign + 21 digits + null terminator
    sprintf(str, "%lld", i);
    pkpy_SStream__append_cstr(self, str);
}

pkpy_Str pkpy_SStream__to_Str(pkpy_SStream* self) {
    pkpy_Str res;
    pkpy_Str__ctor2(&res, c11_vector__data(&self->data), c11_vector__size(&self->data));
    return res;
}
