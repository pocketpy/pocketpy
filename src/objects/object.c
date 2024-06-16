#include "pocketpy/objects/object.h"

void PyVar__ctor3(PyVar* self, PyObject* existing){
    assert(existing);
    self->type = existing->type;
    self->is_ptr = true;
    self->_obj = existing;
}

static PyObject __true_obj = {.type=tp_bool, .gc_is_large=false, .gc_marked=false, ._attr=NULL};
static PyObject __false_obj = {.type=tp_bool, .gc_is_large=false, .gc_marked=false, ._attr=NULL};
static PyObject __none_obj = {.type=tp_none_type, .gc_is_large=false, .gc_marked=false, ._attr=NULL};
static PyObject __not_implemented_obj = {.type=tp_not_implemented_type, .gc_is_large=false, .gc_marked=false, ._attr=NULL};
static PyObject __ellipsis_obj = {.type=tp_ellipsis, .gc_is_large=false, .gc_marked=false, ._attr=NULL};

/* Must be heap objects to support `==` and `is` and `is not` */
PyVar pkpy_True = {.type=tp_bool, .is_ptr=true, .extra=1, ._obj=&__true_obj};
PyVar pkpy_False = {.type=tp_bool, .is_ptr=true, .extra=0, ._obj=&__false_obj};
PyVar pkpy_None = {.type=tp_none_type, .is_ptr=true, ._obj=&__none_obj};
PyVar pkpy_NotImplemented = {.type=tp_not_implemented_type, .is_ptr=true, ._obj=&__not_implemented_obj};
PyVar pkpy_Ellipsis = {.type=tp_ellipsis, .is_ptr=true, ._obj=&__ellipsis_obj};