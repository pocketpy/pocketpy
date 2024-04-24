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
The easiest way to test a feature is to [try it on your browser](https://pocketpy.dev/static/web/).

## Unimplemented features

1. Descriptor protocol `__get__` and `__set__`. However, `@property` is implemented.
2. `__slots__` in class definition.
3. `else` clause in try..except.
4.  Inplace methods like `__iadd__` and `__imul__`.
5. `__del__` in class definition.
6. Multiple inheritance.

## Different behaviors

1. positional and keyword arguments are strictly evaluated.
2. `++i` and `--j` is an increment/decrement statement, not an expression.
3. `int` does not derive from `bool`.
4. `int` is 64-bit. You can use `long` type explicitly for arbitrary sized integers.
5. `__ne__` is not required. Define `__eq__` is enough.
6. Raw string cannot have boundary quotes in it, even escaped. See [#55](https://github.com/pocketpy/pocketpy/issues/55).
7. In a starred unpacked assignment, e.g. `a, b, *c = x`, the starred variable can only be presented in the last position. `a, *b, c = x` is not supported.
8. A `Tab` is equivalent to 4 spaces. You can mix `Tab` and spaces in indentation, but it is not recommended.
9. `%`, `&`, `//`, `^` and `|` for `int` behave the same as C, not python.
10. `str.split` and `str.splitlines` will remove all empty entries.
11. `__getattr__`, `__setattr__` and `__delattr__` can only be set in cpp.
