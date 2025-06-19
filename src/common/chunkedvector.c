#include "pocketpy/common/chunkedvector.h"

#include <stdlib.h>
#include <string.h>
#include "pocketpy/common/utils.h"
#include "pocketpy/config.h"

void c11_chunkedvector__ctor(c11_chunkedvector* self, int elem_size, int initial_chunks) {
    c11_vector__ctor(&self->chunks, sizeof(c11_chunkedvector_chunk));
    self->length = 0;
    self->capacity = 0;
    self->elem_size = elem_size;
    self->initial_chunks = initial_chunks;

    for(int i = 0; i < initial_chunks; i++) {
        // TODO: optimize?
        c11_chunkedvector_chunk chunk;
        chunk.length = 0;
        chunk.capacity = (1 << i);
        chunk.elem_size = elem_size;
        chunk.data = PK_MALLOC(chunk.capacity * elem_size);
        c11_vector__push(c11_chunkedvector_chunk, &self->chunks, chunk);
        initial_capacity += chunk.capacity;
    }
}

void c11_chunkedvector__dtor(c11_chunkedvector* self) {}

void* c11_chunkedvector__emplace(c11_chunkedvector* self) {}

static int c11__bit_length(unsigned int n) { return n == 0 ? 0 : 32 - __builtin_clz(n); }

void* c11_chunkedvector__at(c11_chunkedvector* self, int index) {
    int chunk_index = c11__bit_length(index) - 1;
}
