#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/vm.h"

py_Ref py_getdict(const py_Ref self, py_Name name){
    assert(self && self->is_ptr);
    return pk_NameDict__try_get(PyObject__dict(self->_obj), name);
}

void py_setdict(py_Ref self, py_Name name, const py_Ref val){
    assert(self && self->is_ptr);
    pk_NameDict__set(PyObject__dict(self->_obj), name, *val);
}

py_Ref py_getslot(const py_Ref self, int i){
    assert(self && self->is_ptr);
    assert(i >= 0 && i < self->_obj->slots);
    return PyObject__slots(self->_obj) + i;
}

void py_setslot(py_Ref self, int i, const py_Ref val){
    assert(self && self->is_ptr);
    assert(i >= 0 && i < self->_obj->slots);
    PyObject__slots(self->_obj)[i] = *val;
}

void py_copyref(const py_Ref src, py_Ref dst){
    *dst = *src;
}

/* Stack References */
py_Ref py_gettop(){
    return pk_current_vm->stack.sp - 1;
}

void py_settop(const py_Ref val){
    pk_current_vm->stack.sp[-1] = *val;
}

py_Ref py_getsecond(){
    return pk_current_vm->stack.sp - 2;
}

void py_setsecond(const py_Ref val){
    pk_current_vm->stack.sp[-2] = *val;
}

py_Ref py_peek(int i){
    assert(i < 0);
    return pk_current_vm->stack.sp + i;
}

py_Ref py_push(){
    pk_VM* vm = pk_current_vm;
    py_Ref top = vm->stack.sp;
    vm->stack.sp++;
    return top;
}

void py_pop(){
    pk_VM* vm = pk_current_vm;
    vm->stack.sp--;
}

void py_shrink(int n){
    pk_VM* vm = pk_current_vm;
    vm->stack.sp -= n;
}

void py_pushref(const py_Ref src){
    *py_push() = *src;
}

py_Ref py_pushtmp(){
    py_Ref r = py_push();
    py_newnull(r);
    return r;
}

void py_poptmp(int n){
    py_shrink(n);
}