---
icon: dot
title: Basic Features
order: 100
---

Check this [Cheatsheet](https://reference.pocketpy.dev/python.html)
for a quick overview of the supported features.

The following table shows the basic features of pkpy with respect to [cpython](https://github.com/python/cpython).
The features marked with `YES` are supported, and the features marked with `NO` are not supported.

| Name            | Example                         | Supported |
| --------------- | ------------------------------- | --------- |
| If Else         | `if..else..elif`                | YES       |
| Loop            | `for/while/break/continue`      | YES       |
| Function        | `def f(x,*args,y=1):`           | YES       |
| Subclass        | `class A(B):`                   | YES       |
| List            | `[1, 2, 'a']`                   | YES       |
| ListComp        | `[i for i in range(5)]`         | YES       |
| Slice           | `a[1:2], a[:2], a[1:]`          | YES       |
| Tuple           | `(1, 2, 'a')`                   | YES       |
| Dict            | `{'a': 1, 'b': 2}`              | YES       |
| F-String        | `f'value is {x}'`               | YES       |
| Unpacking       | `a, b = 1, 2`                   | YES       |
| Star Unpacking  | `a, *b = [1, 2, 3]`             | YES       |
| Exception       | `raise/try..catch..finally`     | YES       |
| Dynamic Code    | `eval()/exec()`                 | YES       |
| Reflection      | `hasattr()/getattr()/setattr()` | YES       |
| Import          | `import/from..import`           | YES       |
| Context Block   | `with <expr> as <id>:`          | YES       |
| Type Annotation | `def  f(a:int, b:float=1)`      | YES       |
| Generator       | `yield i`                       | YES       |
| Decorator       | `@cache`                        | YES       |

## Supported magic methods

#### Unary operators

+ `__repr__`
+ `__str__`
+ `__hash__`
+ `__len__`
+ `__iter__`
+ `__next__`
+ `__neg__`

#### Logical operators

+ `__eq__`
+ `__lt__`
+ `__le__`
+ `__gt__`
+ `__ge__`
+ `__contains__`

#### Binary operators

+ `__add__`
+ `__radd__`
+ `__sub__`
+ `__rsub__`
+ `__mul__`
+ `__rmul__`
+ `__truediv__`
+ `__floordiv__`
+ `__mod__`
+ `__pow__`
+ `__matmul__`
+ `__lshift__`
+ `__rshift__`
+ `__and__`
+ `__or__`
+ `__xor__`
+ `__invert__`

#### Indexer

+ `__getitem__`
+ `__setitem__`
+ `__delitem__`

#### Specials

+ `__new__`
+ `__init__`
+ `__call__`
+ `__divmod__`
+ `__enter__`
+ `__exit__`
+ `__name__`
+ `__all__`
