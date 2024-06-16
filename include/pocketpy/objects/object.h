#pragma once

#include "pocketpy/objects/namedict.h"
#include "pocketpy/objects/base.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PyObject{
    pkpy_Type type;         // we have a duplicated type here for convenience
    bool gc_is_large;
    bool gc_marked;
    pkpy_NameDict* _attr;   // gc will delete this on destruction
} PyObject;

static_assert(sizeof(PyObject) <= 16, "!(sizeof(PyObject) <= 16)");

#define PyObject__value_ptr(self)   ((char*)self + 16)
#define PyObject__as(T, self)       (T*)(PyObject__value_ptr(self))

PK_INLINE void PyObject__ctor(PyObject* self, pkpy_Type type, bool gc_is_large){
    self->type = type;
    self->gc_is_large = gc_is_large;
    self->gc_marked = false;
    self->_attr = NULL;
}

#ifdef __cplusplus
}
#endif