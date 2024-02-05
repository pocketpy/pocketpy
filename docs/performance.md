---
icon: zap
order: -10
label: Performance
---

# Performance

Currently, pkpy is as fast as cpython 3.9.
Performance results for cpython 3.9 are applicable to for pkpy.

Here is a benchmark result of `v1.2.6`.
Files are located in `benchmarks/`.

## win32 64-bit cpy39
```
CPython: 3.9.13 (tags/v3.9.13:6de2ca5, May 17 2022, 16:36:42) [MSC v.1929 64 bit (AMD64)]
System: 64-bit
Testing directory: benchmarks/
> benchmarks/fib.py
  cpython:  0.986091s (100%)
  pocketpy: 0.985427s (99.93%)
> benchmarks/loop_0.py
  cpython:  0.515685s (100%)
  pocketpy: 0.344132s (66.73%)
> benchmarks/loop_1.py
  cpython:  0.938407s (100%)
  pocketpy: 0.595634s (63.47%)
> benchmarks/loop_2.py
  cpython:  1.188671s (100%)
  pocketpy: 0.735259s (61.86%)
> benchmarks/loop_3.py
  cpython:  4.957218s (100%)
  pocketpy: 2.314210s (46.68%)
> benchmarks/primes.py
  cpython:  9.146332s (100%)
  pocketpy: 8.507227s (93.01%)
> benchmarks/recursive.py
  cpython:  0.044789s (100%)
  pocketpy: 0.031252s (69.78%)
> benchmarks/simple.py
  cpython:  0.516624s (100%)
  pocketpy: 0.453159s (87.72%)
> benchmarks/sort.py
  cpython:  0.929597s (100%)
  pocketpy: 0.406802s (43.76%)
> benchmarks/sum.py
  cpython:  0.047151s (100%)
  pocketpy: 0.031266s (66.31%)
ALL TESTS PASSED
```

## linux 64-bit cpy38
```
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

## linux 32-bit cpy39
```
CPython: 3.9.18 (main, Aug 26 2023, 11:50:23) [GCC 10.3.1 20211027]
System: 32-bit
Testing directory: benchmarks/
> benchmarks/fib.py
  cpython:  1.967908s (100%)
  pocketpy: 0.960947s (48.83%)
> benchmarks/loop_0.py
  cpython:  1.063461s (100%)
  pocketpy: 0.396626s (37.30%)
> benchmarks/loop_1.py
  cpython:  1.563821s (100%)
  pocketpy: 0.639663s (40.90%)
> benchmarks/loop_2.py
  cpython:  2.626848s (100%)
  pocketpy: 0.757444s (28.83%)
> benchmarks/loop_3.py
  cpython:  13.428345s (100%)
  pocketpy: 2.852351s (21.24%)
> benchmarks/primes.py
  cpython:  18.174904s (100%)
  pocketpy: 8.423515s (46.35%)
> benchmarks/recursive.py
  cpython:  0.025673s (100%)
  pocketpy: 0.012470s (48.57%)
> benchmarks/simple.py
  cpython:  1.042090s (100%)
  pocketpy: 0.480013s (46.06%)
> benchmarks/sort.py
  cpython:  0.989279s (100%)
  pocketpy: 0.379171s (38.33%)
> benchmarks/sum.py
  cpython:  0.024227s (100%)
  pocketpy: 0.012477s (51.50%)
ALL TESTS PASSED
```

See [actions/runs](https://github.com/pocketpy/pocketpy/actions/runs/6511071423/job/17686074263).

## Primes benchmarks

These are the results of the primes benchmark on Intel i5-12400F, WSL (Ubuntu 20.04 LTS).

| name | version | time | file |
| ---- | ---- | ---- | ---- |
| c++ | gnu++11 | `0.104s ■□□□□□□□□□□□□□□□` | [benchmarks/primes.cpp](https://github.com/pocketpy/pocketpy/blob/9481d653b60b81f4590a4d48f2be496f6962261e/benchmarks/primes.cpp) |
| lua | 5.3.3 | `1.576s ■■■■■■■■■□□□□□□□` | [benchmarks/primes.lua](https://github.com/pocketpy/pocketpy/blob/9481d653b60b81f4590a4d48f2be496f6962261e/benchmarks/primes.lua) |
| pkpy | 1.2.7 | `2.385s ■■■■■■■■■■■■■□□□` | [benchmarks/primes.py](https://github.com/pocketpy/pocketpy/blob/9481d653b60b81f4590a4d48f2be496f6962261e/benchmarks/primes.py) |
| cpython | 3.8.10 | `2.871s ■■■■■■■■■■■■■■■■` | [benchmarks/primes.py](https://github.com/pocketpy/pocketpy/blob/9481d653b60b81f4590a4d48f2be496f6962261e/benchmarks/primes.py) |

```sh
$ time lua benchmarks/primes.lua 

real    0m1.576s
user    0m1.514s
sys     0m0.060s
$ time ./main benchmarks/primes.py 

real    0m2.385s
user    0m2.247s
sys     0m0.100s
$ time python benchmarks/primes.py 

real    0m2.871s
user    0m2.798s
sys     0m0.060s
```