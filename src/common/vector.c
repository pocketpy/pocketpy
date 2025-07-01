#include "pocketpy/common/vector.h"

#include <string.h>
#include "pocketpy/common/utils.h"
#include "pocketpy/config.h"

void c11_vector__ctor(c11_vector* self, int elem_size) {
    self->data = NULL;
    self->length = 0;
    self->capacity = 0;
    self->elem_size = elem_size;
}

void c11_vector__dtor(c11_vector* self) {
    if(self->data) PK_FREE(self->data);
    self->data = NULL;
    self->length = 0;
    self->capacity = 0;
}

c11_vector c11_vector__copy(const c11_vector* self) {
    c11_vector retval;
    c11_vector__ctor(&retval, self->elem_size);
    c11_vector__reserve(&retval, self->capacity);
    memcpy(retval.data, self->data, (size_t)self->elem_size * (size_t)self->length);
    retval.length = self->length;
    return retval;
}

void c11_vector__reserve(c11_vector* self, int capacity) {
    if(capacity < 4) capacity = 4;
    if(capacity <= self->capacity) return;
    // self->elem_size * capacity may overflow
    self->data = PK_REALLOC(self->data, (size_t)self->elem_size * (size_t)capacity);
    if(self->data == NULL) c11__abort("c11_vector__reserve(): out of memory");
    self->capacity = capacity;
}

void c11_vector__clear(c11_vector* self) { self->length = 0; }

void* c11_vector__emplace(c11_vector* self) {
    if(self->length == self->capacity) { c11_vector__reserve(self, c11_vector__nextcap(self)); }
    void* p = (char*)self->data + (size_t)self->elem_size * (size_t)self->length;
    self->length++;
    return p;
}

bool c11_vector__contains(const c11_vector* self, void* elem) {
    for(int i = 0; i < self->length; i++) {
        void* p = (char*)self->data + (size_t)self->elem_size * (size_t)i;
        if(memcmp(p, elem, self->elem_size) == 0) return true;
    }
    return false;
}

void* c11_vector__submit(c11_vector* self, int* length) {
    void* retval = self->data;
    *length = self->length;
    self->data = NULL;
    self->length = 0;
    self->capacity = 0;
    return retval;
}

void c11_vector__swap(c11_vector* self, c11_vector* other) {
    c11_vector tmp = *self;
    *self = *other;
    *other = tmp;
}

int c11_vector__nextcap(c11_vector* self) {
    if(self->capacity < 1024) {
        return self->capacity * 2;
    } else {
        // increase by 25%
        return self->capacity + (self->capacity >> 2);
    }
}