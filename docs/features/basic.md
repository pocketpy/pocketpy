---
icon: dot
title: Language Features
order: 100
---

# Language features

pocketpy supports the core Python constructs used in application scripts.
The [compatibility guide](differences.md) explains where their behavior differs
from CPython; support for a construct does not imply support for every Python
variant of it.

| Area | Supported examples |
| --- | --- |
| Control flow | `if/elif/else`, `for`, `while`, `break`, `continue` |
| Containers | `list`, `tuple`, `dict`, `set`, slicing, comprehensions |
| Functions | `def`, `lambda`, defaults, `*args`, `**kwargs`, decorators, closures |
| Classes | Single inheritance, methods, `super()`, `property`, static and class methods |
| Iteration | `iter()`, `next()`, generators with `yield` and `yield from` |
| Errors | `raise`, `try/except`, exception subclasses |
| Modules | `import`, `from ... import`, relative imports |
| Strings | UTF-8 strings, formatting, f-strings |
| Other syntax | Unpacking, type annotations, `with`, simplified `match/case` |
| Runtime access | `eval()`, `exec()`, `getattr()`, `setattr()`, `hasattr()` |

## Functions, data, and iteration

```python
def scaled(values, factor=2):
    for value in values:
        yield value * factor

scores = [3, 5, 8]
assert list(scaled(scores, factor=3)) == [9, 15, 24]
assert {score: score * score for score in scores} == {3: 9, 5: 25, 8: 64}
```

## Classes and properties

```python
class Player:
    def __init__(self, name):
        self.name = name
        self.score = 0

    @property
    def rank(self):
        return self.score // 100

player = Player('Ada')
player.score = 250
assert player.rank == 2
```

## Special methods

These are the principal special methods recognized by the runtime. Define them
on your own types; do not modify built-in types.

| Purpose | Methods |
| --- | --- |
| Display and identity | `__repr__`, `__str__`, `__hash__` |
| Truth and size | `__bool__`, `__len__` |
| Iteration and membership | `__iter__`, `__next__`, `__contains__` |
| Comparison | `__eq__`, `__ne__`, `__lt__`, `__le__`, `__gt__`, `__ge__` |
| Unary and numeric helpers | `__neg__`, `__abs__`, `__round__`, `__invert__`, `__divmod__` |
| Arithmetic | `__add__`, `__sub__`, `__mul__`, `__truediv__`, `__floordiv__`, `__mod__`, `__pow__`, `__matmul__` |
| Reflected arithmetic | `__radd__`, `__rsub__`, `__rmul__`, `__rtruediv__`, `__rfloordiv__`, `__rmod__`, `__rpow__` |
| Bitwise operations | `__lshift__`, `__rshift__`, `__and__`, `__or__`, `__xor__` |
| Items | `__getitem__`, `__setitem__`, `__delitem__`, `__missing__` |
| Construction and calls | `__new__`, `__init__`, `__call__` |
| Attribute fallback | `__getattr__` |
| Context blocks | `__enter__`, `__exit__`, with [limited cleanup semantics](differences.md#context-managers) |
| Serialization | `__reduce__`; see [pickle](../modules/pickle.md) |

In-place hooks such as `__iadd__` and reflected bitwise hooks such as
`__rand__` are not supported. Attributes such as `__name__` and
`__all__` are metadata, rather than callable special methods.

The complete runtime name list is in
[magics.h](https://github.com/pocketpy/pocketpy/blob/main/include/pocketpy/xmacros/magics.h).
