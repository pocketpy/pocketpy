"""Fresh integration cases for consumers of dmath, separate from kernel tests."""
import math
import cmath
import operator
from vmath import vec2


def same_float(x, y):
    if math.isnan(y):
        assert math.isnan(x)
    else:
        assert x == y, (x, y)
        if y == 0.0:
            assert math.copysign(1.0, x) == math.copysign(1.0, y)


def test_floating_power_entry_points():
    cases = [(5.1875, -4.75), (-5.1875, 9.0), (-5.1875, 0.75),
             (-0.0, -9.0), (-0.0, 9.0), (17.0, 8192.0),
             (17.0, -8192.0), (1.0, math.nan), (math.nan, 0.0)]
    for x, y in cases:
        reference = math.pow(x, y)
        same_float(x ** y, reference)
        same_float(operator.pow(x, y), reference)


def test_integer_power_policy():
    # Compute reference modular products using small values / signed endpoints.
    cases = [(3, 39, 4052555153018976267), (-3, 39, -4052555153018976267),
             (4, 32, 0), (9223372036854775806, 2, 4),
             (-9223372036854775807, 2, 1)]
    for x, y, expected in cases:
        assert x ** y == expected
        assert type(x ** y) is int
    try:
        operator.pow(0, -9)
        assert False
    except ZeroDivisionError:
        pass


def test_floating_division_consumers():
    for x, y in [(139.875, 7.5), (-139.875, 7.5), (139.875, -7.5),
                 (-139.875, -7.5), (3.125e225, 0.25), (-3.125e225, 0.25)]:
        quotient, remainder = divmod(x, y)
        same_float(x // y, quotient)
        same_float(x % y, remainder)
        assert quotient == math.floor(quotient)
        assert quotient * y + remainder == x
        if remainder:
            assert math.copysign(1.0, remainder) == math.copysign(1.0, y)


def test_complex_and_rotation_consumers():
    for angle in [0.46875, -27.1875, 7.125e17, -2.6875e211]:
        sine, cosine = math.sin(angle), math.cos(angle)
        z = cmath.rect(1.0, angle)
        same_float(z.real, cosine)
        same_float(z.imag, sine)
        z = cmath.exp(complex(0.0, angle))
        same_float(z.real, cosine)
        same_float(z.imag, sine)
        # Vector storage is float32; this checks the combined sincos consumer.
        rotated = vec2(1.0, 0.0).rotate(angle)
        assert abs(rotated.x - cosine) <= 6e-8
        assert abs(rotated.y - sine) <= 6e-8
    same_float(cmath.exp(complex(707.25, 0.0)).real, math.exp(707.25))


test_floating_power_entry_points()
test_integer_power_policy()
test_floating_division_consumers()
test_complex_and_rotation_consumers()
