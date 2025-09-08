#include "pocketpy/interpreter/vm.h"

py_Ref py_getreg(int i) { return pk_current_vm->reg + i; }

void py_setreg(int i, py_Ref val) { pk_current_vm->reg[i] = *val; }

PK_INLINE py_Ref py_retval() { return &pk_current_vm->last_retval; }

PK_INLINE py_Ref py_getdict(py_Ref self, py_Name name) {
    assert(self && self->is_ptr);
    return NameDict__try_get(PyObject__dict(self->_obj), name);
}

PK_INLINE void py_setdict(py_Ref self, py_Name name, py_Ref val) {
    assert(self && self->is_ptr);
    NameDict__set(PyObject__dict(self->_obj), name, val);
}

bool py_deldict(py_Ref self, py_Name name) {
    assert(self && self->is_ptr);
    return NameDict__del(PyObject__dict(self->_obj), name);
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

void py_cleardict(py_Ref self) {
    assert(self && self->is_ptr);
    NameDict* dict = PyObject__dict(self->_obj);
    NameDict__clear(dict);
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

py_Ref py_getbuiltin(py_Name name) { return py_getdict(pk_current_vm->builtins, name); }

py_Ref py_getglobal(py_Name name) { return py_getdict(pk_current_vm->main, name); }

void py_setglobal(py_Name name, py_Ref val) { py_setdict(pk_current_vm->main, name, val); }
