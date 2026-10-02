---
icon: book
order: -5
label: Script Style
---

# Script style

These conventions keep pocketpy examples readable and make scripts easier to
share with Python users. Runtime restrictions belong in the
[compatibility guide](features/differences.md); style preferences do not change
language behavior.

## Layout and names

- Use four spaces for indentation and avoid tabs.
- Use `CapitalizedWords` for classes, `lower_case_with_underscores` for
  functions and variables, and `UPPER_CASE` for constants.
- Give public parameters descriptive names. Short names such as `i` or
  `x` are appropriate when their meaning is clear locally.
- Avoid shadowing built-ins such as `list`, `type`, and `map`.
- Prefix internal helpers with a single underscore. Reserve special
  `__name__` forms for the runtime protocols they implement.

## Functions and comments

Use type annotations where they clarify an interface. Keep docstrings short:
state what the function does and describe non-obvious constraints or effects.
Explain why a step exists instead of narrating obvious operations.

```python
def clamp(value: float, minimum: float = 0.0, maximum: float = 1.0) -> float:
    """Limit a value to the inclusive interval [minimum, maximum]."""
    if minimum > maximum:
        raise ValueError('minimum must not exceed maximum')
    return min(max(value, minimum), maximum)

assert clamp(1.5) == 1.0
```

Put spaces after commas and around binary operators. For parameters with type
annotations, use spaces around the default `=`; for calls, write
`clamp(0.5, maximum=0.8)`.

Choose either string quote style consistently, switching when that avoids
escaping. Use triple-quoted docstrings.

## Imports and conditions

Group imports by standard modules, optional/native dependencies, then your
application modules. Prefer explicit imports over wildcard imports.

Use `is None`/`is not None` for the absence of a value.
Use direct truth tests for booleans and empty containers:

```python
items = [1, 2, 3]
value = None

if items:
    print(len(items))
if value is None:
    value = 0
```

Keep resource ownership visible. Explicit cleanup may be necessary because of
pocketpy's [context manager behavior](features/differences.md#context-managers).
For native contribution conventions, consult the repository's
[contribution guide](https://github.com/pocketpy/pocketpy/blob/main/CONTRIBUTING.md).
