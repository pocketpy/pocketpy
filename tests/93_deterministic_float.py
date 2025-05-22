import math

assert math.pi == 3.14159265358979323846
assert math.e == 2.7182818284590452354
assert math.inf == math.inf
assert math.nan != math.nan

assert math.ceil(math.pi) == 4.0
assert math.ceil(-math.e) == -2.0
assert math.ceil(math.inf) == math.inf
assert math.fabs(math.pi) == 3.14159265358979323846
assert math.fabs(-math.pi) == 3.14159265358979323846
assert math.floor(math.pi) == 3.0
assert math.floor(-math.e) == -3.0
assert math.trunc(math.pi) == 3.0

assert math.gcd(10, 5) == 5
assert math.gcd(10, 6) == 2
assert math.gcd(10, 7) == 1
assert math.gcd(10, 10) == 10
assert math.gcd(-10, 10) == 10

assert math.isfinite(math.pi) == 1
assert math.isfinite(math.inf) == 0
assert math.isfinite(math.nan) == 0
assert math.isinf(math.pi) == 0
assert math.isinf(math.inf) == 1
assert math.isinf(math.nan) == 0
assert math.isnan(math.pi) == 0
assert math.isnan(math.inf) == 0
assert math.isnan(math.nan) == 1

assert math.isclose(math.pi, 3.14159265358979323846)
assert math.isclose(math.sqrt(3), 1.732050807568877)

assert math.exp(0) == 1.0
assert math.exp(1) == math.e
assert math.exp(1.5) == 4.481689070338065 - 8.881784197001252e-16
assert math.exp(3) == 20.08553692318767 - 3.552713678800501e-15
assert math.exp(-3) == 0.04978706836786394 + 6.938893903907228e-18
assert math.log(0) == -math.inf
assert math.log(1) == 0.0
assert math.log(2) == 0.69314718055994530942
assert math.log(math.e) == 1.0
assert math.log(10) == 2.30258509299404568402
assert math.log2(math.e) == 1.4426950408889634074
assert math.log10(math.e) == 0.43429448190325182765

assert math.pow(2,2) == 4.0
assert math.pow(1.41421356237309504880, 2) == 2.0 + 4.440892098500626e-16
assert math.pow(0.70710678118654752440, 2) == 0.5000000000000001
assert math.sqrt(2) == 1.41421356237309504880
assert math.sqrt(math.pi) == 1.772453850905516 - 2.220446049250313e-16

assert math.cos(0) == 1.0
assert math.cos(math.pi/2) == 6.123233995736766e-17
assert math.cos(math.pi) == -1.0
assert math.sin(0) == 0.0
assert math.sin(math.pi/2) == 1.0
assert math.sin(math.pi) == 1.224646799147353e-16 + 2.465190328815662e-32
assert math.tan(0) == 0.0
assert math.tan(math.pi/2) == 1.633123935319537e+16
assert math.tan(math.pi) == -1.224646799147353e-16 - 2.465190328815662e-32

assert math.acos(0) == 1.570796326794897 - 4.440892098500626e-16
assert math.acos(1) == 0.0
assert math.asin(0) == 0.0
assert math.asin(1) == 1.570796326794897 - 4.440892098500626e-16
assert math.atan(0) == 0.0
assert math.atan(1) == 0.7853981633974483

assert math.atan2(math.pi/4, math.pi/4) == 0.7853981633974483
assert math.atan2(-math.pi/4, math.pi/4) == -0.7853981633974483
assert math.atan2(-math.pi/4, -math.pi/4) == -2.356194490192345
assert math.atan2(math.pi/4, -math.pi/4) == 2.356194490192345

assert math.fsum([math.sin(i) for i in range(5000)] + [math.cos(i) for i in range(5000, 9999)]) == 3.217215564632461 - 4.440892098500626e-16
assert sum([math.sin(i) for i in range(5000)] + [math.cos(i) for i in range(5000, 9999)]) == 3.21721556463248

assert math.fmod(-2.0, 3.0) == -2.0
assert math.fmod(2.0, 3.0) == 2.0
assert math.fmod(4.0, 3.0) == 1.0
assert math.fmod(-4.0, 3.0) == -1.0

x, y = math.modf(math.pi)
assert x == 0.14159265358979323846 - 1.110223024625157e-16
assert y == 3.0

x, y = math.modf(-math.e)
assert x == -0.7182818284590451
assert y == -2.0

assert math.factorial(0) == 1
assert math.factorial(1) == 1
assert math.factorial(2) == 2
assert math.factorial(3) == 6
assert math.factorial(4) == 24
assert math.factorial(5) == 120
