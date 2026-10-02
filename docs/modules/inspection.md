---
icon: package
label: inspect and dis
---

# inspect and dis

Inspect function metadata and the bytecode that pocketpy executes.

## inspect

| Function | Behavior |
| --- | --- |
| `isgeneratorfunction(obj)` | Whether a Python function or bound method is a generator function. |
| `is_user_defined_type(type)` | Whether a type was created by a Python `class` statement. |
| `signature(callable)` | A `Signature` with an ordered `parameters` dictionary for supported callables. |

```python
import inspect

def countdown(start, step=1):
    while start > 0:
        yield start
        start -= step

assert inspect.isgeneratorfunction(countdown)
signature = inspect.signature(countdown)
assert str(signature) == '(start, step=1)'
assert signature.parameters['step'].default == 1
```

Each `Parameter` exposes `name`, `kind`, and `default`.
`Parameter.empty` marks a missing default. The kind constants use the familiar
`POSITIONAL_ONLY`, `POSITIONAL_OR_KEYWORD`, `VAR_POSITIONAL`,
`KEYWORD_ONLY`, and `VAR_KEYWORD` names.

Signature metadata does not override pocketpy's
[argument rules](../features/differences.md#function-arguments).
In particular, the positional-or-keyword label is not a guarantee that a
required parameter can be passed by name.

Signature support is limited to Python functions and compatible wrappers,
bound methods, and class initializers. Not all built-in/native callables have
signatures. General source inspection, binding arguments to a signature,
and annotation introspection are not implemented.

## dis

`dis.dis(function_or_code)` prints a bytecode listing and returns `None`:

```python
import dis

def double(value):
    return value * 2

dis.dis(double)
```

Opcodes are specific to pocketpy and may change between versions.
Use the output to understand execution, not as a stable serialization format.

Implementations:
[inspect.py](https://github.com/pocketpy/pocketpy/blob/main/python/inspect.py),
[inspect.c](https://github.com/pocketpy/pocketpy/blob/main/src/modules/inspect.c),
[dis.c](https://github.com/pocketpy/pocketpy/blob/main/src/modules/dis.c).
