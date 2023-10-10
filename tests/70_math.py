from math import log, log10, log2, sin, cos, tan, e, pi, isnan, isinf, fabs, floor, ceil, sqrt

def isclose(a, b):
    return abs(a-b) < 0.000001

assert isclose(e, 2.718281828459045)
assert isclose(pi, 3.141592653589793)
assert isclose(log(10), 2.302585092994046)
assert isclose(log10(10), 1.0)
assert isclose(log2(10), 3.321928094887362)
assert isclose(sin(0), 0.0)
assert isclose(cos(0), 1.0)
assert isclose(tan(0), 0.0)

a = -0.1
a = a**a
assert isnan(a)
assert not isinf(a)
assert isinf(float("inf"))

assert isclose(fabs(-1.2), 1.2)

assert floor(1.2) == 1
assert floor(-1.2) == -2
assert ceil(1.2) == 2
assert ceil(-1.2) == -1

assert isclose(sqrt(4), 2.0)

import math

# test fsum
assert math.fsum([0.1] * 10) == 1.0

# test gcd
assert math.gcd(10, 5) == 5
assert math.gcd(10, 6) == 2
assert math.gcd(10, 7) == 1
assert math.gcd(10, 10) == 10
assert math.gcd(-10, 10) == 10

# test modf
x, y = math.modf(1.5)
assert isclose(x, 0.5)
assert isclose(y, 1.0)

x, y = math.modf(-1.5)
assert isclose(x, -0.5)
assert isclose(y, -1.0)

# test factorial
assert math.factorial(0) == 1
assert math.factorial(1) == 1
assert math.factorial(2) == 2
assert math.factorial(3) == 6
assert math.factorial(4) == 24
assert math.factorial(5) == 120

