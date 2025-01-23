#include "pocketpy/interpreter/heap.h"
#include "pocketpy/config.h"
#include "pocketpy/interpreter/objectpool.h"
#include "pocketpy/objects/base.h"
#include "pocketpy/pocketpy.h"

void ManagedHeap__ctor(ManagedHeap* self) {
    MultiPool__ctor(&self->small_objects);
    c11_vector__ctor(&self->large_objects, sizeof(PyObject*));

    for(int i = 0; i < c11__count_array(self->freed_ma); i++) {
        self->freed_ma[i] = PK_GC_MIN_THRESHOLD;
    }
    self->gc_threshold = PK_GC_MIN_THRESHOLD;
    self->gc_counter = 0;
    self->gc_enabled = true;
}

void ManagedHeap__dtor(ManagedHeap* self) {
    // small_objects
    MultiPool__dtor(&self->small_objects);
    // large_objects
    for(int i = 0; i < self->large_objects.length; i++) {
        PyObject* obj = c11__getitem(PyObject*, &self->large_objects, i);
        PyObject__dtor(obj);
        PK_FREE(obj);
    }
    c11_vector__dtor(&self->large_objects);
}

void ManagedHeap__collect_if_needed(ManagedHeap* self) {
    if(!self->gc_enabled) return;
    if(self->gc_counter < self->gc_threshold) return;
    self->gc_counter = 0;
    int freed = ManagedHeap__collect(self);
    // adjust `gc_threshold` based on `freed_ma`
    self->freed_ma[0] = self->freed_ma[1];
    self->freed_ma[1] = self->freed_ma[2];
    self->freed_ma[2] = freed;
    int avg_freed = (self->freed_ma[0] + self->freed_ma[1] + self->freed_ma[2]) / 3;
    int upper = self->gc_threshold * 2;
    int lower = c11__max(PK_GC_MIN_THRESHOLD, self->gc_threshold / 2 + 1);
    self->gc_threshold = c11__min(c11__max(avg_freed, lower), upper);
}

int ManagedHeap__collect(ManagedHeap* self) {
    ManagedHeap__mark(self);
    int freed = ManagedHeap__sweep(self);
    return freed;
}

int ManagedHeap__sweep(ManagedHeap* self) {
    // small_objects
    int small_freed = MultiPool__sweep_dealloc(&self->small_objects);
    // large_objects
    int large_living_count = 0;
    for(int i = 0; i < self->large_objects.length; i++) {
        PyObject* obj = c11__getitem(PyObject*, &self->large_objects, i);
        if(obj->gc_marked) {
            obj->gc_marked = false;
            c11__setitem(PyObject*, &self->large_objects, large_living_count, obj);
            large_living_count++;
        } else {
            PyObject__dtor(obj);
            PK_FREE(obj);
        }
    }
    // shrink `self->large_objects`
    int large_freed = self->large_objects.length - large_living_count;
    self->large_objects.length = large_living_count;
    // printf("large_freed=%d\n", large_freed);
    // printf("small_freed=%d\n", small_freed);
    return small_freed + large_freed;
}

PyObject* ManagedHeap__gcnew(ManagedHeap* self, py_Type type, int slots, int udsize) {
    assert(slots >= 0 || slots == -1);
    PyObject* obj;
    // header + slots + udsize
    int size = sizeof(PyObject) + PK_OBJ_SLOTS_SIZE(slots) + udsize;
    if(!PK_LOW_MEMORY_MODE && size <= kPoolMaxBlockSize) {
        obj = MultiPool__alloc(&self->small_objects, size);
        assert(obj != NULL);
    } else {
        obj = PK_MALLOC(size);
        c11_vector__push(PyObject*, &self->large_objects, obj);
    }
    obj->type = type;
    obj->gc_marked = false;
    obj->slots = slots;

    // initialize slots or dict
    if(slots >= 0) {
        memset(obj->flex, 0, slots * sizeof(py_TValue));
    } else {
        NameDict__ctor((void*)obj->flex);
    }

    self->gc_counter++;
    return obj;
}