#include "pocketpy/interpreter/heap.h"
#include "pocketpy/common/memorypool.h"
#include "pocketpy/config.h"
#include "pocketpy/objects/base.h"

void ManagedHeap__ctor(ManagedHeap* self, VM* vm) {
    c11_vector__ctor(&self->no_gc, sizeof(PyObject*));
    c11_vector__ctor(&self->gen, sizeof(PyObject*));

    self->gc_threshold = PK_GC_MIN_THRESHOLD;
    self->gc_counter = 0;
    self->gc_enabled = true;

    self->vm = vm;

    self->gc_on_delete = NULL;
}

void ManagedHeap__dtor(ManagedHeap* self) {
    for(int i = 0; i < self->gen.length; i++) {
        PyObject* obj = c11__getitem(PyObject*, &self->gen, i);
        PyObject__delete(obj);
    }
    for(int i = 0; i < self->no_gc.length; i++) {
        PyObject* obj = c11__getitem(PyObject*, &self->no_gc, i);
        PyObject__delete(obj);
    }
    c11_vector__dtor(&self->no_gc);
    c11_vector__dtor(&self->gen);
}

void ManagedHeap__collect_if_needed(ManagedHeap* self) {
    if(!self->gc_enabled) return;
    if(self->gc_counter < self->gc_threshold) return;
    self->gc_counter = 0;
    ManagedHeap__collect(self);
    self->gc_threshold = self->gen.length * 2;
    if(self->gc_threshold < PK_GC_MIN_THRESHOLD) { self->gc_threshold = PK_GC_MIN_THRESHOLD; }
}

int ManagedHeap__collect(ManagedHeap* self) {
    ManagedHeap__mark(self);
    int freed = ManagedHeap__sweep(self);
    return freed;
}

int ManagedHeap__sweep(ManagedHeap* self) {
    c11_vector alive;
    c11_vector__ctor(&alive, sizeof(PyObject*));
    c11_vector__reserve(&alive, self->gen.length / 2);

    for(int i = 0; i < self->gen.length; i++) {
        PyObject* obj = c11__getitem(PyObject*, &self->gen, i);
        if(obj->gc_marked) {
            obj->gc_marked = false;
            c11_vector__push(PyObject*, &alive, obj);
        } else {
            if(self->gc_on_delete) { self->gc_on_delete(self->vm, obj); }
            PyObject__delete(obj);
        }
    }

    // clear _no_gc marked flag
    for(int i = 0; i < self->no_gc.length; i++) {
        PyObject* obj = c11__getitem(PyObject*, &self->no_gc, i);
        obj->gc_marked = false;
    }

    int freed = self->gen.length - alive.length;

    // destroy old gen
    c11_vector__dtor(&self->gen);
    // move alive to gen
    self->gen = alive;

    PoolObject_shrink_to_fit();
    return freed;
}

PyObject* ManagedHeap__new(ManagedHeap* self, py_Type type, int slots, int udsize) {
    PyObject* obj = PyObject__new(type, slots, udsize);
    c11_vector__push(PyObject*, &self->no_gc, obj);
    return obj;
}

PyObject* ManagedHeap__gcnew(ManagedHeap* self, py_Type type, int slots, int udsize) {
    PyObject* obj = PyObject__new(type, slots, udsize);
    c11_vector__push(PyObject*, &self->gen, obj);
    self->gc_counter++;
    return obj;
}

PyObject* PyObject__new(py_Type type, int slots, int size) {
    assert(slots >= 0 || slots == -1);
    PyObject* self;
    // header + slots + udsize
    size = sizeof(PyObject) + PK_OBJ_SLOTS_SIZE(slots) + size;
    if(!PK_LOW_MEMORY_MODE && size <= kPoolObjectBlockSize) {
        self = PoolObject_alloc();
        self->gc_is_large = false;
    } else {
        self = PK_MALLOC(size);
        self->gc_is_large = true;
    }
    self->type = type;
    self->gc_marked = false;
    self->slots = slots;

    // initialize slots or dict
    if(slots >= 0) {
        memset(self->flex, 0, slots * sizeof(py_TValue));
    } else {
        NameDict__ctor((void*)self->flex);
    }
    return self;
}
