#include "pocketpy.h"
#include "pocketpy_c.h"

pkpy_vm pkpy_vm_create(bool use_stdio, bool enable_os) {
    pkpy::VM* p = new pkpy::VM(use_stdio, enable_os);

    return (pkpy_vm) p;
}

void pkpy_vm_exec(pkpy_vm vm_handle, const char* source) {
    pkpy::VM* vm = (pkpy::VM*) vm_handle;

    vm->exec(source, "main.py", pkpy::EXEC_MODE);
}

void pkpy_vm_destroy(pkpy_vm vm_handle) {
    pkpy::VM* vm = (pkpy::VM*) vm_handle;

    delete vm;
}

void pkpy_push_cfunction(pkpy_vm vm_handle, pkpy_cfunction f) {
    pkpy::VM* vm = (pkpy::VM*) vm_handle;
    vm->s_data.push(VAR((pkpy::StackFunc) f));
}

void pkpy_push_int(pkpy_vm vm_handle, int64_t value) {
    pkpy::VM* vm = (pkpy::VM*) vm_handle;
    vm->s_data.push(VAR(value));
}

void pkpy_push_null(pkpy_vm vm_handle) {
    pkpy::VM* vm = (pkpy::VM*) vm_handle;
    vm->s_data.push(pkpy::PY_NULL);
}


void pkpy_push_float(pkpy_vm vm_handle, double value) {
    pkpy::VM* vm = (pkpy::VM*) vm_handle;
    vm->s_data.push(VAR(value));
}

void pkpy_set_global(pkpy_vm vm_handle, const char* name) {
    pkpy::VM* vm = (pkpy::VM*) vm_handle;

    vm->_main->attr().set(name, vm->s_data.top());

    vm->s_data.pop();
}

void pkpy_get_global(pkpy_vm vm_handle, const char* name) {
    pkpy::VM* vm = (pkpy::VM*) vm_handle;

    pkpy::PyObject* o = vm->_main->attr().try_get(name);

    vm->s_data.push(o);
}

void pkpy_call(pkpy_vm vm_handle, int argc) {
    pkpy::VM* vm = (pkpy::VM*) vm_handle;
    pkpy::PyObject* o = vm->vectorcall(argc, 0, 0);
    vm->s_data.push(o);
}

int pkpy_toint(pkpy_vm vm_handle, int index) {
    pkpy::VM* vm = (pkpy::VM*) vm_handle;

    pkpy::PyObject* o = vm->s_data.peek(index);
    return pkpy::py_cast<int>(vm, o);
}

