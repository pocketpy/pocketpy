---
icon: dot
title: Arbitrary Sized Integers
---

Unlike cpython, pkpy's `int` is of limited precision (64-bit).

For arbitrary sized integers, we provide a builtin `long` type, just like python2's `long`.
`long` is implemented via pure python in [_long.py](https://github.com/pocketpy/pocketpy/blob/main/python/_long.py).

### Create a long object

You can use `L` suffix to create a `long` literal from a decimal literal.
Also, you can use `long()` function to create a `long` object from a `int` object or a `str` object.

```python
a = 1000L
b = long(1000)
c = long('1000')
assert a == b == c
```

```python
a = 2L         # use `L` suffix to create a `long` object
print(a ** 1000)
# 10715086071862673209484250490600018105614048117055336074437503883703510511249361224931983788156958581275946729175531468251871452856923140435984577574698574803934567774824230985421074605062371141877954182153046474983581941267398767559165543946077062914571196477686542167660429831652624386837205668069376L
```
