#include "pocketpy/objects/base.h"
#include "pocketpy/pocketpy.h"

#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/vm.h"

int64_t py_toint(py_Ref self) {
    assert(self->type == tp_int);
    return self->_i64;
}

py_i64 py_totrivial(py_Ref self) { return self->_i64; }

double py_tofloat(py_Ref self) {
    assert(self->type == tp_float);
    return self->_f64;
}

bool py_castfloat(py_Ref self, double* out) {
    switch(self->type) {
        case tp_int: *out = (double)self->_i64; return true;
        case tp_float: *out = self->_f64; return true;
        default: return TypeError("expected 'int' or 'float', got '%t'", self->type);
    }
}

bool py_castfloat32(py_Ref self, float* out) {
    switch(self->type) {
        case tp_int: *out = (float)self->_i64; return true;
        case tp_float: *out = (float)self->_f64; return true;
        default: return TypeError("expected 'int' or 'float', got '%t'", self->type);
    }
}

bool py_castint(py_Ref self, int64_t* out) {
    if(self->type == tp_int) {
        *out = self->_i64;
        return true;
    }
    return TypeError("expected 'int', got '%t'", self->type);
}

bool py_tobool(py_Ref self) {
    assert(self->type == tp_bool);
    return self->_bool;
}

py_Type py_totype(py_Ref self) {
    assert(self->type == tp_type);
    py_TypeInfo* ud = py_touserdata(self);
    return ud->index;
}

void* py_touserdata(py_Ref self) {
    assert(self && self->is_ptr);
    return PyObject__userdata(self->_obj);
}
