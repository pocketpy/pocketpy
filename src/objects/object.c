#include "pocketpy/objects/object.h"
#include "pocketpy/pocketpy.h"
#include <assert.h>

PK_INLINE void* PyObject__userdata(PyObject* self) {
    return self->flex + PK_OBJ_SLOTS_SIZE(self->slots);
}

PK_INLINE NameDict* PyObject__dict(PyObject* self) {
    assert(self->slots == -1);
    return (NameDict*)(self->flex);
}

PK_INLINE py_TValue* PyObject__slots(PyObject* self) {
    assert(self->slots >= 0);
    return (py_TValue*)(self->flex);
}