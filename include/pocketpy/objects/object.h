#pragma once

#include "pocketpy/objects/namedict.h"
#include "pocketpy/objects/base.h"

typedef struct PyObject {
    py_Type type;  // we have a duplicated type here for convenience
    // bool _;
    bool gc_marked;
    int slots;  // number of slots in the object
    char flex[];
} PyObject;

// slots >= 0, allocate N slots
// slots == -1, allocate a dict

// | HEADER | <N slots> | <userdata>
// | HEADER | <dict>    | <userdata>

py_TValue* PyObject__slots(PyObject* self);
NameDict* PyObject__dict(PyObject* self);
void* PyObject__userdata(PyObject* self);

#define PK_OBJ_SLOTS_SIZE(slots) ((slots) >= 0 ? sizeof(py_TValue) * (slots) : sizeof(NameDict))

void PyObject__dtor(PyObject* self);


#define pk__mark_value(val)                                                                        \
    if((val)->is_ptr && !(val)->_obj->gc_marked) {                                                 \
        PyObject* obj = (val)->_obj;                                                               \
        obj->gc_marked = true;                                                                     \
        c11_vector__push(PyObject*, p_stack, obj);                                                 \
    }

