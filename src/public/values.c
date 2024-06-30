#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/vm.h"

void py_newint(py_Ref out, int64_t val) {
    out->type = tp_int;
    out->is_ptr = false;
    out->_i64 = val;
}

void py_newfloat(py_Ref out, double val) {
    out->type = tp_float;
    out->is_ptr = false;
    out->_f64 = val;
}

void py_newbool(py_Ref out, bool val) {
    pk_VM* vm = pk_current_vm;
    *out = val ? vm->True : vm->False;
}

void py_newstr(py_Ref out, const char* data) {
    pk_ManagedHeap* heap = &pk_current_vm->heap;
    PyObject* obj = pk_ManagedHeap__gcnew(heap, tp_str, 0, sizeof(py_Str));
    py_Str__ctor(PyObject__value(obj), data);
    out->type = tp_str;
    out->is_ptr = true;
    out->_obj = obj;
}

void py_newstrn(py_Ref out, const char* data, int size) {
    pk_ManagedHeap* heap = &pk_current_vm->heap;
    PyObject* obj = pk_ManagedHeap__gcnew(heap, tp_str, 0, sizeof(py_Str));
    py_Str__ctor2((py_Str*)PyObject__value(obj), data, size);
    out->type = tp_str;
    out->is_ptr = true;
    out->_obj = obj;
}

void py_newStr_(py_Ref out, py_Str input) {
    pk_ManagedHeap* heap = &pk_current_vm->heap;
    PyObject* obj = pk_ManagedHeap__gcnew(heap, tp_str, 0, sizeof(py_Str));
    py_Str* userdata = PyObject__value(obj);
    *userdata = input;
    out->type = tp_str;
    out->is_ptr = true;
    out->_obj = obj;
}

void py_newbytes(py_Ref out, const unsigned char* data, int size) {
    pk_ManagedHeap* heap = &pk_current_vm->heap;
    // 4 bytes size + data
    PyObject* obj = pk_ManagedHeap__gcnew(heap, tp_bytes, 0, sizeof(int) + size);
    int* psize = (int*)PyObject__value(obj);
    *psize = size;
    memcpy(psize + 1, data, size);
    out->type = tp_bytes;
    out->is_ptr = true;
    out->_obj = obj;
}

void py_newnone(py_Ref out) {
    pk_VM* vm = pk_current_vm;
    *out = vm->None;
}

void py_newnull(py_Ref out) { out->type = 0; }

void py_newfunction(py_Ref out, py_CFunction f, const char* sig) {
    py_newfunction2(out, f, sig, BindType_FUNCTION, NULL, NULL);
}

void py_newfunction2(py_Ref out,
                     py_CFunction f,
                     const char* sig,
                     BindType bt,
                     const char* docstring,
                     const py_Ref upvalue) {}

void py_newnativefunc(py_Ref out, py_CFunction f) {
    out->type = tp_nativefunc;
    out->is_ptr = false;
    out->_cfunc = f;
}

void py_bindmethod(py_Type type, const char *name, py_CFunction f){
    py_bindmethod2(type, name, f, BindType_FUNCTION);
}

void py_bindmethod2(py_Type type, const char *name, py_CFunction f, BindType bt){
    py_TValue tmp;
    py_newnativefunc(&tmp, f);
    py_setdict(py_tpobject(type), py_name(name), &tmp);
}

void py_bindnativefunc(py_Ref obj, const char *name, py_CFunction f){
    py_TValue tmp;
    py_newnativefunc(&tmp, f);
    py_setdict(obj, py_name(name), &tmp);
}

void py_newnotimplemented(py_Ref out) {
    pk_VM* vm = pk_current_vm;
    *out = vm->NotImplemented;
}

void py_newslice(py_Ref out, const py_Ref start, const py_Ref stop, const py_Ref step) {
    py_newobject(out, tp_slice, 3, 0);
    py_setslot(out, 0, start);
    py_setslot(out, 1, stop);
    py_setslot(out, 2, step);
}

void py_newobject(py_Ref out, py_Type type, int slots, int udsize) {
    pk_ManagedHeap* heap = &pk_current_vm->heap;
    PyObject* obj = pk_ManagedHeap__gcnew(heap, type, slots, udsize);
    out->type = type;
    out->is_ptr = true;
    out->_obj = obj;
}
