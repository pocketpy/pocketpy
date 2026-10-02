---
icon: package
label: pkpy
---

# pkpy

pocketpy-specific runtime controls and diagnostics.

| API | Purpose |
| --- | --- |
| `configmacros` | Dictionary of selected compile-time settings. |
| `currentvm()` | Index of the current VM. |
| `memory_usage()` | Interpreter memory estimate in bytes. |
| `memory_usage_info()` | Human-readable allocation/GC report. |
| `profiler_begin()`, `profiler_end()` | Start/stop line profiling; see the [startup limitation](../features/profiling.md#profile-execution-in-an-embedded-host). |
| `profiler_reset()`, `profiler_report()` | Reset measurements, or stop active profiling and return a report dictionary. |
| `ComputeThread` | Run independent jobs in another VM; requires thread support. |
| `watchdog_begin(timeout)`, `watchdog_end()` | Enable/disable execution timeout checks; requires watchdog support. |
| `TValue[type](value)` | Typed value wrapper with a read-only `value` property. |

```python
import pkpy

assert pkpy.currentvm() == 0
assert type(pkpy.memory_usage()) is int
print(pkpy.memory_usage_info())
assert pkpy.TValue[int](42).value == 42
```

`configmacros` includes `PK_ENABLE_OS`, `PK_ENABLE_THREADS`,
`PK_ENABLE_DETERMINISM`, `PK_ENABLE_WATCHDOG`, `PK_GC_MIN_THRESHOLD`,
and `PK_VM_STACK_SIZE`. It is a diagnostic snapshot; editing it does not
reconfigure the compiled runtime. The memory estimate is not the process's
resident memory usage.

`TValue` provides wrappers for `int`, `float`, `vmath.vec2`, and
`vmath.vec2i`; it is not a general-purpose runtime generic type.

## Watchdog

When built with `PK_ENABLE_WATCHDOG=ON`, `watchdog_begin(timeout)` takes
milliseconds and timeout checks can raise `TimeoutError`.
Call `watchdog_end()` after the protected work on both success and error paths.
These checks do not preempt a blocking native call.

For complete workflows, see [compute threads](../features/threading.md),
[profiling](../features/profiling.md), and [build configuration](../build.md).
API declarations: [pkpy.pyi](https://github.com/pocketpy/pocketpy/blob/main/include/typings/pkpy.pyi).
