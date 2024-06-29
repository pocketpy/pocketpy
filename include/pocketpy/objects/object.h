#pragma once

#include "pocketpy/objects/namedict.h"
#include "pocketpy/objects/base.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PyObject{
    py_Type type;          // we have a duplicated type here for convenience
    bool gc_is_large;
    bool gc_marked;
    int slots;          // number of slots in the object
} PyObject;

// slots >= 0, allocate N slots
// slots == -1, allocate a dict

// | 8 bytes HEADER | <N slots> | <value>
// | 8 bytes HEADER | <dict>    | <value>

static_assert(sizeof(PyObject) <= 8, "!(sizeof(PyObject) <= 8)");

py_TValue* PyObject__slots(PyObject* self);
pk_NameDict* PyObject__dict(PyObject* self);
void* PyObject__value(PyObject* self);

#define PK_OBJ_HEADER_SIZE(slots) ((slots)>=0 ? 8+sizeof(py_TValue)*(slots) : 8+sizeof(pk_NameDict))

PyObject* PyObject__new(py_Type type, int slots, int size);
void PyObject__delete(PyObject* self);

PK_INLINE py_TValue PyVar__fromobj(PyObject* obj){
    if(!obj) return PY_NULL;
    py_TValue retval = {
        .type = obj->type,
        .is_ptr = true,
        ._obj = obj
    };
    return retval;
}

#ifdef __cplusplus
}
#endif