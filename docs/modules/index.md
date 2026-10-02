---
title: Module Reference
label: Modules
icon: package
order: 10
---

# Module reference

pocketpy includes a focused set of Python modules and game-oriented extensions.
These pages describe the APIs shipped in this repository, including differences
from CPython. They are not a reference to the full Python standard library.

Most native functions accept positional arguments. Examples show supported
call forms; see the [argument rules](../features/differences.md#function-arguments).

## Data and algorithms

| Module | Use |
| --- | --- |
| [bisect](bisect.md) | Binary search and insertion into sorted lists. |
| [collections](collections.md) | Counts, default dictionaries, and double-ended queues. |
| [dataclasses](dataclasses.md) | Small data classes from annotations. |
| [enum](enum.md) | Named constants. |
| [functools](functools.md) | Caching, reductions, and partial calls. |
| [heapq](heapq.md) | Min-heaps and priority queues. |
| [operator](operator.md) | Operators and accessors as functions. |
| [typing](typing.md) | Placeholders for type hints. |

## Numbers, time, and text

| Module | Use |
| --- | --- |
| [math](math.md), [cmath](cmath.md) | Real and complex mathematics. |
| [random](random.md) | Pseudo-random values and independent generators. |
| [time](time.md), [datetime](datetime.md) | Clocks and basic local date/time values. |
| [unicodedata](unicodedata.md) | East Asian character-width classification. |
| [base64](base64.md) | Binary-to-text encoding. |
| [json](json.md) | JSON output and trusted-input expression loading. |
| [pickle](pickle.md) | pocketpy object serialization. |

## Runtime and host integration

| Module | Use |
| --- | --- |
| [gc](gc.md) | Garbage collection control. |
| [importlib](importlib.md) | Source module reloads. |
| [inspect and dis](inspection.md) | Function metadata and bytecode inspection. |
| [os and io](os.md) | Files and host OS access; requires `PK_ENABLE_OS`. |
| [pkpy](pkpy.md) | VM diagnostics, profiling, timeouts, and compute threads. |
| [stdc](stdc.md) | Native memory and typed C storage. |
| [sys](sys.md) | Version, platform, arguments, and recursion limit. |
| [traceback](traceback.md) | Exception reports. |
| [picoterm and conio](console.md) | Terminal text and platform keyboard input. |

## Games and grids

| Module | Use |
| --- | --- |
| [array2d](array2d.md) | Grids, views, masks, and chunked maps. |
| [vmath](vmath.md) | Native vectors, matrices, and colors. |
| [easing](easing.md) | Animation interpolation curves. |

## Optional native modules

These modules default to **off** in CMake. See [build configuration](../build.md)
for enabling them and initializing native dependencies.

| Module | Required option |
| --- | --- |
| [cute_png](cute_png.md) | `PK_BUILD_MODULE_CUTE_PNG=ON` |
| [lz4](lz4.md) | `PK_BUILD_MODULE_LZ4=ON` |
| [msgpack](msgpack.md) | `PK_BUILD_MODULE_MSGPACK=ON` |
| [periphery](periphery.md) | `PK_BUILD_MODULE_PERIPHERY=ON`, Linux target |

A host application may add or replace modules. If an import fails, check the
running build's capabilities and [module loader](../C-API/modules.md).
Installing a CPython package does not automatically make it available to
pocketpy.
