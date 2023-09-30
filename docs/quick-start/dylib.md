---
icon: dot
label: 'Use dynamic library'
order: 45
---

!!!
This feature is optional. Set `PK_USE_DYLIB` to `1` to enable it.
!!!

You can import a native module from a dynamic library at runtime.
This feature is supported on Windows, Linux, macOS, and Android.

## Create a module as a dynamic library

Implement a `pkpy_module__init__` function and export it as a symbol.
This is the entry point of the module. When users call `__import__` function,
the VM will call this function to initialize the module.

You can create one or more modules inside `pkpy_module__init__` function,
and return the name of the module you want users to import directly.

You should use C-APIs to interact with the VM in the dynamic library.
This is to make sure the dynamic library is compatible with different compilers.

```c
#include "pocketpy_c.h"
#include <stdio.h>
#include <stdlib.h>

static int hello(pkpy_vm* vm){
    printf("Hello from dylib!\n");
    return 0;
}

PK_EXPORT
const char* pkpy_module__init__(pkpy_vm* vm, const char* version){
    printf("version: %s\n", version);
    pkpy_push_function(vm, "hello()", hello);
    pkpy_push_module(vm, "test");
    pkpy_setattr(vm, pkpy_name("hello"));
    // check if initialization failed
    if(pkpy_clear_error(vm, NULL)) return NULL;
    return "test";
}
```

## Load a dynamic library

You can load a dynamic library with `__import__` function with a path to the library.

```python
test = __import__("test.dll")   # Windows

test = __import__("libtest.so") # Linux

test = __import__("libtest.dylib") # macOS

test = __import__("libtest.so") # Android

test.hello()                # Hello from dylib!
```
