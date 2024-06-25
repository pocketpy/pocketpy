#include "pocketpy/objects/object.h"
#include "pocketpy/pocketpy.h"
#include <assert.h>

void* PyObject__value(PyObject* self){
    return (char*)self + PK_OBJ_HEADER_SIZE(self->slots);
}

pk_NameDict* PyObject__dict(PyObject* self){
    assert(self->slots == -1);
    return (pk_NameDict*)((char*)self + 8);
}

PyVar* PyObject__slots(PyObject* self){
    assert(self->slots >= 0);
    return (PyVar*)((char*)self + 8);
}