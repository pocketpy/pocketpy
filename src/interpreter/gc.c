#include "pocketpy/interpreter/gc.h"
#include "pocketpy/common/memorypool.h"

void pk_ManagedHeap__ctor(pk_ManagedHeap *self, pk_VM *vm){
    c11_vector__ctor(&self->no_gc, sizeof(PyObject*));
    c11_vector__ctor(&self->gen, sizeof(PyObject*));

    self->gc_threshold = PK_GC_MIN_THRESHOLD;
    self->gc_counter = 0;
    self->gc_lock_counter = 0;
    self->vm = vm;

    self->_gc_on_delete = NULL;
    self->_gc_marker_ex = NULL;
}

void pk_ManagedHeap__dtor(pk_ManagedHeap *self){
    for(int i = 0; i < self->gen.count; i++){
        PyObject* obj = c11__getitem(PyObject*, &self->gen, i);
        PyObject__delete(obj);
    }
    for(int i = 0; i < self->no_gc.count; i++){
        PyObject* obj = c11__getitem(PyObject*, &self->no_gc, i);
        PyObject__delete(obj);
    }
    c11_vector__dtor(&self->no_gc);
    c11_vector__dtor(&self->gen);
}

void pk_ManagedHeap__push_lock(pk_ManagedHeap *self){
    self->gc_lock_counter++;
}

void pk_ManagedHeap__pop_lock(pk_ManagedHeap *self){
    self->gc_lock_counter--;
}

void pk_ManagedHeap__collect_if_needed(pk_ManagedHeap *self){
    if(self->gc_counter < self->gc_threshold) return;
    if(self->gc_lock_counter > 0) return;
    self->gc_counter = 0;
    pk_ManagedHeap__collect(self);
    self->gc_threshold = self->gen.count * 2;
    if(self->gc_threshold < PK_GC_MIN_THRESHOLD){
        self->gc_threshold = PK_GC_MIN_THRESHOLD;
    }
}

int pk_ManagedHeap__collect(pk_ManagedHeap *self){
    assert(self->gc_lock_counter == 0);
    pk_ManagedHeap__mark(self);
    int freed = pk_ManagedHeap__sweep(self);
    return freed;
}

int pk_ManagedHeap__sweep(pk_ManagedHeap *self){
    c11_vector alive;
    c11_vector__ctor(&alive, sizeof(PyObject*));
    c11_vector__reserve(&alive, self->gen.count / 2);

    for(int i = 0; i < self->gen.count; i++){
        PyObject* obj = c11__getitem(PyObject*, &self->gen, i);
        if(obj->gc_marked) {
            obj->gc_marked = false;
            c11_vector__push(PyObject*, &alive, obj);
        } else {
            if(self->_gc_on_delete){
                self->_gc_on_delete(self->vm, obj);
            }
            PyObject__delete(obj);
        }
    }

    // clear _no_gc marked flag
    for(int i=0; i<self->no_gc.count; i++){
        PyObject* obj = c11__getitem(PyObject*, &self->no_gc, i);
        obj->gc_marked = false;
    }

    int freed = self->gen.count - alive.count;

    // destroy old gen
    c11_vector__dtor(&self->gen);
    // move alive to gen
    self->gen = alive;

    PoolObject_shrink_to_fit();
    return freed;
}

PyObject* pk_ManagedHeap__new(pk_ManagedHeap *self, Type type, int size){
    PyObject* obj = PyObject__new(type, size);
    c11_vector__push(PyObject*, &self->no_gc, obj);
    return obj;
}

PyObject* pk_ManagedHeap__gcnew(pk_ManagedHeap *self, Type type, int size){
    PyObject* obj = PyObject__new(type, size);
    c11_vector__push(PyObject*, &self->gen, obj);
    self->gc_counter++;
    return obj;
}

PyObject* PyObject__new(Type type, int size){
    PyObject* self;
    if(size <= kPoolObjectBlockSize){
        self = PoolObject_alloc();
        self->gc_is_large = false;
    }else{
        self = malloc(size);
        self->gc_is_large = true;
    }
    self->type = type;
    self->gc_marked = false;
    self->dict = NULL;
    return self;
}
