#pragma once

#include <string.h>
#include <stdlib.h>
#include "pocketpy/common/algorithm.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct c11_array{
    void* data;
    int count;
    int elem_size;
} c11_array;

void c11_array__ctor(c11_array* self, int elem_size, int count);
void c11_array__dtor(c11_array* self);
c11_array c11_array__copy(const c11_array* self);
void* c11_array__at(c11_array* self, int index);

typedef struct c11_vector{
    void* data;
    int count;
    int capacity;
    int elem_size;
} c11_vector;

void c11_vector__ctor(c11_vector* self, int elem_size);
void c11_vector__dtor(c11_vector* self);
c11_vector c11_vector__copy(const c11_vector* self);
void* c11_vector__at(c11_vector* self, int index);
void c11_vector__reserve(c11_vector* self, int capacity);
void c11_vector__clear(c11_vector* self);

#define c11__getitem(T, self, index) ((T*)(self)->data)[index]
#define c11__setitem(T, self, index, value) ((T*)(self)->data)[index] = value;

#define c11_vector__push(T, self, elem) \
    do{ \
        if((self)->count == (self)->capacity) c11_vector__reserve((self), (self)->capacity*2); \
        ((T*)(self)->data)[(self)->count] = (elem); \
        (self)->count++; \
    }while(0)

#define c11_vector__pop(T, self) \
    do{ \
        (self)->count--; \
    }while(0)

#define c11_vector__extend(T, self, p, size) \
    do{ \
        c11_vector__reserve((self), (self)->count + (size)); \
        memcpy((T*)(self)->data + (self)->count, (p), (size) * sizeof(T)); \
        (self)->count += (size); \
    }while(0)


#define c11_vector__insert(T, self, p, elem) \
    do{ \
        if((self)->count == (self)->capacity) c11_vector__reserve((self), (self)->capacity*2); \
        int __n = (self)->count - (p - (T*)(self)->data); \
        memmove(p + 1, p, __n * sizeof(T)); \
        *p = (elem); \
        (self)->count++; \
    }while(0)

#define c11_vector__erase(T, self, p) \
    do{ \
        int __n = (self)->count - (p - (T*)(self)->data) - 1; \
        memmove(p, p + 1, __n * sizeof(T)); \
        (self)->count--; \
    }while(0)

#define c11_vector__reverse(T, self, start, end) \
    do{ \
        T* p = (T*)(self)->data + (start); \
        T* q = (T*)(self)->data + (end); \
        while(p < q){ \
            T tmp = *p; *p = *q; *q = tmp; \
            p++; q--; \
        } \
    }while(0)

#ifdef __cplusplus
}
#endif