---
icon: dot
title: Comparison with CPython
order: 99
---

[cpython](https://github.com/python/cpython) is the reference implementation of the Python programming language. It is written in C and is the most widely used implementation of Python.

## The design goal

**pkpy aims to be an alternative to lua for
game scripting, not cpython for general purpose programming.**

+ For syntax and semantics, pkpy is designed to be as close to cpython as possible.
+ For ecosystem and others, pkpy is not compatible with cpython.

pkpy supports most of the syntax and semantics of python.
For performance and simplicity, some features are not implemented, or behave differently.

## Unimplemented features
1. `**kwargs` in function definition.
2. `__getattr__` and `__setattr__`.
3. Descriptor protocol `__get__` and `__set__`. However, `@property` is implemented.
4. `__slots__` in class definition.
5. `import` module from a directory with `__init__.py`.
6. One element tuple. `(1,)` is not supported.
7. Unpacking in `list` and `dict` literals, i.e. `[1, 2, *a]`.
8. Access the exception object in try..except.
9. `else` clause in try..except.
10. Inplace methods like `__iadd__` and `__imul__`.
11. `__del__` in class definition.

## Different behaviors
1. positional and keyword arguments are strictly evaluated.
2. When a generator is exhausted, `StopIteration` is returned instead of raised.
3. `++i` and `--j` is an increment/decrement statement, not an expression.
4. `int` and `float` are not subclasses of `bool`.
5. `int` is not of unlimited precision. In 32 bit system, `int` and `float` is 30 bit; in 64 bit system, they are both 62 bit.
6. `__ne__` is not required. Define `__eq__` is enough.