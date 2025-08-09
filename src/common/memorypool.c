#include "pocketpy/common/memorypool.h"
#include "pocketpy/config.h"

#include <stdbool.h>

void FixedMemoryPool__ctor(FixedMemoryPool* self, int BlockSize, int BlockCount) {
    self->BlockSize = BlockSize;
    self->BlockCount = BlockCount;
    self->exceeded_bytes = 0;
    self->data = PK_MALLOC(BlockSize * BlockCount);
    self->data_end = self->data + BlockSize * BlockCount;
    self->_free_list = PK_MALLOC(sizeof(void*) * BlockCount);
    self->_free_list_length = BlockCount;
    for(int i = 0; i < BlockCount; i++) {
        self->_free_list[i] = self->data + i * BlockSize;
    }
}

void FixedMemoryPool__dtor(FixedMemoryPool* self) {
    PK_FREE(self->_free_list);
    PK_FREE(self->data);
}

void* FixedMemoryPool__alloc(FixedMemoryPool* self) {
    if(self->_free_list_length > 0) {
        self->_free_list_length--;
        return self->_free_list[self->_free_list_length];
    } else {
        self->exceeded_bytes += self->BlockSize;
        return PK_MALLOC(self->BlockSize);
    }
}

void FixedMemoryPool__dealloc(FixedMemoryPool* self, void* p) {
    bool is_valid = (char*)p >= self->data && (char*)p < self->data_end;
    if(is_valid) {
        self->_free_list[self->_free_list_length] = p;
        self->_free_list_length++;
    } else {
        self->exceeded_bytes -= self->BlockSize;
        PK_FREE(p);
    }
}

// static int FixedMemoryPool__used_bytes(FixedMemoryPool* self) {
//     return (self->_free_list_end - self->_free_list) * self->BlockSize;
// }

// static int FixedMemoryPool__total_bytes(FixedMemoryPool* self) {
//     return self->BlockCount * self->BlockSize;
// }