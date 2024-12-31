#include "pocketpy/objects/object.h"

typedef struct ManagedHeap{
    c11_vector no_gc;
    c11_vector gen;

    int gc_threshold;
    int gc_counter;
    bool gc_enabled;
    
    VM* vm;

    void (*gc_on_delete)(VM*, PyObject*);
} ManagedHeap;

void ManagedHeap__ctor(ManagedHeap* self, VM* vm);
void ManagedHeap__dtor(ManagedHeap* self);

void ManagedHeap__collect_if_needed(ManagedHeap* self);
int ManagedHeap__collect(ManagedHeap* self);
int ManagedHeap__sweep(ManagedHeap* self);

PyObject* ManagedHeap__new(ManagedHeap* self, py_Type type, int slots, int udsize);
PyObject* ManagedHeap__gcnew(ManagedHeap* self, py_Type type, int slots, int udsize);

// external implementation
void ManagedHeap__mark(ManagedHeap* self);
