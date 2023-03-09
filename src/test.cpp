#include "cffi.h"
#include "pocketpy.h"

using namespace pkpy;

double add(int a, double b){
    return a + b;
}

int main(){
    VM* vm = pkpy_new_vm(true);

    vm->bind_builtin_func<2>("add", ProxyFunction(&add));
    pkpy_vm_exec(vm, "print( add(1, 2.0) )");

    pkpy_delete(vm);
    return 0;
}