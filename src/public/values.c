#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/vm.h"

void py_newint(py_Ref self, int64_t val) {
    self->type = tp_int;
    self->is_ptr = false;
    self->_i64 = val;
}

void py_newfloat(py_Ref self, double val) {
    self->type = tp_float;
    self->is_ptr = false;
    self->_f64 = val;
}

void py_newbool(py_Ref self, bool val) {
    pk_VM* vm = pk_current_vm;
    *self = val ? vm->True : vm->False;
}

void py_newstr(py_Ref self, const char* data) {
    pk_ManagedHeap* heap = &pk_current_vm->heap;
    PyObject* obj = pk_ManagedHeap__gcnew(heap, tp_str, 0, sizeof(py_Str));
    py_Str__ctor(PyObject__value(obj), data);
    self->type = tp_str;
    self->is_ptr = true;
    self->_obj = obj;
}

void py_newstrn(py_Ref self, const char* data, int size) {
    pk_ManagedHeap* heap = &pk_current_vm->heap;
    PyObject* obj = pk_ManagedHeap__gcnew(heap, tp_str, 0, sizeof(py_Str));
    py_Str__ctor2((py_Str*)PyObject__value(obj), data, size);
    self->type = tp_str;
    self->is_ptr = true;
    self->_obj = obj;
}

void py_newnone(py_Ref self) {
    pk_VM* vm = pk_current_vm;
    *self = vm->None;
}

void py_newnull(py_Ref self) { self->type = 0; }

void py_newtuple(py_Ref self, int n) {
    pk_VM* vm = pk_current_vm;
    PyObject* obj = pk_ManagedHeap__gcnew(&vm->heap, tp_tuple, n, 0);
    self->type = tp_tuple;
    self->is_ptr = true;
    self->_obj = obj;
}

void py_newfunction(py_Ref self, py_CFunction f, const char* sig) {
    py_newfunction2(self, f, sig, BindType_FUNCTION, NULL, NULL);
}

void py_newfunction2(py_Ref self,
                     py_CFunction f,
                     const char* sig,
                     BindType bt,
                     const char* docstring,
                     const py_Ref upvalue) {}

void py_newnativefunc(py_Ref self, py_CFunction f, int argc) {
    py_newnativefunc2(self, f, argc, BindType_FUNCTION, NULL, NULL);
}

void py_newnativefunc2(py_Ref self,
                       py_CFunction f,
                       int argc,
                       BindType bt,
                       const char* docstring,
                       const py_Ref upvalue) {}

void py_pushint(int64_t val) { py_newint(pk_current_vm->stack.sp++, val); }

void py_pushfloat(double val) { py_newfloat(pk_current_vm->stack.sp++, val); }

void py_pushbool(bool val) { py_newbool(pk_current_vm->stack.sp++, val); }

void py_pushstr(const py_Str* val) { py_newstr(pk_current_vm->stack.sp++, py_Str__data(val)); }

void py_pushcstr(const char* val) { py_newstr(pk_current_vm->stack.sp++, val); }

void py_pushcstrn(const char* val, int size) { py_newstrn(pk_current_vm->stack.sp++, val, size); }

void py_push_notimplemented() {
    pk_VM* vm = pk_current_vm;
    *vm->stack.sp++ = vm->NotImplemented;
}
