#include "pocketpy/interpreter/objectpool.h"

#include "pocketpy/config.h"
#include "pocketpy/objects/object.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static PoolArena* PoolArena__new(int block_size) {
    assert(kPoolArenaSize % block_size == 0);
    int block_count = kPoolArenaSize / block_size;
    int total_size = sizeof(PoolArena) + sizeof(int) * block_count;
    PoolArena* self = PK_MALLOC(total_size);
    self->block_size = block_size;
    self->block_count = block_count;
    self->unused_count = block_count;
    self->unused = PK_MALLOC(sizeof(int) * block_count);
    for(int i = 0; i < block_count; i++) {
        self->unused[i] = i;
    }
    memset(self->data, 0, kPoolArenaSize);
    return self;
}

static void PoolArena__delete(PoolArena* self) {
    for(int i = 0; i < self->block_count; i++) {
        PyObject* obj = (PyObject*)(self->data + i * self->block_size);
        if(obj->type != 0) PyObject__dtor(obj);
    }
    PK_FREE(self->unused);
    PK_FREE(self);
}

static void* PoolArena__alloc(PoolArena* self) {
    assert(self->unused_count > 0);
    int index = self->unused[self->unused_count - 1];
    self->unused_count--;
    return self->data + index * self->block_size;
}

static int PoolArena__sweep_dealloc(PoolArena* self) {
    int freed = 0;
    self->unused_count = 0;
    for(int i = 0; i < self->block_count; i++) {
        PyObject* obj = (PyObject*)(self->data + i * self->block_size);
        if(obj->type == 0) {
            self->unused[self->unused_count] = i;
            self->unused_count++;
        } else {
            if(!obj->gc_marked) {
                obj->type = 0;
                freed++;
                self->unused[self->unused_count] = i;
                self->unused_count++;
            } else {
                obj->gc_marked = false;
            }
        }
    }
    return freed;
}

static void Pool__ctor(Pool* self, int block_size) {
    c11_vector__ctor(&self->arenas, sizeof(PoolArena*));
    c11_vector__ctor(&self->not_free_arenas, sizeof(PoolArena*));
    self->block_size = block_size;
}

static void Pool__dtor(Pool* self) {
    c11__foreach(PoolArena*, &self->arenas, arena) { PoolArena__delete(*arena); }
    c11__foreach(PoolArena*, &self->not_free_arenas, arena) { PoolArena__delete(*arena); }
    c11_vector__dtor(&self->arenas);
    c11_vector__dtor(&self->not_free_arenas);
}

static void* Pool__alloc(Pool* self) {
    PoolArena* arena;
    if(self->arenas.length == 0) {
        arena = PoolArena__new(self->block_size);
        c11_vector__push(PoolArena*, &self->arenas, arena);
    } else {
        arena = c11_vector__back(PoolArena*, &self->arenas);
    }
    void* ptr = PoolArena__alloc(arena);
    if(arena->unused_count == 0) {
        c11_vector__pop(&self->arenas);
        c11_vector__push(PoolArena*, &self->not_free_arenas, arena);
    }
    return ptr;
}

static int Pool__sweep_dealloc(Pool* self, c11_vector* arenas, c11_vector* not_free_arenas) {
    c11_vector__clear(arenas);
    c11_vector__clear(not_free_arenas);

    int freed = 0;
    for(int i = 0; i < self->arenas.length; i++) {
        PoolArena* item = c11__getitem(PoolArena*, &self->arenas, i);
        assert(item->unused_count > 0);
        freed += PoolArena__sweep_dealloc(item);
        if(item->unused_count == item->block_count) {
            // all free
            if(self->arenas.length > 0) {
                // at least one arena
                PoolArena__delete(item);
            } else {
                // no arena
                c11_vector__push(PoolArena*, arenas, item);
            }
        } else {
            // some free
            c11_vector__push(PoolArena*, arenas, item);
        }
    }
    for(int i = 0; i < self->not_free_arenas.length; i++) {
        PoolArena* item = c11__getitem(PoolArena*, &self->not_free_arenas, i);
        freed += PoolArena__sweep_dealloc(item);
        if(item->unused_count == 0) {
            // still not free
            c11_vector__push(PoolArena*, not_free_arenas, item);
        } else {
            if(item->unused_count == item->block_count) {
                // all free
                PoolArena__delete(item);
            } else {
                // some free
                c11_vector__push(PoolArena*, arenas, item);
            }
        }
    }

    c11_vector__swap(&self->arenas, arenas);
    c11_vector__swap(&self->not_free_arenas, not_free_arenas);
    return freed;
}

void* MultiPool__alloc(MultiPool* self, int size) {
    if(size == 0) return NULL;
    int index = (size - 1) >> 5;
    if(index < kMultiPoolCount) {
        Pool* pool = &self->pools[index];
        return Pool__alloc(pool);
    }
    return NULL;
}

int MultiPool__sweep_dealloc(MultiPool* self) {
    c11_vector arenas;
    c11_vector not_free_arenas;
    c11_vector__ctor(&arenas, sizeof(PoolArena*));
    c11_vector__ctor(&not_free_arenas, sizeof(PoolArena*));
    int freed = 0;
    for(int i = 0; i < kMultiPoolCount; i++) {
        Pool* item = &self->pools[i];
        freed += Pool__sweep_dealloc(item, &arenas, &not_free_arenas);
    }
    c11_vector__dtor(&arenas);
    c11_vector__dtor(&not_free_arenas);
    return freed;
}

void MultiPool__ctor(MultiPool* self) {
    for(int i = 0; i < kMultiPoolCount; i++) {
        Pool__ctor(&self->pools[i], 32 * (i + 1));
    }
}

void MultiPool__dtor(MultiPool* self) {
    for(int i = 0; i < kMultiPoolCount; i++) {
        Pool__dtor(&self->pools[i]);
    }
}