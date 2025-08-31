---
icon: dot
title: Basic Features
order: 100
---

The following table shows the basic features of pkpy with respect to [cpython](https://github.com/python/cpython).

| Name            | Example                         | Supported |
| --------------- | ------------------------------- | --------- |
| If Else         | `if..else..elif`                | ✅       |
| Loop            | `for/while/break/continue`      | ✅       |
| Function        | `def f(x,*args,y=1):`           | ✅       |
| Subclass        | `class A(B):`                   | ✅       |
| List            | `[1, 2, 'a']`                   | ✅       |
| ListComp        | `[i for i in range(5)]`         | ✅       |
| Slice           | `a[1:2], a[:2], a[1:]`          | ✅       |
| Tuple           | `(1, 2, 'a')`                   | ✅       |
| Dict            | `{'a': 1, 'b': 2}`              | ✅       |
| F-String        | `f'value is {x}'`               | ✅       |
| Unpacking       | `a, b = 1, 2`                   | ✅       |
| Star Unpacking  | `a, *b = [1, 2, 3]`             | ✅       |
| Exception       | `raise/try..except..`           | ✅       |
| Dynamic Code    | `eval()/exec()`                 | ✅       |
| Reflection      | `hasattr()/getattr()/setattr()` | ✅       |
| Import          | `import/from..import`           | ✅       |
| Context Block   | `with <expr> as <id>:`          | ✅       |
| Type Annotation | `def f(a:int, b:float=1)`       | ✅       |
| Generator       | `yield i`                       | ✅       |
| Decorator       | `@cache`                        | ✅       |
| Match Case      | `match code: case 200:`         | ✅       |

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
