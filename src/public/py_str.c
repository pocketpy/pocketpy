#include "pocketpy/common/str.h"
#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/vm.h"

py_Type pk_str__register() {
    pk_VM* vm = pk_current_vm;
    py_Type type = pk_VM__new_type(vm, "str", tp_object, NULL, false);
    // no need to dtor because the memory is controlled by the object
    return type;
}

py_Type pk_bytes__register() {
    pk_VM* vm = pk_current_vm;
    py_Type type = pk_VM__new_type(vm, "bytes", tp_object, NULL, false);
    // no need to dtor because the memory is controlled by the object
    return type;
}

void py_newstr(py_Ref out, const char* data) {
    return py_newstrn(out, data, strlen(data));
}

void py_newstrn(py_Ref out, const char* data, int size) {
    pk_ManagedHeap* heap = &pk_current_vm->heap;
    int total_size = sizeof(c11_string) + size + 1;
    PyObject* obj = pk_ManagedHeap__gcnew(heap, tp_str, 0, total_size);
    c11_string* ud = PyObject__userdata(obj);
    c11_string__ctor2(ud, data, size);
    out->type = tp_str;
    out->is_ptr = true;
    out->_obj = obj;
}

unsigned char* py_newbytes(py_Ref out, int size) {
    pk_ManagedHeap* heap = &pk_current_vm->heap;
    // 4 bytes size + data
    PyObject* obj = pk_ManagedHeap__gcnew(heap, tp_bytes, 0, sizeof(c11_bytes) + size);
    c11_bytes* ud = PyObject__userdata(obj);
    ud->size = size;
    out->type = tp_bytes;
    out->is_ptr = true;
    out->_obj = obj;
    return ud->data;
}

const char* py_tostr(const py_Ref self) {
    assert(self->type == tp_str);
    c11_string* ud = PyObject__userdata(self->_obj);
    return ud->data;
}

const char* py_tostrn(const py_Ref self, int* size) {
    assert(self->type == tp_str);
    c11_string* ud = PyObject__userdata(self->_obj);
    *size = ud->size;
    return ud->data;
}

unsigned char* py_tobytes(const py_Ref self, int* size) {
    assert(self->type == tp_bytes);
    c11_bytes* ud = PyObject__userdata(self->_obj);
    *size = ud->size;
    return ud->data;
}

