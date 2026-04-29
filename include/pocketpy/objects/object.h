#pragma once

#include "pocketpy/objects/namedict.h"
#include "pocketpy/objects/base.h"

typedef struct PyObject {
    py_Type type;  // we have a duplicated type here for convenience
    uint8_t size_8b;
    uint8_t gc_marked;  // lsb (self is marked), 2nd lsb (no recursively mark)
    int slots;          // number of slots in the object
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
    if((val)->is_ptr) {                                                                            \
        PyObject* obj = (val)->_obj;                                                               \
        if(!(obj->gc_marked & 0b01)) {                                                             \
            obj->gc_marked |= 0b01;                                                                \
            if(!(obj->gc_marked & 0b10)) { c11_vector__push(PyObject*, p_stack, obj); }            \
        }                                                                                          \
    }
