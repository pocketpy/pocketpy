#include "pocketpy/common/chunkedvector.h"
#include "pocketpy/pocketpy.h"
#include <assert.h>

#if defined(_MSC_VER)
#include <intrin.h>
#endif

PK_INLINE static int c11__bit_length(unsigned long x) {
#if(defined(__clang__) || defined(__GNUC__))
    return x == 0 ? 0 : (int)sizeof(unsigned long) * 8 - __builtin_clzl(x);
#elif defined(_MSC_VER)
    static_assert(sizeof(unsigned long) <= 4, "unsigned long is greater than 4 bytes");
    unsigned long msb;
    if(_BitScanReverse(&msb, x)) { return (int)msb + 1; }
    return 0;
#else
    const int BIT_LENGTH_TABLE[32] = {0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4,
                                      5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5};
    int msb = 0;
    while(x >= 32) {
        msb += 6;
        x >>= 6;
    }
    msb += BIT_LENGTH_TABLE[x];
    return msb;
#endif
}

void c11_chunkedvector__ctor(c11_chunkedvector* self, int elem_size, int initial_chunks) {
    if(initial_chunks < 5) initial_chunks = 5;
    c11_vector__ctor(&self->chunks, sizeof(c11_chunkedvector_chunk));
    self->length = 0;
    self->capacity = (1U << (unsigned int)initial_chunks) - 1U;
    self->elem_size = elem_size;
    self->initial_chunks = initial_chunks;
    void* chunks_data = PK_MALLOC(elem_size * ((1U << (unsigned int)initial_chunks) - 1));
    for(int i = 0; i < initial_chunks; i++) {
        c11_chunkedvector_chunk chunk = {.length = 0,
                                         .capacity = 1U << i,
                                         .data = (char*)chunks_data + elem_size * ((1U << i) - 1U)};
        c11_vector__push(c11_chunkedvector_chunk, &self->chunks, chunk);
    }
}

void c11_chunkedvector__dtor(c11_chunkedvector* self) {
    for(int index = self->initial_chunks; index < self->chunks.length; index++) {
        c11_chunkedvector_chunk* chunk = c11__at(c11_chunkedvector_chunk, &self->chunks, index);
        PK_FREE(chunk->data);
    }
    c11_chunkedvector_chunk* initial_chunk = c11__at(c11_chunkedvector_chunk, &self->chunks, 0);
    PK_FREE(initial_chunk->data);
    c11_vector__dtor(&self->chunks);
}

void* c11_chunkedvector__emplace(c11_chunkedvector* self) {
    if(self->length == self->capacity) {
#ifndef NDEBUG
        c11_chunkedvector_chunk last_chunk =
            c11_vector__back(c11_chunkedvector_chunk, &self->chunks);
        assert(last_chunk.capacity == last_chunk.length);
#endif
        c11_chunkedvector_chunk chunk = {
            .length = 0,
            .capacity = 1U << (unsigned int)self->chunks.length,
            .data = PK_MALLOC(self->elem_size * ((1U << (unsigned int)self->chunks.length)))};
        self->capacity += chunk.capacity;
        c11_vector__push(c11_chunkedvector_chunk, &self->chunks, chunk);
    }
#if 1
    int last_chunk_index = c11__bit_length(self->length + 1) - 1;
    c11_chunkedvector_chunk* last_chunk =
        c11__at(c11_chunkedvector_chunk, &self->chunks, last_chunk_index);
#else
    // This is not correct, because there is some pre-allocated chunks
    c11_chunkedvector_chunk* last_chunk = &c11_vector__back(c11_chunkedvector_chunk, &self->chunks);
#endif
    void* p = (char*)last_chunk->data + self->elem_size * last_chunk->length;
    last_chunk->length++;
    self->length++;
    return p;
}

void* c11_chunkedvector__at(c11_chunkedvector* self, int index) {
    int chunk_index = c11__bit_length(index + 1) - 1;
    c11_chunkedvector_chunk* chunk = c11__at(c11_chunkedvector_chunk, &self->chunks, chunk_index);
    return (char*)chunk->data + (index + 1 - (1U << (unsigned int)chunk_index)) * self->elem_size;
}
