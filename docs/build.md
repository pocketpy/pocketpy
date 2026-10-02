---
icon: tools
order: 19
label: Build Configuration
---

# Build configuration

[Quick start](quick-start.md) covers the default build. This page explains the
options that change what the interpreter can do.

## CMake options

Pass options when configuring the build:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DPK_BUILD_STATIC_MAIN=ON -DPK_BUILD_MODULE_LZ4=ON
cmake --build build --config Release
```

When embedding with `add_subdirectory(pocketpy)`, set cache options **before**
adding the subdirectory, or pass them on the CMake command line.

| Option | Default | Effect |
| --- | --- | --- |
| `PK_BUILD_SHARED_LIB` | `OFF` | Build only the shared library. |
| `PK_BUILD_STATIC_LIB` | `OFF` at the repository root; `ON` as a dependency | Build only the static library. |
| `PK_BUILD_STATIC_MAIN` | `OFF` | Link the standalone executable statically when neither library-only option is selected. |
| `PK_BUILD_WITH_UNITY` | `ON` | Combine interpreter sources into a unity build. |
| `PK_ENABLE_OS` | `ON` | Enable host OS facilities, including file access and the default debugger transport. |
| `PK_ENABLE_THREADS` | `ON` | Enable thread support and `pkpy.ComputeThread`. |
| `PK_ENABLE_DLL` | `ON` | Enable native dynamic-module loading on supported desktop platforms. |
| `PK_ENABLE_DETERMINISM` | `ON` | Use the project's deterministic math implementation. |
| `PK_ENABLE_WATCHDOG` | `OFF` | Enable execution timeout checks. |
| `PK_ENABLE_CUSTOM_SNAME` | `OFF` | Omit the built-in name implementation; an advanced host integration must supply its own. |
| `PK_ENABLE_MIMALLOC` | `OFF` | Fetch and use mimalloc instead of the default allocator. |

Select at most one library-only option. With both off, the root build creates
the standalone `main` executable and a library.

The following modules are optional and default to `OFF`:

| Option | Module |
| --- | --- |
| `PK_BUILD_MODULE_LZ4` | [lz4](modules/lz4.md) |
| `PK_BUILD_MODULE_CUTE_PNG` | [cute_png](modules/cute_png.md) |
| `PK_BUILD_MODULE_MSGPACK` | [msgpack](modules/msgpack.md) |
| `PK_BUILD_MODULE_PERIPHERY` | [periphery](modules/periphery.md), for Linux hardware I/O |

Initialize the required Git submodules before enabling a module that uses them:

```sh
git submodule update --init --recursive
```

The exact options are defined in
[CMakeOptions.txt](https://github.com/pocketpy/pocketpy/blob/main/CMakeOptions.txt).

## Compiling without the project CMake target

Compile the interpreter as C11, even when your application uses C++. Use an
optimized build with `NDEBUG` for deployment. For MSVC, include `/utf-8` and
`/experimental:c11atomics`.

The defaults in
[config.h](https://github.com/pocketpy/pocketpy/blob/main/include/pocketpy/config.h)
differ from CMake: `PK_ENABLE_THREADS`, `PK_ENABLE_DLL`, and
`PK_ENABLE_DETERMINISM` default to `0` unless explicitly enabled.
Do not assume a manually compiled interpreter has the same features as a CMake
build.

Match the platform libraries and feature definitions in
[CMakeLists.txt](https://github.com/pocketpy/pocketpy/blob/main/CMakeLists.txt).
For example, Windows uses `ws2_32`; enabled thread support needs the platform
thread library, and supported Unix dynamic loading uses `dl`.
Optional modules also need their native sources and libraries; defining a
module macro alone is insufficient.

## Inspect the running build

```python
import pkpy

print(pkpy.configmacros)
print(pkpy.currentvm())  # 0 in the default VM
```

This is useful when a script runs on the desktop but a module or feature is
unavailable in an embedded or browser build. Disabling OS access reduces the
exposed facilities; it does not turn the interpreter into a security sandbox.
