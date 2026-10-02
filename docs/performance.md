---
icon: zap
order: -10
label: Performance
---

# Performance

Measure pocketpy with your application's workload, compiler, target hardware,
and enabled features. Results from one script or an older interpreter version
do not predict the performance of all Python programs.

## Measure a current build

Use a Release build with optimization and `NDEBUG`. Record the pocketpy
revision, compiler flags, CPU, OS, and comparison interpreter version.
Keep inputs identical, run each case several times, and compare medians.

The repository's
[benchmarks](https://github.com/pocketpy/pocketpy/tree/main/benchmarks)
cover loops, recursion, sorting, and other small workloads.
For an application, also measure startup, script/native calls, allocations,
and the longest frame or request time.

Use [line profiling](features/profiling.md) to locate expensive code, then
measure elapsed time again without the profiler. Tracing adds overhead.

## Practical optimization choices

- Move frequently repeated native operations into a larger binding call when
  repeated C/Python boundary crossings dominate.
- Avoid rebuilding unchanged containers inside a frame loop.
- Use [vmath](modules/vmath.md) and [array2d](modules/array2d.md) when they fit
  the data and operations.
- Consider [GC scheduling](modules/gc.md) if collections cause frame spikes.
  Manual scheduling also affects memory growth.
- Use [compute threads](features/threading.md) for independent jobs large enough
  to justify serialization and thread overhead.

Confirm each change with measurements; none is a universal speedup.

## Historical results

The following **1.2.7** results are retained for context. They are not a
benchmark of the current 2.x C11 interpreter.

Machine: Intel i5-12400F, WSL with Ubuntu 20.04 LTS. Workload: the
[primes benchmark at the recorded revision](https://github.com/pocketpy/pocketpy/tree/9481d653b60b81f4590a4d48f2be496f6962261e/benchmarks).

| Implementation | Version / language mode | Elapsed time |
| --- | --- | --- |
| C++ | gnu++11 | 0.104 s |
| Lua | 5.3.3 | 1.576 s |
| pocketpy | 1.2.7 | 2.385 s |
| CPython | 3.8.10 | 2.871 s |

Earlier 1.2.6 comparisons across Windows/Linux and 32/64-bit builds are available
in the [original CI run](https://github.com/pocketpy/pocketpy/actions/runs/6511071423/job/17686074263).
Use these as historical observations, not as a claim of equivalence to a
particular CPython release.
