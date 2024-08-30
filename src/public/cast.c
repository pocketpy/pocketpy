#include "pocketpy/common/str.h"
#include "pocketpy/objects/base.h"
#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/vm.h"

int64_t py_toint(py_Ref self) {
    assert(self->type == tp_int);
    return self->_i64;
}

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

bool py_castfloat32(py_Ref self, float *out){
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
    py_Type* ud = py_touserdata(self);
    return *ud;
}

void* py_touserdata(py_Ref self) {
    assert(self && self->is_ptr);
    return PyObject__userdata(self->_obj);
}

bool py_istype(py_Ref self, py_Type type) { return self->type == type; }

bool py_checktype(py_Ref self, py_Type type) {
    if(self->type == type) return true;
    return TypeError("expected '%t', got '%t'", type, self->type);
}

bool py_isinstance(py_Ref obj, py_Type type) { return py_issubclass(obj->type, type); }

bool py_issubclass(py_Type derived, py_Type base) {
    TypeList* types = &pk_current_vm->types;
    do {
        if(derived == base) return true;
        derived = TypeList__get(types, derived)->base;
    } while(derived);
    return false;
}

py_Type py_typeof(py_Ref self) { return self->type; }

py_Type py_gettype(const char* module, py_Name name) {
    py_Ref mod = py_getmodule(module);
    if(!mod) return 0;
    py_Ref object = py_getdict(mod, name);
    if(object && py_istype(object, tp_type)) return py_totype(object);
    return 0;
}