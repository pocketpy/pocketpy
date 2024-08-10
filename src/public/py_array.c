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

int pk_arrayview(py_Ref self, py_TValue** p) {
    if(self->type == tp_list) {
        *p = py_list_data(self);
        return py_list_len(self);
    }
    if(self->type == tp_tuple) {
        *p = PyObject__slots(self->_obj);
        return py_tuple_len(self);
    }
    return -1;
}

bool pk_wrapper__arrayequal(py_Type type, int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    if(!py_istype(py_arg(1), type)) {
        py_newnotimplemented(py_retval());
        return true;
    }
    py_TValue *p0, *p1;
    int lhs_length = pk_arrayview(py_arg(0), &p0);
    int rhs_length = pk_arrayview(py_arg(1), &p1);
    assert(lhs_length != -1 && rhs_length != -1);
    if(lhs_length != rhs_length) {
        py_newbool(py_retval(), false);
        return true;
    }
    for(int i = 0; i < lhs_length; i++) {
        int res = py_equal(p0 + i, p1 + i);
        if(res == -1) return false;
        if(!res) {
            py_newbool(py_retval(), false);
            return true;
        }
    }
    py_newbool(py_retval(), true);
    return true;
}

bool pk_arrayiter(py_Ref val) {
    py_TValue* p;
    int length = pk_arrayview(val, &p);
    if(length == -1) return TypeError("expected list or tuple, got %t", val->type);
    array_iterator* ud = py_newobject(py_retval(), tp_array_iterator, 1, sizeof(array_iterator));
    ud->p = p;
    ud->length = length;
    ud->index = 0;
    py_setslot(py_retval(), 0, val);  // keep a reference to the object
    return true;
}

bool pk_arraycontains(py_Ref self, py_Ref val) {
    py_TValue* p;
    int length = pk_arrayview(self, &p);
    if(length == -1) return TypeError("expected list or tuple, got %t", self->type);
    for(int i = 0; i < length; i++) {
        int res = py_equal(p + i, val);
        if(res == -1) return false;
        if(res) {
            py_newbool(py_retval(), true);
            return true;
        }
    }
    py_newbool(py_retval(), false);
    return true;
}

static bool array_iterator__iter__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    *py_retval() = *argv;
    return true;
}

static bool array_iterator__next__(int argc, py_Ref argv) {
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
    py_bindmagic(type, __iter__, array_iterator__iter__);
    py_bindmagic(type, __next__, array_iterator__next__);
    return type;
}
