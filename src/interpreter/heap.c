#include "pocketpy/interpreter/heap.h"
#include "pocketpy/config.h"
#include "pocketpy/interpreter/objectpool.h"
#include "pocketpy/objects/base.h"
#include "pocketpy/pocketpy.h"
#include <assert.h>

void ManagedHeap__ctor(ManagedHeap* self) {
    MultiPool__ctor(&self->small_objects);
    c11_vector__ctor(&self->large_objects, sizeof(PyObject*));
    c11_vector__ctor(&self->gc_roots, sizeof(PyObject*));

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
    c11_vector__dtor(&self->gc_roots);
}

void ManagedHeap__collect_if_needed(ManagedHeap* self, ManagedHeapSwpetInfo* out_info) {
    if(!self->gc_enabled) return;
    if(self->gc_counter < self->gc_threshold) return;
    int freed = ManagedHeap__collect(self, out_info);
    // adjust `gc_threshold` based on `freed_ma`
    self->freed_ma[0] = self->freed_ma[1];
    self->freed_ma[1] = self->freed_ma[2];
    self->freed_ma[2] = freed;
    int avg_freed = (self->freed_ma[0] + self->freed_ma[1] + self->freed_ma[2]) / 3;
    const int upper = PK_GC_MIN_THRESHOLD * 16;
    const int lower = PK_GC_MIN_THRESHOLD / 2;
    float free_ratio = (float)avg_freed / self->gc_threshold;
    int new_threshold = self->gc_threshold * (1.5f / free_ratio);
    if(out_info) {
        out_info->auto_thres.valid = true;
        out_info->auto_thres.before = self->gc_threshold;
        out_info->auto_thres.after = new_threshold;
        out_info->auto_thres.upper = upper;
        out_info->auto_thres.lower = lower;
        out_info->auto_thres.avg_freed = avg_freed;
        out_info->auto_thres.free_ratio = free_ratio;
    }
    self->gc_threshold = c11__min(c11__max(new_threshold, lower), upper);
}

int ManagedHeap__collect(ManagedHeap* self, ManagedHeapSwpetInfo* out_info) {
    self->gc_counter = 0;
    ManagedHeap__mark(self);
    return ManagedHeap__sweep(self, out_info);
}

int ManagedHeap__sweep(ManagedHeap* self, ManagedHeapSwpetInfo* out_info) {
    // small_objects
    int small_freed =
        MultiPool__sweep_dealloc(&self->small_objects, out_info ? out_info->small_types : NULL);
    // large_objects
    int large_living_count = 0;
    for(int i = 0; i < self->large_objects.length; i++) {
        PyObject* obj = c11__getitem(PyObject*, &self->large_objects, i);
        if(obj->gc_marked) {
            obj->gc_marked = false;
            c11__setitem(PyObject*, &self->large_objects, large_living_count, obj);
            large_living_count++;
        } else {
            if(out_info) out_info->large_types[obj->type]++;
            PyObject__dtor(obj);
            PK_FREE(obj);
        }
    }
    // shrink `self->large_objects`
    int large_freed = self->large_objects.length - large_living_count;
    self->large_objects.length = large_living_count;
    if(out_info) {
        out_info->small_freed = small_freed;
        out_info->large_freed = large_freed;
        out_info->end = clock();
    }
    return small_freed + large_freed;
}

PyObject* ManagedHeap__gcnew(ManagedHeap* self, py_Type type, int slots, int udsize) {
    assert(slots >= 0 || slots == -1);
    PyObject* obj;
    // header + slots + udsize
    int size = sizeof(PyObject) + PK_OBJ_SLOTS_SIZE(slots) + udsize;
    if(size <= kPoolMaxBlockSize) {
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
        float load_factor = (type == tp_type || type == tp_module) ? PK_TYPE_ATTR_LOAD_FACTOR
                                                                   : PK_INST_ATTR_LOAD_FACTOR;
        NameDict__ctor((void*)obj->flex, load_factor);
    }

    self->gc_counter++;
    return obj;
}