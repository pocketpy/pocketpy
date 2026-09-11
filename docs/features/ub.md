---
icon: dot
title: Undefined Behaviour
---

These are the undefined behaviours of pkpy. The behaviour of pkpy is undefined if you do the following things.

1. Modify or delete an attribute of a builtin type. For example, `int.__add__ = f` or
   `del int.__add__`. The interpreter takes fast paths that assume the arithmetic and
   comparison methods of `int` and `float` are the original ones.
2. Call an unbound method with the wrong type of `self`. For example, `int.__add__('1', 2)`.
3. Type `T`'s `__new__` returns an object that is not an instance of `T`.
4. Call `__new__` with a type that is not a subclass of `type`.

