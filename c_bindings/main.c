#include "pocketpy_c.h"
#include <stdio.h>


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

    pkpy_vm_exec(vm, "x = lambda x : x + 1");

    pkpy_get_global(vm, "x");
    pkpy_push_null(vm);
    pkpy_push_int(vm, 1);
    pkpy_call(vm, 1);

    int64_t r = pkpy_toint(vm, 1);
    printf("%li\n", r);

    pkpy_vm_destroy(vm);
    return 0;
}
