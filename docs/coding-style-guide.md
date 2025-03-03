---
icon: book
order: -5
label: Coding Style Guide
---

## Indentation

Use four spaces for indentation. Do not use `TAB`.

## Strings

```python
# Prefer single quotes for strings
s = 'this is a string'

# Use double quotes only if the string itself contains a single quote
s = "this ' is single quote"
```

## Docstrings

Always use triple quotes for docstrings.

```python
def f():
    """This is a multi-line docstring.

    Here is some content. Docstrings partially support Markdown.
    """

def g():
    """This is a single-line docstring."""
```

Use natural language to describe the function's purpose. Do not enumerate each parameter and return value.

```python
# Correct
def add(a: int, b: int):
    """Add two integers `a` and `b`."""

# Incorrect
def add(a: int, b: int):
    """
    @param a, the first argument
    @param b, the second argument
    @return, the result of a + b
    """
```

## Spaces

```python
# Add a space after `,` or `:`
a, b = 1, 2
c = [1, 2, 3]
d = {'key': 'value'}

# Spaces may be added around operators
res = 1 + 2
if res < 2: pass
# Spaces can also be selectively added to indicate operator precedence
x = x * 2 - 1
hypot2 = x * x + y * y
c = (a + b) * (a - b)

# Add a space after `:` in type annotations
def f(a: int, b: float): ...
def g() -> int: ...

# Add spaces around `=` when specifying default values in function parameters
def f(a: int = 1, b: int | None = None): ...
# However, omit spaces if the parameter has no type annotation
def f(a=1, b=2): pass

# Do not add spaces in keyword arguments when calling functions
print(1, 2, 3, end='', sep=',')
f(a=10, b=20)
```

## Naming Conventions

+ Classes: `CapitalizedWords`
+ Functions and variables: `lower_case_with_underscores`
+ Constants and enums: `UPPER_CASE_WITH_UNDERSCORES` or `CapitalizedWords`
+ Anonymous ordered variables: `_0`, `_1`, `_2`
+ Discarded variables: `_`
+ Some standard library functions: `lowercase`

Here are some commonly used naming conventions:
+ `self`: The first parameter of an instance method
+ `cls`: The first parameter of class methods and `__new__`

### Using Abbreviations

Use abbreviations only for temporary variables and internal implementations.

Abbreviations should be well-established, include key syllables of the original word, and be immediately recognizable.

* `context` -> `ctx` (✔)
* `temporary` -> `tmp` (✔)
* `distribution` -> `dist` (✔)
* `visited` -> `vis` (❌)

```python
# Incorrect: Using abbreviations in public function parameters
def some_pub_fn(ctx, req_id, data):
    pass

# Correct
def some_public_function(context, request_id, data):
    pass
```

### Using Precise Terminology

Naming should convey precise meanings, especially when multiple synonyms exist.

For example, `count`, `size`, and `length` all relate to quantity, but they have different nuances:

+ `count`: Represents a counted value
+ `length`: Represents the number of elements in a container
+ `size`: Represents the byte size of an object

```python
s = 'aaabc⭐'
count = s.count('a')
length = len(s)
size = len(s.encode())

print(f"{s!r} has a length of {length}, a size of {size} bytes, and contains {count} occurrences of 'a'")
# 'aaabc⭐' has a length of 6, a size of 8 bytes, and contains 3 occurrences of 'a'
```

### Using Professional Terminology

+ For item quantities in a game: `quantity` is better than `item_count`
+ For grid counts: `area` (meaning surface area) is better than `grid_count`

### Avoiding Built-in Names

```python
# Incorrect: Overwriting `builtins.map`
map = [[1, 2, 3], [4, 5, 6]]
# Incorrect: Overwriting `builtins.type`
type = some_thing.type
```

### Internal Functions and Classes

Use a single underscore `_` as a prefix for internal functions. Never use a double underscore `__` (except for magic methods).

```python
def _internal_func():
    """This is an internal function."""

class _InternalClass:
    def _internal_f(self): pass
```

## Importing Modules

1. Import standard library modules first.
2. Then import third-party dependencies.
3. Finally, import project-specific modules.

```python
from typing import Any
from collections import deque

from array2d import array2d

from ..utils import logger
```

## Coding Practices

Use `is not` when checking for `None`. Do not explicitly compare with `True` or `False`.

```python
# Correct
if x is not None: pass

# Incorrect
if x != None: pass

# Correct
x = True
if x: pass
if not x: pass

# Incorrect
if x == True: pass
if x is True: pass
if x != False: pass
```

The `if` statement implicitly calls `bool()`, so it can be used to check if a container is empty.

```python
not_empty_list = [1]
not_empty_string = '1'
truth = True

if not_empty_list:
    print('true value')

if not_empty_string:
    print('true value')

if truth:
    print('true value')

# Explicitly checking for emptiness is also valid
if len(not_empty_list) > 0: pass
```

## References

[PEP 8 – Style Guide for Python Code](https://peps.python.org/pep-0008/)
