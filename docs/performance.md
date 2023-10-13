---
icon: zap
order: -10
label: Performance
---

# Performance

Currently, pkpy is as fast as cpython 3.8.
**Performance results for cpython 3.8 are applicable to for pkpy.**

Here is a benchmark result of `v1.2.6`.
Files are located in `benchmarks/`.

```
Run python3 scripts/run_tests.py benchmark
CPython: 3.8.10 (default, May 26 2023, 14:05:08) [GCC 9.4.0]
System: 64-bit
Testing directory: benchmarks/
> benchmarks/fib.py
  cpython:  0.739547s (100%)
  pocketpy: 0.613591s (82.97%)
> benchmarks/loop_0.py
  cpython:  0.356003s (100%)
  pocketpy: 0.224396s (63.03%)
> benchmarks/loop_1.py
  cpython:  0.661924s (100%)
  pocketpy: 0.446577s (67.47%)
> benchmarks/loop_2.py
  cpython:  0.937243s (100%)
  pocketpy: 0.514324s (54.88%)
> benchmarks/loop_3.py
  cpython:  3.671752s (100%)
  pocketpy: 1.876151s (51.10%)
> benchmarks/primes.py
  cpython:  7.293947s (100%)
  pocketpy: 5.427518s (74.41%)
> benchmarks/recursive.py
  cpython:  0.021559s (100%)
  pocketpy: 0.010227s (47.44%)
> benchmarks/simple.py
  cpython:  0.408654s (100%)
  pocketpy: 0.265084s (64.87%)
> benchmarks/sort.py
  cpython:  0.372539s (100%)
  pocketpy: 0.243566s (65.38%)
> benchmarks/sum.py
  cpython:  0.021242s (100%)
  pocketpy: 0.010113s (47.61%)
ALL TESTS PASSED
```

See [actions/runs](https://github.com/blueloveTH/pocketpy/actions/runs/6511071423/job/17686074263).