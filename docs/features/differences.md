---
icon: dot
title: Comparison with CPython
order: 99
---

# Comparison with CPython

pocketpy is designed for embedded scripting. It follows familiar Python syntax
while keeping a smaller runtime and library. Test scripts on the same pocketpy
build that your application ships.

## Porting at a glance

| Area | pocketpy behavior | What to do when porting |
| --- | --- | --- |
| Integers | Signed 64-bit, rather than arbitrary precision | Keep calculations within range; use a host library for larger integers. |
| Booleans | `bool` is separate from `int` | Convert explicitly when arithmetic needs 0 or 1. |
| Function arguments | Parameters without defaults require positional arguments | Check call sites that pass every argument by name. |
| Inheritance | One base class | Use composition instead of multiple inheritance. |
| Descriptors | General `__get__`/`__set__` protocol unavailable | Use supported `property` bindings. |
| In-place operations | `+=` syntax exists; `__iadd__` and similar hooks do not | Do not depend on a custom in-place method being called. |
| Generator expressions | `(value for value in items)` is unsupported | Use a list comprehension or a generator function with `yield`. |
| Bytes | A smaller operation set, including no repetition with `*` | Repeat text before encoding, or construct the required bytes in the host. |
| Exceptions | `try/except` supported; `else` and `finally` clauses unsupported | Arrange explicit cleanup on success and failure. |
| Context managers | Cleanup occurs only when execution reaches the end of the block | Read the cleanup example below. |
| Pattern matching | `match/case` compares values for equality | Use explicit tests for destructuring and type checks. |
| Imports | Host-controlled loading; no `sys.path` search list | Configure the working directory or a [custom loader](../C-API/modules.md). |
| Packages | A selected standard library and native binding API | Check each [module page](../modules/index.md); CPython binary extensions are incompatible. |

Class `__slots__`, Python-level `__del__`, and general metaclass behavior
are not supported. A native type can use a C destructor callback instead.

## Function arguments

A parameter without a default is positional. A parameter with a default may
also be supplied by keyword:

```python
def move(distance, speed=1):
    return distance * speed

assert move(10, speed=2) == 20
assert move(10, 2) == 20

try:
    move(distance=10, speed=2)
except TypeError:
    print('distance must be positional')
```

`*args` and `**kwargs` are available, but they do not make all CPython call
signatures interchangeable. Function defaults are parsed as literals; compute
a dynamic default inside the function, commonly using `None` as a sentinel.

## Numeric types

```python
assert not isinstance(True, int)
assert int(True) == 1
assert type(9223372036854775807) is int
```

The integer range is `-2**63` through `2**63 - 1`.
Do not rely on CPython's arbitrary-precision results or on consistent overflow
exceptions. Floating-point values use C `double`; the
[math module](../modules/math.md) has additional API differences.

## Context managers

On normal completion, pocketpy calls `__exit__()` with **no exception
arguments**. It does not call `__exit__` automatically after `return`,
`break`, `continue`, or an exception leaves the block. It also does not use
its return value to suppress an exception.

This pattern explicitly closes a file on both paths:

```python
# Requires a build with PK_ENABLE_OS enabled.
file = open('example.txt', 'w')
try:
    file.write('Hello\n')
except Exception:
    file.close()
    raise
file.close()
```

For resources owned by C/C++, consider keeping cleanup in the host.
A plain `with` block is suitable only when its limited exit behavior is
acceptable.

## Unpacking and matching

A starred assignment target must be last:

```python
first, *rest = [1, 2, 3]
assert first == 1
assert rest == [2, 3]
```

`first, *middle, last = values` is unsupported. Split that assignment into
ordinary indexing and slicing.

`match` is a keyword, not a soft keyword. Each `case` expression is compared
to the subject, and `case _` is the fallback:

```python
status = 404
match status:
    case 200:
        message = 'ok'
    case 404:
        message = 'not found'
    case _:
        message = 'unknown'
assert message == 'not found'
```

A tuple case compares against that tuple; it is not a set of alternatives.
Capture patterns, class patterns, and CPython's structural matching semantics
are unavailable.

## Strings and indentation

Use four spaces per indentation level. A tab is interpreted as four spaces,
but mixing tabs and spaces makes code harder to port.

In a raw string, escaping the delimiting quote does not work as in CPython.
Choose the other quote style or use an ordinary escaped string.

## Library and native-code compatibility

Pure Python code works only if its syntax and dependencies are supported.
A module called `dataclasses`, `functools`, or `datetime` is a subset
implementation, not a promise of full standard-library compatibility.
Module pages call out important differences and include short examples.

CPython wheels and extension modules cannot be loaded through pocketpy's C API.
Expose native dependencies through [C bindings](../bindings.md) or the
[bundled C++ layer](../bindings-cpp.md). Also read
[unsupported operations](ub.md) before relying on low-level object behavior.
