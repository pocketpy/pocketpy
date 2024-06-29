#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/vm.h"

py_Ref py_getreg(int i){
    return pk_current_vm->reg + i;
}

void py_setreg(int i, const py_Ref val){
    pk_current_vm->reg[i] = *val;
}

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

void py_assign(py_Ref dst, const py_Ref src){
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

void py_duptop(){
    pk_VM* vm = pk_current_vm;
    *vm->stack.sp = vm->stack.sp[-1];
    vm->stack.sp++;
}

void py_dupsecond(){
    pk_VM* vm = pk_current_vm;
    *vm->stack.sp = vm->stack.sp[-2];
    vm->stack.sp++;
}

py_Ref py_peek(int i){
    assert(i < 0);
    return pk_current_vm->stack.sp + i;
}

void py_pop(){
    pk_VM* vm = pk_current_vm;
    vm->stack.sp--;
}

void py_shrink(int n){
    pk_VM* vm = pk_current_vm;
    vm->stack.sp -= n;
}

void py_push(const py_Ref src){
    pk_VM* vm = pk_current_vm;
    *vm->stack.sp++ = *src;
}

py_Ref py_pushtmp(){
    pk_VM* vm = pk_current_vm;
    py_newnull(vm->stack.sp++);
    return py_gettop();
}

void py_poptmp(int n){
    py_shrink(n);
}