#include "pocketpy/pocketpy.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/objects/iterator.h"
#include "pocketpy/interpreter/vm.h"

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

bool list_iterator__next__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    list_iterator* ud = py_touserdata(argv);
    if(ud->index < ud->vec->length) {
        py_TValue* res = c11__at(py_TValue, ud->vec, ud->index);
        py_assign(py_retval(), res);
        ud->index++;
        return true;
    }
    return StopIteration();
}

bool tuple_iterator__next__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    tuple_iterator* ud = py_touserdata(argv);
    if(ud->index < ud->length) {
        py_assign(py_retval(), ud->p + ud->index);
        ud->index++;
        return true;
    }
    return StopIteration();
}

py_Type pk_list_iterator__register() {
    py_Type type = pk_newtype("list_iterator", tp_object, NULL, NULL, false, true);
    py_bindmagic(type, __iter__, pk_wrapper__self);
    py_bindmagic(type, __next__, list_iterator__next__);
    return type;
}

py_Type pk_tuple_iterator__register() {
    py_Type type = pk_newtype("tuple_iterator", tp_object, NULL, NULL, false, true);
    py_bindmagic(type, __iter__, pk_wrapper__self);
    py_bindmagic(type, __next__, tuple_iterator__next__);
    return type;
}
