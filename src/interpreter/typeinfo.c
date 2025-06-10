#include "pocketpy/interpreter/typeinfo.h"
#include "pocketpy/common/utils.h"

#define CHUNK_SIZE 128
#define LOG2_CHUNK_SIZE 7

void TypeList__ctor(TypeList* self) {
    self->length = 0;
    memset(self->chunks, 0, sizeof(self->chunks));
}

void TypeList__dtor(TypeList* self) {
    for(py_Type t = 0; t < self->length; t++) {
        py_TypeInfo* info = TypeList__get(self, t);
        c11_vector__dtor(&info->ordered_attrs);
    }
    for(int i = 0; i < PK_MAX_CHUNK_LENGTH; i++) {
        if(self->chunks[i]) PK_FREE(self->chunks[i]);
    }
}

py_TypeInfo* TypeList__get(TypeList* self, py_Type index) {
    assert(index < self->length);
    int chunk = index >> LOG2_CHUNK_SIZE;
    int offset = index & (CHUNK_SIZE - 1);
    return self->chunks[chunk] + offset;
}

py_TypeInfo* TypeList__emplace(TypeList* self) {
    int chunk = self->length >> LOG2_CHUNK_SIZE;
    int offset = self->length & (CHUNK_SIZE - 1);
    if(self->chunks[chunk] == NULL) {
        if(chunk >= PK_MAX_CHUNK_LENGTH) {
            c11__abort("TypeList__emplace(): max chunk length exceeded");
        }
        self->chunks[chunk] = PK_MALLOC(sizeof(py_TypeInfo) * CHUNK_SIZE);
        memset(self->chunks[chunk], 0, sizeof(py_TypeInfo) * CHUNK_SIZE);
    }
    self->length++;
    return self->chunks[chunk] + offset;
}

void TypeList__apply(TypeList* self, void (*f)(py_TypeInfo*, void*), void* ctx) {
    for(int i = 0; i < self->length; i++) {
        py_TypeInfo* info = TypeList__get(self, i);
        f(info, ctx);
    }
}

#undef CHUNK_SIZE
#undef LOG2_CHUNK_SIZE