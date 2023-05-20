---
icon: zap
order: -1
label: Performance
---

# Performance

Currently, pkpy is a bit faster than cpython 3.8. Here is a benchmark result of a special commit.

Benchmark files are located in `benchmarks/`.

See [actions/runs/5031865496/jobs/9025183674](https://github.com/blueloveTH/pocketpy/actions/runs/5031865496/jobs/9025183674).


```
Testing directory: benchmarks/
> benchmarks/fib.py
  cpython:  0.684102s (100%)
  pocketpy: 0.680497s (99.47%)
> benchmarks/loop_0.py
  cpython:  0.338688s (100%)
  pocketpy: 0.189629s (55.99%)
> benchmarks/loop_1.py
  cpython:  0.569272s (100%)
  pocketpy: 0.332900s (58.48%)
> benchmarks/loop_2.py
  cpython:  0.826183s (100%)
  pocketpy: 0.440160s (53.28%)
> benchmarks/loop_3.py
  cpython:  3.121079s (100%)
  pocketpy: 1.390122s (44.54%)
> benchmarks/primes.py
  cpython:  6.705832s (100%)
  pocketpy: 5.420015s (80.83%)
> benchmarks/recursive.py
  cpython:  0.019430s (100%)
  pocketpy: 0.005298s (27.27%)
> benchmarks/simple.py
  cpython:  0.373461s (100%)
  pocketpy: 0.281169s (75.29%)
> benchmarks/sort.py
  cpython:  0.338689s (100%)
  pocketpy: 0.287986s (85.03%)
> benchmarks/sum.py
  cpython:  0.019819s (100%)
  pocketpy: 0.005130s (25.88%)
ALL TESTS PASSED
```