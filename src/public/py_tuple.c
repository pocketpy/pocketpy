#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/vm.h"

void py_newtuple(py_Ref out, int n) {
    pk_VM* vm = pk_current_vm;
    PyObject* obj = pk_ManagedHeap__gcnew(&vm->heap, tp_tuple, n, 0);
    out->type = tp_tuple;
    out->is_ptr = true;
    out->_obj = obj;
}

py_Ref py_tuple__getitem(const py_Ref self, int i){
    return py_getslot(self, i);
}

void py_tuple__setitem(py_Ref self, int i, const py_Ref val){
    py_setslot(self, i, val);
}

int py_tuple__len(const py_Ref self){
    return self->_obj->slots;
}
