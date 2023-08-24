---
icon: dot
label: 'Installation'
order: 100
---

You have two options to integrate pkpy into your project.

#### Use the single header file

Download the `pocketpy.h` on our [GitHub Release](https://github.com/blueloveTH/pocketpy/releases) page.
And `#include` it in your project.

#### Use CMake

Clone the whole repository as a submodule in your project,
You need **Python 3** installed on your system because CMakeLists.txt
requires it to generate some files.

In your CMakelists.txt, add the following lines:

```cmake
option(PK_BUILD_STATIC_LIB "Build static library" ON)
add_subdirectory(pocketpy)
target_link_libraries(your_target pocketpy)
```

These variables can be set to control the build process:
+ `PK_BUILD_STATIC_LIB` - Build the static library
+ `PK_BUILD_SHARED_LIB` - Build the shared library
+ `PK_USE_BOX2D` - Build with Box2D module

See [CMakeLists.txt](https://github.com/blueloveTH/pocketpy/blob/main/CMakeLists.txt) for details.

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
DO NOT declare it on the stack.
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
typedef void(*PrintFunc)(VM*, const char*, int)
```