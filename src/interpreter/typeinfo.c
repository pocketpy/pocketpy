#include "pocketpy/interpreter/typeinfo.h"

#define CHUNK_SIZE 128
#define LOG2_CHUNK_SIZE 7

void TypeList__ctor(TypeList* self) {
    self->length = 0;
    memset(self->chunks, 0, sizeof(self->chunks));
}

void TypeList__dtor(TypeList* self) {
    for (int i = 0; i < self->length; i++) {
        if(self->chunks[i]) free(self->chunks[i]);
    }
}

py_TypeInfo* TypeList__get(TypeList* self, py_Type index) {
    assert(index < self->length);
    int chunk = index >> LOG2_CHUNK_SIZE;
    int offset = index & (CHUNK_SIZE - 1);
    return self->chunks[chunk] + offset;
}

py_TypeInfo* TypeList__emplace(TypeList* self){
    int chunk = self->length >> LOG2_CHUNK_SIZE;
    int offset = self->length & (CHUNK_SIZE - 1);
    assert(chunk < 256);
    if(self->chunks[chunk] == NULL){
        self->chunks[chunk] = malloc(sizeof(py_TypeInfo) * CHUNK_SIZE);
        memset(self->chunks[chunk], 0, sizeof(py_TypeInfo) * CHUNK_SIZE);
    }
    self->length++;
    return self->chunks[chunk] + offset;
}

void TypeList__apply(TypeList* self, void (*f)(py_TypeInfo*, void*), void* ctx) {
    for (int i = 0; i < self->length; i++) {
        py_TypeInfo* info = TypeList__get(self, i);
        f(info, ctx);
    }
}