#include "cffi.h"
#include "pocketpy.h"

using namespace pkpy;

float* f(int* a){
    *a = 100;
    return new float(3.5f);
}

int main(){
    VM* vm = pkpy_new_vm(true);
    vm->bind_builtin_func<1>("f", NativeProxyFunc(&f));

    pkpy_vm_exec(vm, R"(
from c import *
p = malloc(4).cast("int*")
ret = f(p)
print(p.get())          # 100
print(ret, ret.get())   # 3.5
)");

    pkpy_delete(vm);
    return 0;
}