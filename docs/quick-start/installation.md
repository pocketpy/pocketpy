---
icon: dot
label: 'Installation'
order: 100
---

You have two options to integrate pkpy into your project.

#### Use the single header file

Download the `pocketpy.h` on our [GitHub Release](https://github.com/pocketpy/pocketpy/releases) page.
And `#include` it in your project. The header can only be included once.

#### Use CMake

Clone the whole repository as a submodule into your project,
In your CMakelists.txt, add the following lines:

```cmake
add_subdirectory(pocketpy)
target_link_libraries(<your_target> pocketpy)

if(EMSCRIPTEN)
    # exceptions must be enabled for emscripten
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fexceptions")
endif()
```

See [CMakeLists.txt](https://github.com/pocketpy/pocketpy/blob/main/CMakeLists.txt) for details.

It is safe to use `main` branch in production if CI badge is green.

### Compile flags

To compile it with your project, these flags must be set:

+ `--std=c++17` flag must be set
+ RTTI must be enabled
+ Exception must be enabled
+ For MSVC, `/utf-8` flag must be set

For emscripten, you must enable exceptions to make pocketpy work properly.
See https://emscripten.org/docs/porting/exceptions.html.

### Get prebuilt binaries

We have prebuilt binaries,
check them out on our [GitHub Actions](https://github.com/pocketpy/pocketpy/actions/workflows/main.yml).

You can download an artifact there which contains the following files.

```
├── android
│   ├── arm64-v8a
│   │   └── libpocketpy.so
│   ├── armeabi-v7a
│   │   └── libpocketpy.so
│   └── x86_64
│       └── libpocketpy.so
├── ios
│   └── libpocketpy.a
├── linux
│   └── x86_64
│       ├── libpocketpy.so
│       └── main
├── macos
│   └── pocketpy.bundle
│       └── Contents
│           ├── Info.plist
│           └── MacOS
│               └── pocketpy
└── windows
    └── x86_64
        ├── main.exe
        └── pocketpy.dll
```

### Example

```cpp
#include "pocketpy.h"

using namespace pkpy;

int main(){
    // Create a virtual machine
    VM* vm = new VM();

    // Hello world!
    vm->exec("print('Hello world!')");

    // Create a list
    vm->exec("a = [1, 2, 3]");

    // Eval the sum of the list
    PyObject* result = vm->eval("sum(a)");
    std::cout << "Sum of the list: "<< py_cast<int>(vm, result) << std::endl;   // 6

    // Bindings
    vm->bind(vm->_main, "add(a: int, b: int)",
      [](VM* vm, ArgsView args){
        int a = py_cast<int>(vm, args[0]);
        int b = py_cast<int>(vm, args[1]);
        return py_var(vm, a + b);
      });

    // Call the function
    PyObject* f_add = vm->_main->attr("add");
    result = vm->call(f_add, py_var(vm, 3), py_var(vm, 7));
    std::cout << "Sum of 2 variables: "<< py_cast<int>(vm, result) << std::endl;   // 10

    // Dispose the virtual machine
    delete vm;
    return 0;
```

### Overview

pkpy's C++ interfaces are organized in an object-oriented way.
All classes are located in `pkpy` namespace.

The most important class is the `VM` class. A `VM` instance is a python virtual machine which holds all necessary runtime states, including callstack, modules, variables, etc.

A process can have multiple `VM` instances. Each `VM` instance is independent from each other.

!!!
Always use C++ `new` operator to create a `VM` instance.
DO NOT declare it on the stack. It may cause stack overflow.
!!!

```cpp
VM* vm = new VM();
```

The constructor can take 1 extra parameters.

#### `VM(bool enable_os=true)`

+ `enable_os`, whether to enable OS-related features or not. This setting controls the availability of privileged modules such os `io` and `os` as well as builtin function `open`. **It is designed for sandboxing.**

When you are done with the `VM` instance, use `delete` operator to dispose it.

```cpp
delete vm;
```

### Hook standard buffer

By default, pkpy outputs all messages and errors to `stdout` and `stderr`.
You can redirect them to your own buffer by setting `vm->_stdout` and `vm->_stderr`.

These two fields are C function pointers `PrintFunc` with the following signature:

```cpp
typedef void(*PrintFunc)(const char*, int)
```

Or you can override these two virtual functions:
```cpp
    virtual void stdout_write(const Str& s){
        _stdout(s.data, s.size);
    }

    virtual void stderr_write(const Str& s){
        _stderr(s.data, s.size);
    }
```