#include "pocketpy/common/vector.h"

#include <stdlib.h>
#include <string.h>

void c11_array__ctor(c11_array* self, int elem_size, int count){
    self->data = malloc(elem_size * count);
    self->count = count;
    self->elem_size = elem_size;
}

void c11_array__dtor(c11_array* self){
    free(self->data);
    self->data = NULL;
    self->count = 0;
}

c11_array c11_array__copy(const c11_array* self){
    c11_array retval;
    c11_array__ctor(&retval, self->elem_size, self->count);
    memcpy(retval.data, self->data, self->elem_size * self->count);
    return retval;
}

void c11_vector__ctor(c11_vector* self, int elem_size){
    self->data = NULL;
    self->count = 0;
    self->capacity = 0;
    self->elem_size = elem_size;
}

void c11_vector__dtor(c11_vector* self){
    if(self->data) free(self->data);
    self->data = NULL;
    self->count = 0;
    self->capacity = 0;
}

c11_vector c11_vector__copy(const c11_vector* self){
    c11_vector retval;
    c11_vector__ctor(&retval, self->elem_size);
    c11_vector__reserve(&retval, self->capacity);
    memcpy(retval.data, self->data, self->elem_size * self->count);
    retval.count = self->count;
    return retval;
}

void c11_vector__reserve(c11_vector* self, int capacity){
    if(capacity < 4) capacity = 4;
    if(capacity <= self->capacity) return;
    self->data = realloc(self->data, self->elem_size * capacity);
    self->capacity = capacity;
}

void c11_vector__clear(c11_vector* self){
    self->count = 0;
}

void* c11_vector__emplace(c11_vector* self){
    if(self->count == self->capacity) c11_vector__reserve(self, self->capacity*2);
    void* p = (char*)self->data + self->elem_size * self->count;
    self->count++;
    return p;
}

c11_array c11_vector__submit(c11_vector* self){
    c11_array retval = {
        .data = self->data,
        .count = self->count,
        .elem_size = self->elem_size
    };
    self->data = NULL;
    self->count = 0;
    self->capacity = 0;
    return retval;
}
