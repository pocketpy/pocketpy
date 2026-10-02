---
icon: package
label: vmath
---

# vmath

Native vector, matrix, and color types for game scripts.

| Type | Purpose |
| --- | --- |
| `vec2(x, y)`, `vec3(x, y, z)` | Floating-point vectors. |
| `vec2i(x, y)`, `vec3i(x, y, z)`, `vec4i(x, y, z, w)` | Integer vectors. |
| `mat3x3(...)` | 3 x 3 matrix, including 2D affine transforms. |
| `color32(r, g, b, a)` | Four 8-bit color channels, each from 0 to 255. |

Vector components are read-only. Methods such as `with_x()` return a new
value. Floating vectors use single-precision components, so allow for rounding
when comparing calculated results.

## Vector calculations

```python
from vmath import vec2, vec2i

velocity = vec2(3, 4)
assert velocity.length() == 5.0
position = vec2(10, 20) + velocity * 2
assert position == vec2(16, 28)
assert velocity.with_x(0) == vec2(0, 4)

tile = vec2i(2, 3)
assert tile + vec2i(1, 0) == vec2i(3, 3)
```

Floating vectors provide `dot()`, `length()`, `length_squared()`,
and `normalize()`. Multiplication by another vector is component-wise;
use `dot()` for a dot product. Integer vectors provide integer arithmetic,
dot products, and hashing. Vector components can be unpacked by iteration.

For 2D motion, `vec2.rotate(radians)`, `vec2.angle(from_vector, to_vector)`,
and `vec2.smooth_damp(...)` are available. With a downward screen Y axis,
positive rotation appears clockwise.

## Matrices and colors

```python
from vmath import mat3x3, vec2, color32

transform = mat3x3.trs(vec2(10, 20), 0.0, vec2(2, 2))
assert transform.transform_point(vec2(1, 1)) == vec2(12, 22)
assert transform.transform_vector(vec2(1, 1)) == vec2(2, 2)

red = color32(255, 0, 0, 255)
assert red.r == 255
assert red.with_a(128).a == 128
```

`trs(translation, rotation, scale)` constructs an affine transform.
`transform_point()` includes translation; `transform_vector()` does not.
Matrices support `@`, indexing by `[row, column]`, `determinant()`,
`inverse()`, `copy()`, and the in-place variants `copy_`/`inverse_`.
A singular matrix has no inverse.

`rgb(r, g, b)` creates an opaque color. `rgba(r, g, b, a)` uses a float
alpha in `[0, 1]`, unlike `color32`'s integer alpha.
Colors also support hex conversion, RGB565 conversion, ANSI coloring, and
`alpha_blend()`.

Full signatures and type-specific methods:
[vmath.pyi](https://github.com/pocketpy/pocketpy/blob/main/include/typings/vmath.pyi).
