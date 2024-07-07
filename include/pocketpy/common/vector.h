#pragma once

#include "pocketpy/common/algorithm.h"

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct c11_array {
    void* data;
    int count;
    int elem_size;
} c11_array;

void c11_array__ctor(c11_array* self, int elem_size, int count);
void c11_array__dtor(c11_array* self);
c11_array c11_array__copy(const c11_array* self);

typedef struct c11_vector {
    void* data;
    int count;
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
c11_array c11_vector__submit(c11_vector* self);

#define c11__getitem(T, self, index) (((T*)(self)->data)[index])
#define c11__setitem(T, self, index, value) ((T*)(self)->data)[index] = value;
#define c11__at(T, self, index) ((T*)(self)->data + index)

#define c11_vector__push(T, self, elem)                                                            \
    do {                                                                                           \
        if((self)->count == (self)->capacity) c11_vector__reserve((self), (self)->capacity * 2);   \
        ((T*)(self)->data)[(self)->count] = (elem);                                                \
        (self)->count++;                                                                           \
    } while(0)

#define c11_vector__pop(self) (--(self)->count)

#define c11_vector__back(T, self) (((T*)(self)->data)[(self)->count - 1])

#define c11_vector__extend(T, self, p, size)                                                       \
    do {                                                                                           \
        c11_vector__reserve((self), (self)->count + (size));                                       \
        memcpy((T*)(self)->data + (self)->count, (p), (size) * sizeof(T));                         \
        (self)->count += (size);                                                                   \
    } while(0)

#define c11_vector__insert(T, self, index, elem)                                                   \
    do {                                                                                           \
        if((self)->count == (self)->capacity) c11_vector__reserve((self), (self)->capacity * 2);   \
        T* p = (T*)(self)->data + (index);                                                         \
        memmove(p + 1, p, ((self)->count - (index)) * sizeof(T));                                  \
        *p = (elem);                                                                               \
        (self)->count++;                                                                           \
    } while(0)

#define c11_vector__erase(T, self, index)                                                          \
    do {                                                                                           \
        T* p = (T*)(self)->data + (index);                                                         \
        memmove(p, p + 1, ((self)->count - (index)-1) * sizeof(T));                                \
        (self)->count--;                                                                           \
    } while(0)

#define c11__reverse(T, self)                                                                      \
    do {                                                                                           \
        T* p = (T*)(self)->data;                                                                   \
        T* q = (T*)(self)->data + (self)->count - 1;                                               \
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
    for(T* it = (self)->data; it && it != (T*)(self)->data + (self)->count; it++)

#ifdef __cplusplus
}
#endif