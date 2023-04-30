#include "pocketpy_c.h"
#include <stdio.h>

//tests the c bindings for pocketpy


void test_binding(pkpy_vm vm) {
    pkpy_push_int(vm, 12);
}

int main(int argc, char** argv) {

    pkpy_vm vm = pkpy_vm_create(true, true);

    pkpy_vm_exec(vm, "print('hello world!')");

    pkpy_push_int(vm, 11);
    pkpy_set_global(vm, "eleven");

    //pkpy_push_cfunction(vm, test_binding);
    //pkpy_set_global(vm, "binding");

    pkpy_vm_exec(vm, "print(eleven)");
    //pkpy_vm_exec(vm, "print(binding())");

    pkpy_vm_exec(vm, "def x(x) : return x + 1");

    pkpy_get_global(vm, "x");
    pkpy_push_int(vm, 1);
    pkpy_call(vm, 1);

    int r;
    pkpy_to_int(vm, -1, &r);
    printf("%li\n", r);

    pkpy_clear_error(vm, NULL);

    pkpy_vm_destroy(vm);
    return 0;
}
