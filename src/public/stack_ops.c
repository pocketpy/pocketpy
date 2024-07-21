#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/vm.h"

py_Ref py_reg(int i) { return pk_current_vm->reg + i; }

py_Ref py_getdict(py_Ref self, py_Name name) {
    assert(self && self->is_ptr);
    if(!py_ismagicname(name) || self->type != tp_type) {
        return pk_NameDict__try_get(PyObject__dict(self->_obj), name);
    } else {
        py_Type* ud = py_touserdata(self);
        py_Ref slot = py_tpmagic(*ud, name);
        return py_isnil(slot) ? NULL : slot;
    }
}

void py_setdict(py_Ref self, py_Name name, py_Ref val) {
    assert(self && self->is_ptr);
    if(!py_ismagicname(name) || self->type != tp_type) {
        pk_NameDict__set(PyObject__dict(self->_obj), name, *val);
    } else {
        py_Type* ud = py_touserdata(self);
        *py_tpmagic(*ud, name) = *val;
    }
}

bool py_deldict(py_Ref self, py_Name name) {
    assert(self && self->is_ptr);
    if(!py_ismagicname(name) || self->type != tp_type) {
        return pk_NameDict__del(PyObject__dict(self->_obj), name);

    } else {
        py_Type* ud = py_touserdata(self);
        py_newnil(py_tpmagic(*ud, name));
        return true;
    }
}

py_Ref py_getslot(py_Ref self, int i) {
    assert(self && self->is_ptr);
    assert(i >= 0 && i < self->_obj->slots);
    return PyObject__slots(self->_obj) + i;
}

void py_setslot(py_Ref self, int i, py_Ref val) {
    assert(self && self->is_ptr);
    assert(i >= 0 && i < self->_obj->slots);
    PyObject__slots(self->_obj)[i] = *val;
}

void py_assign(py_Ref dst, py_Ref src) { *dst = *src; }

/* Stack References */
py_Ref py_peek(int i) {
    assert(i < 0);
    return pk_current_vm->stack.sp + i;
}

void py_pop() {
    pk_VM* vm = pk_current_vm;
    vm->stack.sp--;
}

void py_shrink(int n) {
    pk_VM* vm = pk_current_vm;
    vm->stack.sp -= n;
}

void py_push(py_Ref src) {
    pk_VM* vm = pk_current_vm;
    *vm->stack.sp++ = *src;
}

void py_pushnil() {
    pk_VM* vm = pk_current_vm;
    py_newnil(vm->stack.sp++);
}

py_Ref py_pushtmp() {
    pk_VM* vm = pk_current_vm;
    py_newnil(vm->stack.sp++);
    return py_peek(-1);
}