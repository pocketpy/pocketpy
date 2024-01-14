---
icon: dot
title: Undefined Behaviour
---

These are the undefined behaviours of pkpy. The behaviour of pkpy is undefined if you do the following things.

1. Delete a builtin object. For example, `del int.__add__`.
2. Call an unbound method with the wrong type of `self`. For example, `int.__add__('1', 2)`.
3. Type `T`'s `__new__` returns an object that is not an instance of `T`.
4. Call `__new__` with a type that is not a subclass of `type`.
5. `__eq__`, `__lt__` or `__contains__`, etc.. returns a value that is not a boolean.
