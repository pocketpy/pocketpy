---
icon: package
label: math
---

### `math.pi`

3.141592653589793

### `math.e`

2.718281828459045

### `math.inf`

The `inf`.

### `math.nan`

The `nan`.

### `math.ceil(x)`

Return the ceiling of `x` as a float, the smallest integer value greater than or equal to `x`.

### `math.fabs(x)`

Return the absolute value of `x`.

### `math.floor(x)`

Return the floor of `x` as a float, the largest integer value less than or equal to `x`.

### `math.fsum(iterable)`

Return an accurate floating point sum of values in the iterable. Avoids loss of precision by tracking multiple intermediate partial sums:

```
>>> sum([0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1])
0.9999999999999999
>>> fsum([0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1])
1.0
```

### `math.gcd(a, b)`

Return the greatest common divisor of the integers `a` and `b`.


### `math.isfinite(x)`

Return `True` if `x` is neither an infinity nor a NaN, and `False` otherwise.

### `math.isinf(x)`

Return `True` if `x` is a positive or negative infinity, and `False` otherwise.

### `math.isnan(x)`

Return `True` if `x` is a NaN (not a number), and `False` otherwise.

### `math.isclose(a, b)`

Return `True` if the values `a` and `b` are close to each other and `False` otherwise.

### `math.exp(x)`

Return `e` raised to the power of `x`.

### `math.log(x)`

Return the natural logarithm of `x` (to base `e`).

### `math.log2(x)`

Return the base-2 logarithm of `x`. This is usually more accurate than `log(x, 2)`.

### `math.log10(x)`

Return the base-10 logarithm of `x`. This is usually more accurate than `log(x, 10)`.

### `math.pow(x, y)`

Return `x` raised to the power `y`.

### `math.sqrt(x)`

Return the square root of `x`.

### `math.acos(x)`

Return the arc cosine of `x`, in radians.

### `math.asin(x)`

Return the arc sine of `x`, in radians.

### `math.atan(x)`

Return the arc tangent of `x`, in radians.

### `math.atan2(y, x)`

Return `atan(y / x)`, in radians. The result is between `-pi` and `pi`. The vector in the plane from the origin to point `(x, y)` makes this angle with the positive X axis. The point of `atan2()` is that the signs of both inputs are known to it, so it can compute the correct quadrant for the angle. For example, `atan(1)` and `atan2(1, 1)` are both `pi/4`, but `atan2(-1, -1)` is `-3*pi/4`.

### `math.cos(x)`

Return the cosine of `x` radians.

### `math.sin(x)`

Return the sine of `x` radians.

### `math.tan(x)`

Return the tangent of `x` radians.

### `math.degrees(x)`

Convert angle `x` from radians to degrees.

### `math.radians(x)`

Convert angle `x` from degrees to radians.


### `math.modf(x)`

Return the fractional and integer parts of `x`. Both results carry the sign of `x` and are floats.

### `math.factorial(x)`

Return `x` factorial as an integer.