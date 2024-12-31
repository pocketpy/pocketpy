#include "pocketpy/common/memorypool.h"
#include "pocketpy/pocketpy.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct LinkedListNode {
    struct LinkedListNode* prev;
    struct LinkedListNode* next;
} LinkedListNode;

typedef struct LinkedList {
    int length;
    LinkedListNode head;
    LinkedListNode tail;
} LinkedList;

static void LinkedList__ctor(LinkedList* self) {
    self->length = 0;
    self->head.prev = NULL;
    self->head.next = &self->tail;
    self->tail.prev = &self->head;
    self->tail.next = NULL;
}

static void LinkedList__push_back(LinkedList* self, LinkedListNode* node) {
    node->prev = self->tail.prev;
    node->next = &self->tail;
    self->tail.prev->next = node;
    self->tail.prev = node;
    self->length++;
}

static void LinkedList__push_front(LinkedList* self, LinkedListNode* node) {
    node->prev = &self->head;
    node->next = self->head.next;
    self->head.next->prev = node;
    self->head.next = node;
    self->length++;
}

static void LinkedList__pop_back(LinkedList* self) {
    assert(self->length > 0);
    self->tail.prev->prev->next = &self->tail;
    self->tail.prev = self->tail.prev->prev;
    self->length--;
}

static LinkedListNode* LinkedList__back(LinkedList* self) {
    assert(self->length > 0);
    return self->tail.prev;
}

static void LinkedList__erase(LinkedList* self, LinkedListNode* node) {
    node->prev->next = node->next;
    node->next->prev = node->prev;
    self->length--;
}

#define LinkedList__apply(self, __STATEMENTS__) \
    do { \
        LinkedListNode* node = (self)->head.next; \
        while(node != &(self)->tail) { \
            LinkedListNode* next = node->next; \
            __STATEMENTS__ \
            node = next; \
        } \
    } while(0)

typedef struct MemoryPoolBlock{
    void* arena;
    char data[kPoolObjectBlockSize];
} MemoryPoolBlock;

typedef struct MemoryPoolArena{
    /* LinkedListNode */
    LinkedListNode* prev;
    LinkedListNode* next;
    /* Arena */
    MemoryPoolBlock _blocks[kPoolObjectMaxBlocks];
    MemoryPoolBlock* _free_list[kPoolObjectMaxBlocks];
    int _free_list_size;
} MemoryPoolArena;

typedef struct MemoryPool{
    LinkedList _arenas;
    LinkedList _empty_arenas;
} MemoryPool;

static void MemoryPoolArena__ctor(MemoryPoolArena* self) {
    self->prev = NULL;
    self->next = NULL;
    self->_free_list_size = kPoolObjectMaxBlocks;
    for(int i = 0; i < kPoolObjectMaxBlocks; i++) {
        self->_blocks[i].arena = self;
        self->_free_list[i] = &self->_blocks[i];
    }
}

static bool MemoryPoolArena__empty(MemoryPoolArena* self) {
    return self->_free_list_size == 0;
}

static bool MemoryPoolArena__full(MemoryPoolArena* self) {
    return self->_free_list_size == kPoolObjectMaxBlocks;
}

static int MemoryPoolArena__total_bytes(MemoryPoolArena* self) {
    return kPoolObjectArenaSize;
}

static int MemoryPoolArena__used_bytes(MemoryPoolArena* self) {
    return kPoolObjectBlockSize * (kPoolObjectMaxBlocks - self->_free_list_size);
}

static MemoryPoolBlock* MemoryPoolArena__alloc(MemoryPoolArena* self) {
    assert(!MemoryPoolArena__empty(self));
    self->_free_list_size--;
    return self->_free_list[self->_free_list_size];
}

static void MemoryPoolArena__dealloc(MemoryPoolArena* self, MemoryPoolBlock* block) {
    assert(!MemoryPoolArena__full(self));
    self->_free_list[self->_free_list_size] = block;
    self->_free_list_size++;
}

static void MemoryPool__ctor(MemoryPool* self) {
    LinkedList__ctor(&self->_arenas);
    LinkedList__ctor(&self->_empty_arenas);
}

static void* MemoryPool__alloc(MemoryPool* self) {
    MemoryPoolArena* arena;
    if(self->_arenas.length == 0){
        arena = PK_MALLOC(sizeof(MemoryPoolArena));
        MemoryPoolArena__ctor(arena);
        LinkedList__push_back(&self->_arenas, (LinkedListNode*)arena);
    } else {
        arena = (MemoryPoolArena*)LinkedList__back(&self->_arenas);
    }
    void* p = MemoryPoolArena__alloc(arena)->data;
    if(MemoryPoolArena__empty(arena)) {
        LinkedList__pop_back(&self->_arenas);
        LinkedList__push_back(&self->_empty_arenas, (LinkedListNode*)arena);
    }
    return p;
}

static void MemoryPool__dealloc(MemoryPool* self, void* p) {
    assert(p != NULL);
    MemoryPoolBlock* block = (MemoryPoolBlock*)((char*)p - sizeof(void*));
    assert(block->arena != NULL);
    MemoryPoolArena* arena = (MemoryPoolArena*)block->arena;
    if(MemoryPoolArena__empty(arena)) {
        LinkedList__erase(&self->_empty_arenas, (LinkedListNode*)arena);
        LinkedList__push_front(&self->_arenas, (LinkedListNode*)arena);
    }
    MemoryPoolArena__dealloc(arena, block);
}

static void MemoryPool__shrink_to_fit(MemoryPool* self) {
    const int MIN_ARENA_COUNT = PK_GC_MIN_THRESHOLD * 100 / (kPoolObjectArenaSize);
    if(self->_arenas.length < MIN_ARENA_COUNT) return;
    LinkedList__apply(&self->_arenas,
            MemoryPoolArena* arena = (MemoryPoolArena*)node;
            if(MemoryPoolArena__full(arena)) {
                LinkedList__erase(&self->_arenas, node);
                PK_FREE(arena);
            });
}


static void MemoryPool__dtor(MemoryPool* self) {
    LinkedList__apply(&self->_arenas, PK_FREE(node););
    LinkedList__apply(&self->_empty_arenas, PK_FREE(node););
}

typedef struct FixedMemoryPool {
    int BlockSize;
    int BlockCount;

    char* data;
    char* data_end;
    int exceeded_bytes;

    char** _free_list;
    char** _free_list_end;
} FixedMemoryPool;

static void FixedMemoryPool__ctor(FixedMemoryPool* self, int BlockSize, int BlockCount) {
    self->BlockSize = BlockSize;
    self->BlockCount = BlockCount;
    self->exceeded_bytes = 0;
    self->data = PK_MALLOC(BlockSize * BlockCount);
    self->data_end = self->data + BlockSize * BlockCount;
    self->_free_list = PK_MALLOC(sizeof(void*) * BlockCount);
    self->_free_list_end = self->_free_list;
    for(int i = 0; i < BlockCount; i++) {
        self->_free_list[i] = self->data + i * BlockSize;
    }
}

static void FixedMemoryPool__dtor(FixedMemoryPool* self) {
    PK_FREE(self->_free_list);
    PK_FREE(self->data);
}

static void* FixedMemoryPool__alloc(FixedMemoryPool* self) {
    if(self->_free_list_end != self->_free_list) {
        self->_free_list_end--;
        return *self->_free_list_end;
    } else {
        self->exceeded_bytes += self->BlockSize;
        return PK_MALLOC(self->BlockSize);
    }
}

static void FixedMemoryPool__dealloc(FixedMemoryPool* self, void* p) {
    bool is_valid = (char*)p >= self->data && (char*)p < self->data_end;
    if(is_valid) {
        *self->_free_list_end = p;
        self->_free_list_end++;
    } else {
        self->exceeded_bytes -= self->BlockSize;
        PK_FREE(p);
    }
}

static int FixedMemoryPool__used_bytes(FixedMemoryPool* self) {
    return (self->_free_list_end - self->_free_list) * self->BlockSize;
}

static int FixedMemoryPool__total_bytes(FixedMemoryPool* self) {
    return self->BlockCount * self->BlockSize;
}

static FixedMemoryPool PoolExpr;
static FixedMemoryPool PoolFrame;
static MemoryPool PoolObject;

void MemoryPools__initialize(){
    FixedMemoryPool__ctor(&PoolExpr, kPoolExprBlockSize, 64);
    FixedMemoryPool__ctor(&PoolFrame, kPoolFrameBlockSize, 128);
    MemoryPool__ctor(&PoolObject);
}

void MemoryPools__finalize(){
    FixedMemoryPool__dtor(&PoolExpr);
    FixedMemoryPool__dtor(&PoolFrame);
    MemoryPool__dtor(&PoolObject);
}

void* PoolExpr_alloc() {
    return FixedMemoryPool__alloc(&PoolExpr);
}

void PoolExpr_dealloc(void* p) {
    FixedMemoryPool__dealloc(&PoolExpr, p);
}

void* PoolFrame_alloc() {
    return FixedMemoryPool__alloc(&PoolFrame);
}

void PoolFrame_dealloc(void* p) {
    FixedMemoryPool__dealloc(&PoolFrame, p);
}

void* PoolObject_alloc() {
    return MemoryPool__alloc(&PoolObject);
}

void PoolObject_dealloc(void* p) {
    MemoryPool__dealloc(&PoolObject, p);
}

void PoolObject_shrink_to_fit() {
    MemoryPool__shrink_to_fit(&PoolObject);
}

void Pools_debug_info(char* buffer, int size) {
    double BYTES_PER_MB = 1024.0f * 1024.0f;
    double BYTES_PER_KB = 1024.0f;
    int n = 0;
    n = snprintf(
        buffer, size,  "PoolExpr: %.2f KB (used) / %.2f KB (total) - %.2f KB (exceeded)\n",
        FixedMemoryPool__used_bytes(&PoolExpr) / BYTES_PER_KB,
        FixedMemoryPool__total_bytes(&PoolExpr) / BYTES_PER_KB,
        PoolExpr.exceeded_bytes / BYTES_PER_KB
    );
    buffer += n; size -= n;
    n = snprintf(
        buffer, size, "PoolFrame: %.2f KB (used) / %.2f KB (total) - %.2f KB (exceeded)\n",
        FixedMemoryPool__used_bytes(&PoolFrame) / BYTES_PER_KB,
        FixedMemoryPool__total_bytes(&PoolFrame) / BYTES_PER_KB,
        PoolFrame.exceeded_bytes / BYTES_PER_KB
    );
    buffer += n; size -= n;
    // PoolObject
    int empty_arenas = PoolObject._empty_arenas.length;
    int arenas = PoolObject._arenas.length;
    // print empty arenas count
    n = snprintf(
        buffer, size, "PoolObject: %d empty arenas, %d arenas\n",
        empty_arenas, arenas
    );
    buffer += n; size -= n;
    // log each non-empty arena
    LinkedList__apply(&PoolObject._arenas,
        MemoryPoolArena* arena = (MemoryPoolArena*)node;
        n = snprintf(
            buffer, size, "  - %p: %.2f MB (used) / %.2f MB (total)\n",
            (void*)arena,
            MemoryPoolArena__used_bytes(arena) / BYTES_PER_MB,
            MemoryPoolArena__total_bytes(arena) / BYTES_PER_MB
        );
        buffer += n; size -= n;
    );
}

#undef LinkedList__apply