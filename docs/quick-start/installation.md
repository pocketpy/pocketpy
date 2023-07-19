---
icon: dot
label: 'Installation'
order: 100
---

Download the `pocketpy.h` on our [GitHub Release](https://github.com/blueloveTH/pocketpy/releases) page.
And `#include` it in your project.

You can also use cmake to build it from source. See CMakeLists.txt for details.
These variables can be set to control the build process:
+ `PK_BUILD_STATIC_LIB` - Build the static library
+ `PK_BUILD_SHARED_LIB` - Build the shared library

If you are working with [Unity Engine](https://unity.com/), you can download our plugin [PocketPython](https://assetstore.unity.com/packages/tools/visual-scripting/pocketpy-241120) on the Asset Store.

If you use [Dear ImGui](https://github.com/ocornut/imgui), we provide official bindings for it. See [pkpy-imgui](https://github.com/blueloveTH/pkpy-imgui) for details.

### Compile flags

To compile it with your project, these flags must be set:

+ `--std=c++17` flag must be set
+ Exception must be enabled

### Example

```cpp
#include "pocketpy.h"

using namespace pkpy;

int main(){
    // Create a virtual machine
    VM* vm = new VM();
    
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

### Overview

pkpy's C++ interfaces are organized in an object-oriented way.
All classes are located in `pkpy` namespace.

The most important class is the `VM` class. A `VM` instance is a python virtual machine which holds all necessary runtime states, including callstack, modules, variables, etc.

A process can have multiple `VM` instances. Each `VM` instance is independent from each other.

!!!
Always use C++ `new` operator to create a `VM` instance.
Do not declare it on the stack.
!!!

```cpp
VM* vm = new VM();
```

The constructor can take 1 extra parameters.

#### `VM(bool enable_os=true)`

+ `enable_os`, whether to enable OS-related features or not. This setting controls the availability of priviledged modules such os `io` and `os` as well as builtin function `open`.

When you are done with the `VM` instance, use `delete` operator to dispose it.

```cpp
delete vm;
```

### Hook standard buffer

By default, pkpy outputs all messages and errors to `stdout` and `stderr`.
You can redirect them to your own buffer by setting `vm->_stdout` and `vm->_stderr`.

These two fields are C function pointers `PrintFunc` with the following signature:

```cpp
typedef void(*PrintFunc)(VM*, const Str&);
```