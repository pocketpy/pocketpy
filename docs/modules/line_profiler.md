---
icon: package
label: line_profiler
---

Line-by-line profiler for Python.

## Example

```python
from line_profiler import LineProfiler

def my_func():
    a = 0
    for i in range(1000000):
        a += i
    return a

lp = LineProfiler()

lp.add_function(my_func)

lp.runcall(my_func)

lp.print_stats()
```

```txt
Total time: 0.243s
File: 84_line_profiler.py
Function: my_func at line 3
Line #      Hits         Time  Per Hit   % Time  Line Contents
==============================================================
     3                                           def my_func():
     4         1            0        0      0.0      a = 0
     5   1000001           69        0     28.4      for i in range(1000000):
     6   1000001          174        0     71.6          a += i
     7         1            0        0      0.0      return a
```