---
icon: zap
order: -10
label: Performance
---

# Performance

Currently, pkpy is as fast as cpython 3.8.
**Performance results for cpython 3.8 are applicable to for pkpy.**

Here is a benchmark result of `v1.0.0`.
Files are located in `benchmarks/`.

```
Testing directory: benchmarks/
> benchmarks/fib.py
  cpython:  0.695462s (100%)
  pocketpy: 0.606675s (87.23%)
> benchmarks/loop_0.py
  cpython:  0.315025s (100%)
  pocketpy: 0.177018s (56.19%)
> benchmarks/loop_1.py
  cpython:  0.568521s (100%)
  pocketpy: 0.319714s (56.24%)
> benchmarks/loop_2.py
  cpython:  0.802686s (100%)
  pocketpy: 0.426311s (53.11%)
> benchmarks/loop_3.py
  cpython:  3.040100s (100%)
  pocketpy: 1.748905s (57.53%)
> benchmarks/primes.py
  cpython:  6.566063s (100%)
  pocketpy: 5.314596s (80.94%)
> benchmarks/recursive.py
  cpython:  0.020200s (100%)
  pocketpy: 0.004595s (22.75%)
> benchmarks/simple.py
  cpython:  0.375262s (100%)
  pocketpy: 0.283474s (75.54%)
> benchmarks/sort.py
  cpython:  0.327771s (100%)
  pocketpy: 0.242722s (74.05%)
> benchmarks/sum.py
  cpython:  0.020165s (100%)
  pocketpy: 0.004495s (22.29%)
ALL TESTS PASSED
```

See [actions/runs](https://github.com/blueloveTH/pocketpy/actions/runs/5113363233/jobs/9192476164).