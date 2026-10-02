---
icon: package
label: cmath
---

# cmath

Complex-number arithmetic implemented in Python. Construct values with
`complex(real, imag=0)` or imaginary literals such as `2j`.

```python
import cmath

value = complex(3, 4)
assert value.real == 3.0
assert value.imag == 4.0
assert value.conjugate() == complex(3, -4)
assert cmath.isclose(cmath.sqrt(complex(-1, 0)), complex(0, 1))
radius, angle = cmath.polar(value)
assert cmath.isclose(cmath.rect(radius, angle), value)
```

| Group | Members |
| --- | --- |
| Coordinates | `phase(z)`, `polar(z)`, `rect(radius, angle)` |
| Powers/logarithms | `exp(z)`, `log(z, base=e)`, `log10(z)`, `sqrt(z)` |
| Trigonometry | `sin`, `cos`, `tan`, `asin`, `acos`, `atan` |
| Hyperbolic functions | `sinh`, `cosh`, `tanh`, `asinh`, `acosh`, `atanh` |
| Classification | `isfinite(z)`, `isinf(z)`, `isnan(z)`, `isclose(a, b)` |
| Constants | `pi`, `e`, `tau`, `inf`, `infj`, `nan`, `nanj` |

Pass complex values explicitly to functions that access `real` and `imag`.
`isclose` checks the real and imaginary parts with [math.isclose](math.md);
it does not accept CPython's tolerance keywords. Do not assume identical
branch-cut or domain-error behavior for all inputs.

Implementation: [cmath.py](https://github.com/pocketpy/pocketpy/blob/main/python/cmath.py).
