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
| Other syntax | Unpacking, type annotations, `with` |
| Pattern matching | Integer, float, string and singleton patterns; dotted values; captures; fixed-length sequences; `as`; guards |
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

## Pattern matching

`match/case` supports the following forms:

| Form | Example | Behavior |
| --- | --- | --- |
| Integer and float literals | `case -2:`, `case 1.5:` | Compare the subject with the number using equality. |
| String literals | `case "point":` | Compare the subject with the string using equality. |
| Singleton literals | `case True:`, `case False:`, `case None:` | Compare by identity. |
| Wildcard | `case _:` | Match any value without binding a name. |
| Capture | `case item:` | Match any value and bind it to `item`; a bare name does not look up an existing value. |
| Fixed-length sequences | `case [x, y]:`, `case (x, y):`, `case x, y:` | Match a built-in tuple or list with exactly two items and bind the items. Nested patterns are supported. |
| Dotted values | `case Tags.POINT:` | Look up the dotted name and compare its value with the subject using equality. |
| Aliases | `case [x, y] as pair:` | Bind the whole subject to `pair` after the inner pattern succeeds. |
| Guards | `case [x, y] if x < y:` | Evaluate the condition after matching and binding captures; try the next case if it is false. |

Parentheses around a single pattern group it: `case (item):` captures the
whole subject. A comma makes it a sequence pattern: `case (item,):` matches a
one-item tuple or list. Both `case []:` and `case ():` match an empty tuple or
list. Only built-in tuples and lists match sequence patterns; strings, bytes
and iterators do not.

```python
def evaluate(expr):
    match expr:
        case ("lit", value):
            return value
        case ("+", left, right):
            return evaluate(left) + evaluate(right)
        case _:
            return None

assert evaluate(("+", ("lit", 1), ("lit", 2))) == 3
```

Use a dotted name such as `Tags.POINT` to compare against an existing value.
Use `as` to capture the matched value and `if` to check a condition after the
pattern succeeds:

```python
class Tags:
    POINT = "point"

def positive_point(value):
    match value:
        case (Tags.POINT, x, y) as point if x > 0 and y > 0:
            return point
        case _:
            return None

assert positive_point(("point", 2, 3)) == ("point", 2, 3)
assert positive_point(("point", -2, 3)) is None
```

The subject is evaluated once, then cases are tried in order until a pattern
and its optional guard succeed. A guard is not evaluated when its pattern
fails. Exceptions from dotted value lookup, comparison or guard evaluation
propagate to the caller.

A capture or wildcard without a guard must be the last case because it matches
every subject. This also applies to aliases such as `case _ as item:`. Each
capture name may occur only once within a pattern, and `_` cannot be the target
of `as`.

The literal patterns `True`, `False` and `None` compare by identity, as in
CPython. For example, `case True` does not match `1`, but `case 1` matches
`True` through equality. Dotted value patterns always use equality, even when
the referenced value is `True`, `False` or `None`.

This is a subset of Python pattern matching. OR patterns, starred sequences,
mapping patterns and class patterns are unsupported. The
[CPython comparison guide](differences.md#unpacking-and-matching) lists the
literal syntax and sequence types that are unsupported.

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
