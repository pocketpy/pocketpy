#include "pocketpy/interpreter/heap.h"
#include "pocketpy/config.h"
#include "pocketpy/interpreter/objectpool.h"
#include "pocketpy/objects/base.h"
#include "pocketpy/common/sstream.h"
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
    self->debug_callback = *py_None();
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

static void ManagedHeap__fire_debug_callback(ManagedHeap* self, ManagedHeapSwpetInfo* out_info) {
    assert(out_info != NULL);

    c11_sbuf buf;
    c11_sbuf__ctor(&buf);

    const int64_t NANOS_PER_MS = 1000000000 / 1000;
    const char* DIVIDER = "------------------------------------------------------------\n";

    double start = out_info->start_ns / 1e9;
    int64_t mark_ms = (out_info->mark_end_ns - out_info->start_ns) / NANOS_PER_MS;
    int64_t swpet_ms = (out_info->swpet_end_ns - out_info->mark_end_ns) / NANOS_PER_MS;

    c11_sbuf__write_cstr(&buf, DIVIDER);
    pk_sprintf(&buf, "start:        %f\n", (double)start / 1000);
    pk_sprintf(&buf, "mark_ms:      %i\n", (py_i64)mark_ms);
    pk_sprintf(&buf, "swpet_ms:     %i\n", (py_i64)swpet_ms);
    pk_sprintf(&buf, "total_ms:     %i\n", (py_i64)(mark_ms + swpet_ms));
    c11_sbuf__write_cstr(&buf, DIVIDER);
    pk_sprintf(&buf, "types_length: %d\n", out_info->types_length);
    pk_sprintf(&buf, "small_freed:  %d\n", out_info->small_freed);
    pk_sprintf(&buf, "large_freed:  %d\n", out_info->large_freed);
    c11_sbuf__write_cstr(&buf, DIVIDER);

    if(out_info->small_freed != 0 || out_info->large_freed != 0) {
        char line_buf[256];
        for(int i = 0; i < out_info->types_length; i++) {
            const char* type_name = py_tpname(i);
            int s_freed = out_info->small_types[i];
            int l_freed = out_info->large_types[i];
            if(s_freed == 0 && l_freed == 0) continue;
            snprintf(line_buf,
                    sizeof(line_buf),
                    "[%-24s] small: %6d  large: %6d\n",
                    type_name,
                    s_freed,
                    l_freed);
            c11_sbuf__write_cstr(&buf, line_buf);
        }
        c11_sbuf__write_cstr(&buf, DIVIDER);
    }

    pk_sprintf(&buf, "auto_thres.before:        %d\n", out_info->auto_thres.before);
    pk_sprintf(&buf, "auto_thres.after:         %d\n", out_info->auto_thres.after);
    pk_sprintf(&buf, "auto_thres.upper:         %d\n", out_info->auto_thres.upper);
    pk_sprintf(&buf, "auto_thres.lower:         %d\n", out_info->auto_thres.lower);
    pk_sprintf(&buf, "auto_thres.avg_freed:     %d\n", out_info->auto_thres.avg_freed);
    pk_sprintf(&buf, "auto_thres.free_ratio:    %f\n", out_info->auto_thres.free_ratio);
    c11_sbuf__write_cstr(&buf, DIVIDER);

    py_push(&self->debug_callback);
    py_pushnil();
    py_StackRef arg = py_pushtmp();
    c11_sbuf__py_submit(&buf, arg);
    bool ok = py_vectorcall(1, 0);
    if(!ok) {
        char* msg = py_formatexc();
        c11__abort("gc_debug_callback error!!\n%s", msg);
    }
}

void ManagedHeap__collect_hint(ManagedHeap* self) {
    if(self->gc_counter < self->gc_threshold) return;
    self->gc_counter = 0;

    ManagedHeapSwpetInfo* out_info = NULL;
    if(!py_isnone(&self->debug_callback)) out_info = ManagedHeapSwpetInfo__new();
    
    ManagedHeap__mark(self);
    if(out_info) out_info->mark_end_ns = time_ns();
    int freed = ManagedHeap__sweep(self, out_info);
    if(out_info) out_info->swpet_end_ns = time_ns();

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
        out_info->auto_thres.before = self->gc_threshold;
        out_info->auto_thres.after = new_threshold;
        out_info->auto_thres.upper = upper;
        out_info->auto_thres.lower = lower;
        out_info->auto_thres.avg_freed = avg_freed;
        out_info->auto_thres.free_ratio = free_ratio;
    }
    self->gc_threshold = c11__min(c11__max(new_threshold, lower), upper);

    if(!py_isnone(&self->debug_callback)) {
        ManagedHeap__fire_debug_callback(self, out_info);
        ManagedHeapSwpetInfo__delete(out_info);
    }
}

int ManagedHeap__collect(ManagedHeap* self) {
    self->gc_counter = 0;

    ManagedHeapSwpetInfo* out_info = NULL;
    if(!py_isnone(&self->debug_callback)) out_info = ManagedHeapSwpetInfo__new();
    
    ManagedHeap__mark(self);
    if(out_info) out_info->mark_end_ns = time_ns();
    int freed = ManagedHeap__sweep(self, out_info);
    if(out_info) out_info->swpet_end_ns = time_ns();

    if(out_info) {
        out_info->auto_thres.before = self->gc_threshold;
        out_info->auto_thres.after = self->gc_threshold;
    }

    if(!py_isnone(&self->debug_callback)) {
        ManagedHeap__fire_debug_callback(self, out_info);
        ManagedHeapSwpetInfo__delete(out_info);
    }
    return freed;
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
    }
    return small_freed + large_freed;
}

PyObject* ManagedHeap__gcnew(ManagedHeap* self, py_Type type, int slots, int udsize) {
    assert(slots >= 0 || slots == -1);
    // header + slots + udsize
    int size = sizeof(PyObject) + PK_OBJ_SLOTS_SIZE(slots) + udsize;
    PyObject* obj = MultiPool__alloc(&self->small_objects, size);
    if(obj == NULL) {
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