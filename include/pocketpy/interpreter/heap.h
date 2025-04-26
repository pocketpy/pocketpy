#pragma once

#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/objectpool.h"

typedef struct ManagedHeap {
    MultiPool small_objects;
    c11_vector /* PyObject_p */ large_objects;
    c11_vector /* PyObject_p */ gc_roots;

    int freed_ma[3];
    int gc_threshold;  // threshold for gc_counter
    int gc_counter;    // objects created since last gc
    bool gc_enabled;
} ManagedHeap;

void ManagedHeap__ctor(ManagedHeap* self);
void ManagedHeap__dtor(ManagedHeap* self);

void ManagedHeap__collect_if_needed(ManagedHeap* self);
int ManagedHeap__collect(ManagedHeap* self);
int ManagedHeap__sweep(ManagedHeap* self);

#define ManagedHeap__new(self, type, slots, udsize)                                                \
    ManagedHeap__gcnew((self), (type), (slots), (udsize))
PyObject* ManagedHeap__gcnew(ManagedHeap* self, py_Type type, int slots, int udsize);

// external implementation
void ManagedHeap__mark(ManagedHeap* self);
