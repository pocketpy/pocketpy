#include "pocketpy_c.h"


int main(int argc, char** argv) {

    pkpy_vm vm = pkpy_vm_create(true, true);

    pkpy_vm_exec(vm, "print('hello world!')");

    pkpy_vm_destroy(vm);

    return 0;
}
