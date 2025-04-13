---
icon: rocket
order: 20
label: Quick Start
---

You have two options to integrate pkpy into your project.

#### Use the single header file

Download the `pocketpy.h` and `pocketpy.c` on our [GitHub Release](https://github.com/pocketpy/pocketpy/releases) page.
And `#include` it in your project.

#### Use CMake

Clone the whole repository as a submodule into your project,
In your CMakelists.txt, add the following lines:

```cmake
add_subdirectory(pocketpy)
target_link_libraries(<your_target> pocketpy)
```

See [CMakeLists.txt](https://github.com/pocketpy/pocketpy/blob/main/CMakeLists.txt) for details.

It is safe to use `main` branch in production if CI badge is green.

### Compile flags

To compile it with your project, these flags must be set:

+ `--std=c11` flag must be set
+ For MSVC, `/utf-8` and `/experimental:c11atomics` flag must be set
+ `NDEBUG` macro should be defined for release build, or you will get poor performance

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
└── windows
    └── x86_64
        ├── main.exe
        └── pocketpy.dll
```

### Example

```c
#include "pocketpy.h"
#include <stdio.h>

static bool int_add(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(0, tp_int);
    PY_CHECK_ARG_TYPE(1, tp_int);
    py_i64 a = py_toint(py_arg(0));
    py_i64 b = py_toint(py_arg(1));
    py_newint(py_retval(), a + b);
    return true;
}

int main() {
    // Initialize pocketpy
    py_initialize();

    // Hello world!
    bool ok = py_exec("print('Hello world!')", "<string>", EXEC_MODE, NULL);
    if(!ok) goto __ERROR;

    // Create a list: [1, 2, 3]
    py_Ref r0 = py_getreg(0);
    py_newlistn(r0, 3);
    py_newint(py_list_getitem(r0, 0), 1);
    py_newint(py_list_getitem(r0, 1), 2);
    py_newint(py_list_getitem(r0, 2), 3);

    // Eval the sum of the list
    py_Ref f_sum = py_getbuiltin(py_name("sum"));
    py_push(f_sum);
    py_pushnil();
    py_push(r0);
    ok = py_vectorcall(1, 0);
    if(!ok) goto __ERROR;

    printf("Sum of the list: %d\n", (int)py_toint(py_retval()));  // 6

    // Bind native `int_add` as a global variable
    py_newnativefunc(r0, int_add);
    py_setglobal(py_name("add"), r0);

    // Call `add` in python
    ok = py_exec("add(3, 7)", "<string>", EVAL_MODE, NULL);
    if(!ok) goto __ERROR;

    py_i64 res = py_toint(py_retval());
    printf("Sum of 2 variables: %d\n", (int)res);  // 10

    py_finalize();
    return 0;

__ERROR:
    py_printexc();
    py_finalize();
    return 1;
}
```
