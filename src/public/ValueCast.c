#include "pocketpy/objects/base.h"
#include "pocketpy/pocketpy.h"

#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/vm.h"

py_i64 py_toint(py_Ref self) {
    assert(self->type == tp_int);
    return self->_i64;
}

void* py_totrivial(py_Ref self) { return &self->_chars; }

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

const char* py_tostr(py_Ref self) { return pk_tostr(self)->data; }

const char* py_tostrn(py_Ref self, int* size) {
    c11_string* ud = pk_tostr(self);
    *size = ud->size;
    return ud->data;
}

c11_sv py_tosv(py_Ref self) {
    c11_string* ud = pk_tostr(self);
    return c11_string__sv(ud);
}

unsigned char* py_tobytes(py_Ref self, int* size) {
    assert(self->type == tp_bytes);
    c11_bytes* ud = PyObject__userdata(self->_obj);
    *size = ud->size;
    return ud->data;
}

void py_bytes_resize(py_Ref self, int size) {
    assert(self->type == tp_bytes);
    c11_bytes* ud = PyObject__userdata(self->_obj);
    if(size > ud->size) c11__abort("bytes can only be resized down: %d > %d", ud->size, size);
    ud->size = size;
}
