---
icon: home
label: Welcome
---

# pocketpy

pocketpy is a small Python 3.x interpreter written in C11, designed for embedding
in games and other C/C++ applications. The core interpreter has no third-party
dependencies. Optional modules add features such as compression and image I/O.

This documentation describes the **2.2.x API in this repository**. pocketpy
implements a practical subset of Python; a familiar module name does not imply
the full CPython API. Start with the [compatibility guide](features/differences.md)
when porting an existing script.

## Choose a starting point

| I want to... | Read |
| --- | --- |
| Build the interpreter or embed it in an application | [Quick start](quick-start.md) |
| Expose a C function to Python | [C bindings](bindings.md) |
| Bind C++ functions and classes | [C++ bindings](bindings-cpp.md) |
| Manage Python values and call scripts from C | [C API guide](C-API/introduction.md) |
| Find a module and check its limitations | [Module overview](modules/index.md) |
| Debug, profile, or distribute scripts | [Debugging](features/debugging.md), [profiling](features/profiling.md), [bytecode deployment](features/deploy.md) |

## Try a script

```python
def is_prime(value):
    if value < 2:
        return False
    for divisor in range(2, value):
        if value % divisor == 0:
            return False
    return True

print([value for value in range(2, 20) if is_prime(value)])
# [2, 3, 5, 7, 11, 13, 17, 19]
```

The [browser playground](https://pocketpy.github.io/static/web/) runs Python
without installation. The [C examples](https://pocketpy.github.io/examples/)
demonstrate the embedding API. Browser builds may enable different modules from
a desktop build.

## Platforms and integrations

The project targets Windows, Linux, macOS, Android, iOS, Emscripten, and small
Linux devices. A C11 compiler is required; Windows builds use MSVC. The current
runtime requires little-endian hardware. See [build configuration](build.md) for
compiler flags and optional features.

Integrations include [Godot](https://github.com/pocketpy/godot-pocketpy),
[raylib](https://github.com/pocketpy/raylib-bindings), and
[Flutter](https://pub.dev/packages/pocketpy). The
[VS Code extension](https://marketplace.visualstudio.com/items?itemName=pocketpy.pocketpy)
provides debugging and line profiling.

## Moving from 1.x

Version 2 rewrote the interpreter in C11 and replaced the 1.x C++ API. Existing
1.x embedding code needs to be ported. Choose the C API for explicit VM control
or the bundled C\+\+17 binding layer for convenient C\+\+ integration. Script
compatibility is described separately in the [language guide](features/differences.md).

## Community and support

Report issues or contribute through [GitHub](https://github.com/pocketpy/pocketpy).
See the [contribution guide](https://github.com/pocketpy/pocketpy/blob/main/CONTRIBUTING.md)
for development setup. You can support development through
[GitHub Sponsors](https://github.com/sponsors/blueloveTH) or
[Buy Me a Coffee](https://www.buymeacoffee.com/blueloveth).

Special thanks to **[Tesselmax](https://fdtd.io)** for sponsoring pocketpy.
Tesselmax.EM embeds pocketpy in its electromagnetic simulation console to script
geometry, materials, sources, and post-processing.
