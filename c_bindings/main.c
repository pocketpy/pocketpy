#include "pocketpy_c.h"


void test_binding(pkpy_vm vm) {
    pkpy_push_int(vm, 12);
}

int main(int argc, char** argv) {

    pkpy_vm vm = pkpy_vm_create(true, true);

    pkpy_vm_exec(vm, "print('hello world!')");

    pkpy_push_int(vm, 11);
    pkpy_set_global(vm, "eleven");

    pkpy_push_cfunction(vm, test_binding);
    pkpy_set_global(vm, "binding");

    pkpy_vm_exec(vm, "print(eleven)");
    pkpy_vm_exec(vm, "print(binding())");

    pkpy_vm_destroy(vm);

    return 0;
}
