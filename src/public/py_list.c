#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/vm.h"

typedef c11_vector List;

void py_newlist(py_Ref out) {
    pk_VM* vm = pk_current_vm;
    PyObject* obj = pk_ManagedHeap__gcnew(&vm->heap, tp_list, 0, sizeof(List));
    List* userdata = PyObject__value(obj);
    c11_vector__ctor(userdata, sizeof(py_TValue));
    out->type = tp_list;
    out->is_ptr = true;
    out->_obj = obj;
}

void py_newlistn(py_Ref out, int n) {
    py_newlist(out);
    List* userdata = py_touserdata(out);
    c11_vector__reserve(userdata, n);
    userdata->count = n;
}

py_Ref py_list__getitem(const py_Ref self, int i) {
    List* userdata = py_touserdata(self);
    return c11__at(py_TValue, userdata, i);
}

void py_list__setitem(py_Ref self, int i, const py_Ref val) {
    List* userdata = py_touserdata(self);
    c11__setitem(py_TValue, userdata, i, *val);
}

void py_list__delitem(py_Ref self, int i) {
    List* userdata = py_touserdata(self);
    c11_vector__erase(py_TValue, userdata, i);
}

int py_list__len(const py_Ref self) {
    List* userdata = py_touserdata(self);
    return userdata->count;
}

void py_list__append(py_Ref self, const py_Ref val) {
    List* userdata = py_touserdata(self);
    c11_vector__push(py_TValue, userdata, *val);
}

void py_list__clear(py_Ref self) {
    List* userdata = py_touserdata(self);
    c11_vector__clear(userdata);
}

void py_list__insert(py_Ref self, int i, const py_Ref val) {
    List* userdata = py_touserdata(self);
    c11_vector__insert(py_TValue, userdata, i, *val);
}