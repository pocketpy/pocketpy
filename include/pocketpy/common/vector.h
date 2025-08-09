#pragma once

#include <string.h>
#include <stdbool.h>

#include "pocketpy/common/algorithm.h"

typedef struct c11_vector {
    void* data;
    int length;
    int capacity;
    int elem_size;
} c11_vector;

void c11_vector__ctor(c11_vector* self, int elem_size);
void c11_vector__dtor(c11_vector* self);
c11_vector c11_vector__copy(const c11_vector* self);
void c11_vector__reserve(c11_vector* self, int capacity);
void c11_vector__clear(c11_vector* self);
void* c11_vector__emplace(c11_vector* self);
bool c11_vector__contains(const c11_vector* self, void* elem);
void* c11_vector__submit(c11_vector* self, int* length);
void c11_vector__swap(c11_vector* self, c11_vector* other);
int c11_vector__nextcap(c11_vector* self);

#define c11__getitem(T, self, index) (((T*)(self)->data)[index])
#define c11__setitem(T, self, index, value) ((T*)(self)->data)[index] = value;
#define c11__at(T, self, index) ((T*)(self)->data + index)

#define c11_vector__push(T, self, elem)                                                            \
    do {                                                                                           \
        if((self)->length == (self)->capacity) {                                                   \
            c11_vector__reserve((self), c11_vector__nextcap((self)));                              \
        }                                                                                          \
        ((T*)(self)->data)[(self)->length] = (elem);                                               \
        (self)->length++;                                                                          \
    } while(0)

#define c11_vector__pop(self) (--(self)->length)

#define c11_vector__back(T, self) (((T*)(self)->data)[(self)->length - 1])

#define c11_vector__extend(T, self, p, size)                                                       \
    do {                                                                                           \
        int min_capacity = (self)->length + (size);                                                \
        if((self)->capacity < min_capacity) {                                                      \
            int nextcap = c11_vector__nextcap(self);                                               \
            c11_vector__reserve((self), c11__max(nextcap, min_capacity));                          \
        }                                                                                          \
        memcpy((T*)(self)->data + (self)->length, (p), (size) * sizeof(T));                        \
        (self)->length += (size);                                                                  \
    } while(0)

#define c11_vector__insert(T, self, index, elem)                                                   \
    do {                                                                                           \
        if((self)->length == (self)->capacity) {                                                   \
            c11_vector__reserve((self), c11_vector__nextcap(self));                                \
        }                                                                                          \
        T* p = (T*)(self)->data + (index);                                                         \
        memmove(p + 1, p, ((self)->length - (index)) * sizeof(T));                                 \
        *p = (elem);                                                                               \
        (self)->length++;                                                                          \
    } while(0)

#define c11_vector__erase(T, self, index)                                                          \
    do {                                                                                           \
        T* p = (T*)(self)->data + (index);                                                         \
        memmove(p, p + 1, ((self)->length - (index)-1) * sizeof(T));                               \
        (self)->length--;                                                                          \
    } while(0)

#define c11__reverse(T, self)                                                                      \
    do {                                                                                           \
        if(!self->data) break;                                                                     \
        T* p = (T*)(self)->data;                                                                   \
        T* q = (T*)(self)->data + (self)->length - 1;                                              \
        while(p < q) {                                                                             \
            T tmp = *p;                                                                            \
            *p = *q;                                                                               \
            *q = tmp;                                                                              \
            p++;                                                                                   \
            q--;                                                                                   \
        }                                                                                          \
    } while(0)

// NOTE: here we do an extra NULL check for it to avoid UB
#define c11__foreach(T, self, it)                                                                  \
    for(T* it = (self)->data; it && it != (T*)(self)->data + (self)->length; it++)
