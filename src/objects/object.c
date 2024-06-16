#include "pocketpy/objects/object.h"

void PyVar__ctor3(PyVar* self, PyObject* existing){
    assert(existing);
    self->type = existing->type;
    self->is_ptr = true;
    self->_obj = existing;
}
