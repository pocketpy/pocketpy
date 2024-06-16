#include "pocketpy/objects/object.h"

void PyVar__ctor2(PyVar* self, PyObject* existing){
    assert(existing);
    self->type = existing->type;
    self->is_ptr = true;
    self->flags = 0;
    self->flags_ex = 0;
    self->_obj = existing;
}
