---
icon: rocket
order: 20
label: Quick Start
---

# Quick start

Use a C11 compiler and CMake 3.20 or newer. On Windows, use an MSVC environment
with C11 atomics support. C++ bindings additionally require C++17.

## 1. Build and run the interpreter

From a checkout of the repository:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Run `build/main` on a single-configuration build, or
`.\build\Release\main.exe` with the Visual Studio generator. Starting it without
arguments opens the REPL; type `exit()` to leave. To run a file:

```sh
./build/main hello.py
```

On Windows:

```powershell
.\build\Release\main.exe hello.py
```

Save this as `hello.py`:

```python
print('Hello from pocketpy!')
print(sum([1, 2, 3]))  # 6
```

When using the default shared-library build, keep the generated library beside
the executable. Set `-DPK_BUILD_STATIC_MAIN=ON` to link the interpreter statically.
For reproducible application builds, select a release tag or a specific commit.

## 2. Embed pocketpy with CMake

Place pocketpy in a `pocketpy/` subdirectory of your application:

```text
my_app/
├── CMakeLists.txt
├── main.c
└── pocketpy/
```

Use this `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.20)
project(my_app LANGUAGES C CXX)

add_subdirectory(pocketpy)
add_executable(my_app main.c)
target_compile_features(my_app PRIVATE c_std_11)
target_link_libraries(my_app PRIVATE pocketpy)
if(MSVC)
    target_compile_options(my_app PRIVATE /utf-8 /experimental:c11atomics)
endif()
```

As a subdirectory, pocketpy builds a static library by default. Its target
provides the public include path. Build your application with the same
`cmake -S` and `cmake --build` commands used above.

Save the following complete program as `main.c`:

```c
#include "pocketpy.h"

int main(void) {
    py_initialize();

    bool ok = py_exec("print('Hello from embedded Python!')",
                      "hello.py", EXEC_MODE, NULL);
    if(!ok) py_printexc();

    py_finalize();
    return ok ? 0 : 1;
}
```

`EXEC_MODE` executes statements. The filename appears in tracebacks; `NULL`
selects the `__main__` module. Always check the result of an API that can raise
an exception. `py_finalize()` ends the interpreter's lifetime; do not call Python
APIs or initialize it again afterward.

Next, follow [C bindings](bindings.md) to expose a native function, or
[C++ bindings](bindings-cpp.md) to bind a C++ library.

## Alternative: the amalgamated files

The amalgamated distribution consists of **two files**, `pocketpy.h` and
`pocketpy.c`. Download a matching pair from
[Releases](https://github.com/pocketpy/pocketpy/releases), or generate them from a
checkout using a host Python installation:

```sh
python amalgamate.py
```

The generated files are placed in `amalgamated/`. Compile `pocketpy.c` as C11,
add its directory to your include path, and link it with your application.
Include `pocketpy.h` from application code; compile the `.c` file only once.

CMake and amalgamated builds have different feature defaults. See
[build configuration](build.md) for flags, platform libraries, and optional
modules.

## Prebuilt artifacts

The [build workflow](https://github.com/pocketpy/pocketpy/actions/workflows/main.yml)
provides artifacts for supported targets. Choose one that matches your OS,
architecture, and revision. Use headers from the same revision as the binary.

## Common first-run problems

| Symptom | Check |
| --- | --- |
| Compiler cannot find `pocketpy.h` | Link the CMake `pocketpy` target, or add the public include directory. |
| Executable cannot load `pocketpy.dll` | Keep the DLL next to the executable or use a static build. |
| `ImportError` for a local script | Start in the directory containing your modules; see [module loading](C-API/modules.md). |
| Release execution is unexpectedly slow | Enable optimization and define `NDEBUG`; see [performance](performance.md). |
