#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/vm.h"

int64_t py_toint(py_Ref self){
    return self->_i64;
}

double py_tofloat(py_Ref self){
    return self->_f64;
}

bool py_castfloat(py_Ref self, double* out){
    switch(self->type){
        case tp_int:
            *out = (double)self->_i64;
            return true;
        case tp_float:
            *out = self->_f64;
            return true;
        case tp_bool:
            *out = self->extra;
            return true;
        default:
            return false;
    }
}

bool py_tobool(py_Ref self){
    return self->extra;
}

const char* py_tostr(py_Ref self){
    py_Str* ud = PyObject__value(self->_obj);
    return py_Str__data(ud);
}

const char* py_tostrn(py_Ref self, int* out){
    py_Str* ud = PyObject__value(self->_obj);
    *out = ud->size;
    return py_Str__data(ud);
}

void* py_touserdata(py_Ref self){
    assert(self && self->is_ptr);
    return PyObject__value(self->_obj);
}

bool py_istype(const py_Ref self, py_Type type){
    return self->type == type;
}
