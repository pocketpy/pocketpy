#pragma once

#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/objectpool.h"
#include <time.h>

typedef struct ManagedHeap {
    MultiPool small_objects;
    c11_vector /* PyObject_p */ large_objects;
    c11_vector /* PyObject_p */ gc_roots;

    int freed_ma[3];
    int gc_threshold;  // threshold for gc_counter
    int gc_counter;    // objects created since last gc
    bool gc_enabled;
} ManagedHeap;

typedef struct {
    clock_t start;
    clock_t end;
    
    int* small_types;
    int* large_types;
    int small_freed;
    int large_freed;

    struct {
        bool valid;
        int before;
        int after;
        int upper;
        int lower;
        int avg_freed;
        float free_ratio;
    } auto_thres;
} ManagedHeapSwpetInfo;

void ManagedHeap__ctor(ManagedHeap* self);
void ManagedHeap__dtor(ManagedHeap* self);

void ManagedHeapSwpetInfo__ctor(ManagedHeapSwpetInfo* self);
void ManagedHeapSwpetInfo__dtor(ManagedHeapSwpetInfo* self);

void ManagedHeap__collect_if_needed(ManagedHeap* self, ManagedHeapSwpetInfo* out_info);
int ManagedHeap__collect(ManagedHeap* self, ManagedHeapSwpetInfo* out_info);
int ManagedHeap__sweep(ManagedHeap* self, ManagedHeapSwpetInfo* out_info);

#define ManagedHeap__new(self, type, slots, udsize)                                                \
    ManagedHeap__gcnew((self), (type), (slots), (udsize))
PyObject* ManagedHeap__gcnew(ManagedHeap* self, py_Type type, int slots, int udsize);

// external implementation
void ManagedHeap__mark(ManagedHeap* self);
