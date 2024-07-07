#include "pocketpy/common/str.h"
#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/vm.h"

int64_t py_toint(const py_Ref self) {
    assert(self->type == tp_int);
    return self->_i64;
}

double py_tofloat(const py_Ref self) {
    assert(self->type == tp_float);
    return self->_f64;
}

bool py_castfloat(const py_Ref self, double* out) {
    switch(self->type) {
        case tp_int: *out = (double)self->_i64; return true;
        case tp_float: *out = self->_f64; return true;
        default: return false;
    }
}

bool py_tobool(const py_Ref self) {
    assert(self->type == tp_bool);
    return self->_bool;
}

py_Type py_totype(const py_Ref self) {
    assert(self->type == tp_type);
    py_Type* ud = py_touserdata(self);
    return *ud;
}

void* py_touserdata(const py_Ref self) {
    assert(self && self->is_ptr);
    return PyObject__userdata(self->_obj);
}

bool py_istype(const py_Ref self, py_Type type) { return self->type == type; }

bool py_checktype(const py_Ref self, py_Type type) {
    if(self->type != type) {
        return TypeError("expected %t, got %t", type, self->type);
    }
    return true;
}