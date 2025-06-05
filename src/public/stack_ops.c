#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/vm.h"

py_Ref py_getreg(int i) { return pk_current_vm->reg + i; }

void py_setreg(int i, py_Ref val) { pk_current_vm->reg[i] = *val; }

py_Ref py_getdict(py_Ref self, py_Name name) {
    assert(self && self->is_ptr);
    return NameDict__try_get(PyObject__dict(self->_obj), name);
}

void py_setdict(py_Ref self, py_Name name, py_Ref val) {
    assert(self && self->is_ptr);
    NameDict__set(PyObject__dict(self->_obj), name, val);
}

py_ItemRef py_emplacedict(py_Ref self, py_Name name) {
    py_setdict(self, name, py_NIL());
    return py_getdict(self, name);
}

bool py_applydict(py_Ref self, bool (*f)(py_Name, py_Ref, void*), void* ctx) {
    assert(self && self->is_ptr);
    NameDict* dict = PyObject__dict(self->_obj);
    for(int i = 0; i < dict->capacity; i++) {
        NameDict_KV* kv = &dict->items[i];
        if(kv->key == NULL) continue;
        bool ok = f(kv->key, &kv->value, ctx);
        if(!ok) return false;
    }
    return true;
}

bool py_deldict(py_Ref self, py_Name name) {
    assert(self && self->is_ptr);
    return NameDict__del(PyObject__dict(self->_obj), name);
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

py_StackRef py_inspect_currentfunction() {
    VM* vm = pk_current_vm;
    if(vm->curr_decl_based_function) { return vm->curr_decl_based_function; }
    py_Frame* frame = vm->top_frame;
    if(!frame || frame->is_locals_special) return NULL;
    return frame->p0;
}

py_GlobalRef py_inspect_currentmodule() {
    py_Frame* frame = pk_current_vm->top_frame;
    if(!frame) return NULL;
    return frame->module;
}

py_Frame* py_inspect_currentframe() { return pk_current_vm->top_frame; }

void py_assign(py_Ref dst, py_Ref src) { *dst = *src; }

/* Stack References */
py_Ref py_peek(int i) {
    assert(i <= 0);
    return pk_current_vm->stack.sp + i;
}

void py_pop() {
    VM* vm = pk_current_vm;
    vm->stack.sp--;
}

void py_shrink(int n) {
    VM* vm = pk_current_vm;
    vm->stack.sp -= n;
}

void py_push(py_Ref src) {
    VM* vm = pk_current_vm;
    *vm->stack.sp++ = *src;
}

void py_pushnil() {
    VM* vm = pk_current_vm;
    py_newnil(vm->stack.sp++);
}

void py_pushnone() {
    VM* vm = pk_current_vm;
    py_newnone(vm->stack.sp++);
}

void py_pushname(py_Name name) {
    VM* vm = pk_current_vm;
    py_newint(vm->stack.sp++, (uintptr_t)name);
}

py_Ref py_pushtmp() {
    VM* vm = pk_current_vm;
    return vm->stack.sp++;
}