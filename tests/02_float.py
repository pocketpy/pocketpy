def eq(a, b):
    dt = a - b
    return dt > -0.001 and dt < 0.001

# test == != >= <= < >
assert 1.0 == 1.0
assert 1.0 != 1.1
assert 1.0 >= 1.0
assert 1.0 <= 1.0
assert 1.0 < 1.1
assert 1.1 > 1.0

# test + - * ** /
assert eq(1.5 + 3, 4.5)
assert eq(1.5 + 3.9, 5.4)
assert eq(5.3 - 2.5, 2.8)
assert eq(0.2**2, 0.04)
assert eq(4**(-1.0), 0.25)
assert eq(2/1, 2.0)
assert eq(3/2.0, 1.5)
assert eq(1/9, 0.11111)

# test += -= *= /=
x = 3.0
x += 1
assert eq(x, 4.0)
x -= 1
assert eq(x, 3.0)
x *= 2
assert eq(x, 6.0)
x /= 1.8
assert eq(x, 3.3333)

# test __str__, __repr__
assert str(1.0) == '1.0'
assert repr(1.0) == '1.0'

# test float()
assert float() == 0.0
assert float(True) == 1.0
assert float(False) == 0.0
assert float(1) == 1.0
assert float(-2) == -2.0

assert eq(float(1), 1.0)
assert eq(float(1.0), 1.0)
assert eq(float(1.1), 1.1)
assert eq(float(1.9), 1.9)
assert eq(float(-1.9), -1.9)
assert eq(float(1.5), 1.5)
assert eq(float(-1.5), -1.5)
assert eq(float("123"), 123.0)
assert eq(float("123.456"), 123.456)


import math

inf = float("inf")
assert 1/0 == inf
assert -1/0 == -inf
assert 1/inf == 0
assert -1/inf == 0
assert math.isnan(0/0)

assert 2**-6000 == 0.0
assert 2.0 ** 6000 == inf
assert (-2.0) ** 6001 == -inf

# test .123 forms
assert float(".123") == 0.123
assert .123 == 0.123
assert eq(.5 *2, 1.0)
assert eq(2 * .5, 1.0)
assert eq(2 * (.5), 1.0)
assert eq(2 * (.5 + 1), 3.0)


assert 1e3 == 1000.0
assert 1e-3 == 0.001
assert -1e3 == -1000.0
assert -1e-3 == -0.001
assert 1e0 == 1.0
assert 1e-0 == 1.0

assert 2e3 == 2000.0
assert 2e3j == 2000j
assert -2e-3 == -0.002
assert -2e-3j == -0.002j

assert 3.4e-3 == 0.0034
assert 3.4e+3 == 3400.0

try:
    float('-x13')
    exit(1)
except ValueError:
    pass
