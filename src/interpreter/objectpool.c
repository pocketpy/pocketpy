#include "pocketpy/interpreter/objectpool.h"

#include "pocketpy/objects/object.h"
#include "pocketpy/common/sstream.h"

#include <assert.h>
#include <stdbool.h>
#include <string.h>

static PoolArena* PoolArena__new(int block_size) {
    assert(kPoolArenaSize % block_size == 0);
    int block_count = kPoolArenaSize / block_size;
    PoolArena* self = PK_MALLOC(sizeof(PoolArena) + sizeof(int) * block_count);
    self->block_size = block_size;
    self->block_count = block_count;
    self->unused_length = block_count;
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
    PK_FREE(self);
}

static void* PoolArena__alloc(PoolArena* self) {
    assert(self->unused_length > 0);
    int index = self->unused[self->unused_length - 1];
    self->unused_length--;
    return self->data + index * self->block_size;
}

static int PoolArena__sweep_dealloc(PoolArena* self) {
    int freed = 0;
    self->unused_length = 0;
    for(int i = 0; i < self->block_count; i++) {
        PyObject* obj = (PyObject*)(self->data + i * self->block_size);
        if(obj->type == 0) {
            // free slot
            self->unused[self->unused_length] = i;
            self->unused_length++;
        } else {
            if(!obj->gc_marked) {
                // not marked, need to free
                PyObject__dtor(obj);
                obj->type = 0;
                freed++;
                self->unused[self->unused_length] = i;
                self->unused_length++;
            } else {
                // marked, clear mark
                obj->gc_marked = false;
            }
        }
    }
    return freed;
}

static void Pool__ctor(Pool* self, int block_size) {
    c11_vector__ctor(&self->arenas, sizeof(PoolArena*));
    c11_vector__ctor(&self->no_free_arenas, sizeof(PoolArena*));
    self->block_size = block_size;
}

static void Pool__dtor(Pool* self) {
    c11__foreach(PoolArena*, &self->arenas, arena) PoolArena__delete(*arena);
    c11__foreach(PoolArena*, &self->no_free_arenas, arena) PoolArena__delete(*arena);
    c11_vector__dtor(&self->arenas);
    c11_vector__dtor(&self->no_free_arenas);
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
    if(arena->unused_length == 0) {
        c11_vector__pop(&self->arenas);
        c11_vector__push(PoolArena*, &self->no_free_arenas, arena);
    }
    return ptr;
}

static int Pool__sweep_dealloc(Pool* self, c11_vector* arenas, c11_vector* no_free_arenas) {
    c11_vector__clear(arenas);
    c11_vector__clear(no_free_arenas);

    int freed = 0;
    for(int i = 0; i < self->arenas.length; i++) {
        PoolArena* item = c11__getitem(PoolArena*, &self->arenas, i);
        assert(item->unused_length > 0);
        freed += PoolArena__sweep_dealloc(item);
        if(item->unused_length == item->block_count) {
            // all free
            if(arenas->length > 0) {
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
    for(int i = 0; i < self->no_free_arenas.length; i++) {
        PoolArena* item = c11__getitem(PoolArena*, &self->no_free_arenas, i);
        freed += PoolArena__sweep_dealloc(item);
        if(item->unused_length == 0) {
            // still no free
            c11_vector__push(PoolArena*, no_free_arenas, item);
        } else {
            if(item->unused_length == item->block_count) {
                // all free
                PoolArena__delete(item);
            } else {
                // some free
                c11_vector__push(PoolArena*, arenas, item);
            }
        }
    }

    c11_vector__swap(&self->arenas, arenas);
    c11_vector__swap(&self->no_free_arenas, no_free_arenas);
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
    c11_vector no_free_arenas;
    c11_vector__ctor(&arenas, sizeof(PoolArena*));
    c11_vector__ctor(&no_free_arenas, sizeof(PoolArena*));
    int freed = 0;
    for(int i = 0; i < kMultiPoolCount; i++) {
        Pool* item = &self->pools[i];
        freed += Pool__sweep_dealloc(item, &arenas, &no_free_arenas);
    }
    c11_vector__dtor(&arenas);
    c11_vector__dtor(&no_free_arenas);
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

c11_string* MultiPool__summary(MultiPool* self) {
    c11_sbuf sbuf;
    c11_sbuf__ctor(&sbuf);
    for(int i = 0; i < kMultiPoolCount; i++) {
        Pool* item = &self->pools[i];
        int total_bytes = (item->arenas.length + item->no_free_arenas.length) * kPoolArenaSize;
        int used_bytes = 0;
        for(int j = 0; j < item->arenas.length; j++) {
            PoolArena* arena = c11__getitem(PoolArena*, &item->arenas, j);
            used_bytes += (arena->block_count - arena->unused_length) * arena->block_size;
        }
        used_bytes += item->no_free_arenas.length * kPoolArenaSize;
        float used_pct = (float)used_bytes / total_bytes * 100;
        char buf[256];
        snprintf(buf,
                 sizeof(buf),
                 "Pool<%d>: len(arenas)=%d, len(no_free_arenas)=%d, %d/%d (%.1f%% used)",
                 item->block_size,
                 item->arenas.length,
                 item->no_free_arenas.length,
                 used_bytes,
                 total_bytes,
                 used_pct);
        c11_sbuf__write_cstr(&sbuf, buf);
        c11_sbuf__write_char(&sbuf, '\n');
    }
    return c11_sbuf__submit(&sbuf);
}
