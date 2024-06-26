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

const py_Str* py_tostr(py_Ref self){
    return PyObject__value(self->_obj);
}

const char* py_tocstr(py_Ref self){
    const py_Str* s = PyObject__value(self->_obj);
    return py_Str__data(s);
}

void* py_touserdata(py_Ref self){
    assert(self && self->is_ptr);
    return PyObject__value(self->_obj);
}

bool py_istype(const py_Ref self, py_Type type){
    return self->type == type;
}
