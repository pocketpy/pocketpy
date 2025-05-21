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
4. Inplace methods like `__iadd__` and `__imul__`.
5. `__del__` in class definition.
6. Multiple inheritance.

## Different behaviors

1. positional and keyword arguments are strictly evaluated.
2. `int` does not derive from `bool`.
3. `int` is 64-bit.
4. Raw string cannot have boundary quotes in it, even escaped. See [#55](https://github.com/pocketpy/pocketpy/issues/55).
5. In a starred unpacked assignment, e.g. `a, b, *c = x`, the starred variable can only be presented in the last position. `a, *b, c = x` is not supported.
6. A `Tab` is equivalent to 4 spaces. You can mix `Tab` and spaces in indentation, but it is not recommended.
7. A return, break, continue in try/except/with block will make the finally block not executed.
8. `match` is a keyword and `match..case` is equivalent to `if..elif..else`.
