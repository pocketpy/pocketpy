---
icon: code
label: 'Installation'
order: 100
---

You need to download `pocketpy.h` on our GitHub release page.
And `#include` it in your project.

https://github.com/blueloveTH/pocketpy/releases/latest

Alternatively, you can install it via vcpkg.io.
(Will be available soon)

```bash
vcpkg install pocketpy
```

## Compile flags

To compile it with your project, these flags must be set:

+ `--std=c++17` flag must be set
+ Exception must be enabled
+ RTTI is not required

!!!
For maximum performance, we recommend to use `clang++` with `-O2` flag.
!!!

## Example

```cpp
#include "pocketpy.h"

using namespace pkpy;

int main(){
    // Create a virtual machine
    VM* vm = new VM(true);
    
    // Hello world!
    vm->exec("print('Hello world!')", "main.py", EXEC_MODE);

    // Create a list
    vm->exec("a = [1, 2, 3]", "main.py", EXEC_MODE);

    // Eval the sum of the list
    PyObject* result = vm->exec("sum(a)", "<eval>", EVAL_MODE);
    std::cout << CAST(int, result);   // 6
    return 0;
}
```