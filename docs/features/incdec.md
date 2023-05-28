---
icon: dot
title: Increment Statement
---

pkpy provides `++i` and `--j` statements to operate a simple named `int` variable.

+ `++i` is equivalent to `i+=1`, but much faster
+ `--j` is equivalent to `j-=1`, but much faster

## Example

```python
a = 1
++a
assert a == 2

def f(a):
  --a
  return a

assert f(3) == 2
```
