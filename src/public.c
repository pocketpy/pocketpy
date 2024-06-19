#include "pocketpy/objects/public.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/vm.h"

void py_initialize(){
    // initialize the global VM
}

void py_newint(PyVar* self, int64_t val){
    self->type = tp_int;
    self->is_ptr = false;
    self->_i64 = val;
}

void py_newfloat(PyVar* self, double val){
    self->type = tp_float;
    self->is_ptr = false;
    self->_f64 = val;
}

void py_newbool(PyVar* self, bool val){
    // return a global singleton
}

void py_newnone(PyVar* self){
    // return a heap object
}

void py_newstr(PyVar* self, const char* val){
    // return a heap object
}

void py_newstr2(PyVar*, const char*, int);

void py_newbytes(PyVar*, const uint8_t*, int);

