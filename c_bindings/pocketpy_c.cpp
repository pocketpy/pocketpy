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

