---
icon: package
label: math
---

# math

Real-number functions and constants. Most functions accept an `int` or
`float` and return a `float`; exceptions are noted below.
Arguments to native functions are positional.

## Available API

| Group | Members |
| --- | --- |
| Constants | `pi`, `e`, `inf`, `nan` |
| Rounding | `ceil(x)`, `floor(x)`, `trunc(x)` return integers; `fabs(x)` returns a float. |
| Aggregation | `fsum(values)` for a list of numbers; `gcd(a, b)` for two integers. |
| Classification | `isfinite(x)`, `isinf(x)`, `isnan(x)`, `isclose(a, b)` |
| Powers/logarithms | `exp(x)`, `log(x)`, `log(x, base)`, `log2(x)`, `log10(x)`, `pow(x, y)`, `sqrt(x)`, `cbrt(x)` |
| Trigonometry | `sin(x)`, `cos(x)`, `tan(x)`, `asin(x)`, `acos(x)`, `atan(x)`, `atan2(y, x)` |
| Angles | `degrees(radians)`, `radians(degrees)` |
| Decomposition | `modf(x)`, `fmod(x, y)`, `copysign(x, y)` |
| Combinatorics | `factorial(n)` for a nonnegative integer. |

Trigonometric arguments and results use radians. `modf(x)` returns
`(fractional_part, integer_part)`, both floats; pass a float to this function.
`copysign(x, y)` takes the magnitude of `x` and the sign of `y`.

```python
import math

assert math.ceil(1.2) == 2
assert type(math.floor(1.8)) is int
assert math.gcd(18, 24) == 6
assert math.fsum([0.1] * 10) == 1.0
assert math.isclose(math.sin(math.pi / 2), 1.0)
assert math.modf(2.5) == (0.5, 2.0)
```

## Differences from CPython

- `fsum` accepts a list, not an arbitrary iterable, and uses compensated
  summation rather than CPython's full algorithm.
- `isclose(a, b)` checks `abs(a - b) < 1e-9`. It has no relative or absolute
  tolerance keyword arguments; infinities do not compare close by this rule.
- `gcd` takes exactly two arguments.
- `factorial` is limited by signed 64-bit integers. Keep `n <= 20` for an
  exact representable result.
- Domain handling follows the implementation's math functions and may produce
  NaN or infinity instead of CPython exceptions.

For complex values, use [cmath](cmath.md).
Implementation: [math.c](https://github.com/pocketpy/pocketpy/blob/main/src/modules/math.c).
