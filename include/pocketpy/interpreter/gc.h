#include "pocketpy/objects/object.h"
#include "pocketpy/objects/public.h"
#include "pocketpy/common/config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct pk_ManagedHeap{
    c11_vector no_gc;
    c11_vector gen;

    int gc_threshold;
    int gc_counter;
    int gc_lock_counter;
    pkpy_VM* vm;

    void (*_gc_on_delete)(pkpy_VM*, PyObject*);
    void (*_gc_marker_ex)(pkpy_VM*);
} pk_ManagedHeap;

void pk_ManagedHeap__ctor(pk_ManagedHeap* self, pkpy_VM* vm);
void pk_ManagedHeap__dtor(pk_ManagedHeap* self);

void pk_ManagedHeap__push_lock(pk_ManagedHeap* self);
void pk_ManagedHeap__pop_lock(pk_ManagedHeap* self);

void pk_ManagedHeap__collect_if_needed(pk_ManagedHeap* self);
int pk_ManagedHeap__collect(pk_ManagedHeap* self);
int pk_ManagedHeap__sweep(pk_ManagedHeap* self);

PyObject* pk_ManagedHeap__new(pk_ManagedHeap* self, pkpy_Type type, int size, bool gc);

// external implementation
void pk_ManagedHeap__mark(pk_ManagedHeap* self);
void pk_ManagedHeap__delete_obj(pk_ManagedHeap* self, PyObject* obj);

#ifdef __cplusplus
}
#endif