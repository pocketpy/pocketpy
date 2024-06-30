#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/vm.h"

int64_t py_toint(const py_Ref self){
    assert(self->type == tp_int);
    return self->_i64;
}

double py_tofloat(const py_Ref self){
    assert(self->type == tp_float);
    return self->_f64;
}

bool py_castfloat(const py_Ref self, double* out){
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

bool py_tobool(const py_Ref self){
    assert(self->type == tp_bool);
    return self->extra;
}

const char* py_tostr(const py_Ref self){
    assert(self->type == tp_str);
    py_Str* ud = PyObject__value(self->_obj);
    return py_Str__data(ud);
}

const char* py_tostrn(const py_Ref self, int* out){
    assert(self->type == tp_str);
    py_Str* ud = PyObject__value(self->_obj);
    *out = ud->size;
    return py_Str__data(ud);
}

void* py_touserdata(const py_Ref self){
    assert(self && self->is_ptr);
    return PyObject__value(self->_obj);
}

bool py_istype(const py_Ref self, py_Type type){
    return self->type == type;
}
