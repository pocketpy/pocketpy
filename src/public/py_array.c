#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/vm.h"
#include "pocketpy/common/sstream.h"

typedef struct array_iterator {
    py_TValue* p;
    int length;
    int index;
} array_iterator;

py_TValue* pk_arrayview(py_Ref self, int* length) {
    if(self->type == tp_list) {
        *length = py_list__len(self);
        return py_list__data(self);
    }
    if(self->type == tp_tuple) {
        *length = py_tuple__len(self);
        return PyObject__slots(self->_obj);
    }
    return NULL;
}

int pk_arrayeq(py_TValue* lhs, int lhs_length, py_TValue* rhs, int rhs_length) {
    if(lhs_length != rhs_length) return false;
    for(int i = 0; i < lhs_length; i++) {
        int res = py_eq(lhs + i, rhs + i);
        if(res == -1) return -1;
        if(!res) return false;
    }
    return true;
}

static bool _py_array_iterator__new__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    int length;
    py_TValue* p = pk_arrayview(py_arg(1), &length);
    if(!p) return TypeError("expected list or tuple, got %t", py_arg(1)->type);
    array_iterator* ud = py_newobject(py_retval(), tp_array_iterator, 1, sizeof(array_iterator));
    ud->p = p;
    ud->length = length;
    ud->index = 0;
    // keep a reference to the object
    py_setslot(py_retval(), 0, py_arg(1));
    return true;
}

static bool _py_array_iterator__iter__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    *py_retval() = *argv;
    return true;
}

static bool _py_array_iterator__next__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    array_iterator* ud = py_touserdata(argv);
    if(ud->index < ud->length) {
        *py_retval() = ud->p[ud->index++];
        return true;
    }
    return StopIteration();
}

py_Type pk_array_iterator__register() {
    py_Type type = pk_newtype("array_iterator", tp_object, NULL, NULL, false, true);

    py_bindmagic(type, __new__, _py_array_iterator__new__);
    py_bindmagic(type, __iter__, _py_array_iterator__iter__);
    py_bindmagic(type, __next__, _py_array_iterator__next__);

    return type;
}