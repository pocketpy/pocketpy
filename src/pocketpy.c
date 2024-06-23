#include "pocketpy/pocketpy.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/vm.h"

pk_VM* pk_vm;
static pk_VM pk_default_vm;

void py_initialize(){
    Pools_initialize();
    pk_StrName__initialize();
    pk_vm = &pk_default_vm;
    pk_VM__ctor(&pk_default_vm);
}

void py_finalize(){
    pk_VM__dtor(&pk_default_vm);
    pk_vm = NULL;
    pk_StrName__finalize();
    Pools_finalize();
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

