---
icon: code
label: 'C'
order: 5
---

For C developers, you can download the `pocketpy.h` on our GitHub release page.

https://github.com/blueloveTH/pocketpy/releases/latest

## Basic Example

```c
#include "pocketpy.h"

int main(){
    // Create a virtual machine
    auto vm = pkpy_new_vm();
    
    // Hello world!
    pkpy_vm_exec(vm, "print('Hello world!')");

    // Create a list
    pkpy_vm_exec(vm, "a = [1, 2, 3]");

    // Eval the sum of the list
    char* result = pkpy_vm_eval(vm, "sum(a)");
    printf("%s", result);   // 6

    // Free the resources
    pkpy_delete(result);
    pkpy_delete(vm);
    return 0;
}
```