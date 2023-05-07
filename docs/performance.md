---
icon: zap
order: -1
label: Performance
---

# Performance

Currently, pkpy is almost~ fast as cpython 3.8. Here is a benchmark result of a special commit.

Benchmark files are located in `benchmarks/`.

See [actions/runs/4729609975/jobs/8392313856](https://github.com/blueloveTH/pocketpy/actions/runs/4729609975/jobs/8392313856).


```
Testing directory: benchmarks/
> benchmarks/fib.py
  cpython:  0.686955s (100%)
  pocketpy: 0.652851s (95.04%)
> benchmarks/loop_0.py
  cpython:  0.315438s (100%)
  pocketpy: 0.205589s (65.18%)
> benchmarks/loop_1.py
  cpython:  0.621474s (100%)
  pocketpy: 0.347335s (55.89%)
> benchmarks/loop_2.py
  cpython:  0.842779s (100%)
  pocketpy: 0.465181s (55.20%)
> benchmarks/loop_3.py
  cpython:  3.069278s (100%)
  pocketpy: 1.455937s (47.44%)
> benchmarks/primes.py
  cpython:  6.848963s (100%)
  pocketpy: 13.592313s (198.46%)
> benchmarks/recursive.py
  cpython:  0.020444s (100%)
  pocketpy: 0.004801s (23.48%)
> benchmarks/simple.py
  cpython:  0.372713s (100%)
  pocketpy: 0.273696s (73.43%)
> benchmarks/sort.py
  cpython:  0.324214s (100%)
  pocketpy: 0.464951s (143.41%)
> benchmarks/sum.py
  cpython:  0.019418s (100%)
  pocketpy: 0.004817s (24.80%)
```