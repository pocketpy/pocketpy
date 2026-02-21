import math

# ============================================================
# Helper function for floating point comparison
# ============================================================

def isclose(a, b):
    return abs(a-b) < 0.000001

# ============================================================
# Constants: pi, e, inf, nan
# ============================================================

assert isclose(math.pi, 3.141592653589793)
assert isclose(math.e, 2.718281828459045)
assert math.isinf(math.inf)
assert math.isnan(math.nan)

# ============================================================
# Rounding functions: ceil, floor, trunc, fabs
# ============================================================

# ceil - round up to nearest integer
assert math.ceil(1.2) == 2
assert math.ceil(-1.2) == -1
assert math.ceil(2.0) == 2
assert math.ceil(0) == 0

# floor - round down to nearest integer
assert math.floor(1.2) == 1
assert math.floor(-1.2) == -2
assert math.floor(2.0) == 2
assert math.floor(0) == 0

# trunc - truncate towards zero
assert math.trunc(1.7) == 1
assert math.trunc(-1.7) == -1
assert math.trunc(2.0) == 2
assert math.trunc(0) == 0

# fabs - absolute value (returns float)
assert isclose(math.fabs(-1.2), 1.2)
assert isclose(math.fabs(1.2), 1.2)
assert isclose(math.fabs(0), 0.0)
assert isclose(math.fabs(-0.0), 0.0)

# ============================================================
# Classification functions: isnan, isinf, isfinite, isclose
# ============================================================

# isnan - check if value is NaN
assert math.isnan(float('nan'))
assert math.isnan(math.nan)
assert not math.isnan(1.0)
assert not math.isnan(math.inf)

# isinf - check if value is infinity
assert math.isinf(float('inf'))
assert math.isinf(float('-inf'))
assert math.isinf(math.inf)
assert not math.isinf(1.0)
assert not math.isinf(math.nan)

# isfinite - check if value is finite (not inf or nan)
assert math.isfinite(1.0)
assert math.isfinite(0.0)
assert math.isfinite(-1.0)
assert not math.isfinite(math.inf)
assert not math.isfinite(-math.inf)
assert not math.isfinite(math.nan)

# isclose - check if two values are close
assert math.isclose(1.0, 1.0)
assert math.isclose(1.0, 1.0 + 1e-10)
assert not math.isclose(1.0, 1.1)

# ============================================================
# Power and logarithmic functions: exp, log, log2, log10, pow, sqrt
# ============================================================

# exp - e raised to the power x
assert isclose(math.exp(0), 1.0)
assert isclose(math.exp(1), math.e)
assert isclose(math.exp(2), math.e * math.e)

# log - natural logarithm (base e) or logarithm with custom base
assert isclose(math.log(1), 0.0)
assert isclose(math.log(math.e), 1.0)
assert isclose(math.log(10), 2.302585092994046)
assert isclose(math.log(100, 10), 2.0)  # log base 10
assert isclose(math.log(8, 2), 3.0)     # log base 2

# log2 - logarithm base 2
assert isclose(math.log2(1), 0.0)
assert isclose(math.log2(2), 1.0)
assert isclose(math.log2(8), 3.0)
assert isclose(math.log2(10), 3.321928094887362)

# log10 - logarithm base 10
assert isclose(math.log10(1), 0.0)
assert isclose(math.log10(10), 1.0)
assert isclose(math.log10(100), 2.0)
assert isclose(math.log10(1000), 3.0)

# pow - x raised to the power y
assert isclose(math.pow(2, 3), 8.0)
assert isclose(math.pow(2, 0), 1.0)
assert isclose(math.pow(2, -1), 0.5)
assert isclose(math.pow(4, 0.5), 2.0)
assert isclose(math.pow(0, 0), 1.0)

# sqrt - square root
assert isclose(math.sqrt(4), 2.0)
assert isclose(math.sqrt(2), 1.4142135623730951)
assert isclose(math.sqrt(0), 0.0)
assert isclose(math.sqrt(1), 1.0)

# ============================================================
# Trigonometric functions: sin, cos, tan, asin, acos, atan, atan2
# ============================================================

# sin - sine
assert isclose(math.sin(0), 0.0)
assert isclose(math.sin(math.pi / 2), 1.0)
assert isclose(math.sin(math.pi), 0.0)
assert isclose(math.sin(-math.pi / 2), -1.0)
assert isclose(math.sin(-math.pi / 4), -0.7071067811865476)

# cos - cosine
assert isclose(math.cos(0), 1.0)
assert isclose(math.cos(math.pi / 2), 0.0)
assert isclose(math.cos(math.pi), -1.0)
assert isclose(math.cos(-math.pi), -1.0)

# tan - tangent
assert isclose(math.tan(0), 0.0)
assert isclose(math.tan(math.pi / 4), 1.0)
assert isclose(math.tan(-math.pi / 4), -1.0)

# asin - arc sine (inverse sine)
assert isclose(math.asin(0), 0.0)
assert isclose(math.asin(1), math.pi / 2)
assert isclose(math.asin(-1), -math.pi / 2)
assert isclose(math.asin(0.5), math.pi / 6)

# acos - arc cosine (inverse cosine)
assert isclose(math.acos(1), 0.0)
assert isclose(math.acos(0), math.pi / 2)
assert isclose(math.acos(-1), math.pi)
assert isclose(math.acos(0.5), math.pi / 3)

# atan - arc tangent (inverse tangent)
assert isclose(math.atan(0), 0.0)
assert isclose(math.atan(1), math.pi / 4)
assert isclose(math.atan(-1), -math.pi / 4)

# atan2 - arc tangent of y/x (handles quadrants correctly)
assert isclose(math.atan2(0, 1), 0.0)
assert isclose(math.atan2(1, 0), math.pi / 2)
assert isclose(math.atan2(0, -1), math.pi)
assert isclose(math.atan2(-1, 0), -math.pi / 2)
assert isclose(math.atan2(1, 1), math.pi / 4)
assert isclose(math.atan2(-1, -1), -3 * math.pi / 4)

# ============================================================
# Angle conversion functions: degrees, radians
# ============================================================

# degrees - convert radians to degrees
assert isclose(math.degrees(0), 0.0)
assert isclose(math.degrees(math.pi), 180.0)
assert isclose(math.degrees(math.pi / 2), 90.0)
assert isclose(math.degrees(2 * math.pi), 360.0)

# radians - convert degrees to radians
assert isclose(math.radians(0), 0.0)
assert isclose(math.radians(180), math.pi)
assert isclose(math.radians(90), math.pi / 2)
assert isclose(math.radians(360), 2 * math.pi)

# ============================================================
# Arithmetic functions: fmod, modf, copysign, fsum, gcd, factorial
# ============================================================

# fmod - floating point modulo
assert isclose(math.fmod(5.0, 3.0), 2.0)
assert isclose(math.fmod(-5.0, 3.0), -2.0)
assert isclose(math.fmod(5.0, -3.0), 2.0)
assert isclose(math.fmod(-5.0, -3.0), -2.0)
assert isclose(math.fmod(4.0, 3.0), 1.0)
assert isclose(math.fmod(-4.0, 3.0), -1.0)
assert isclose(math.fmod(-2.0, 3.0), -2.0)
assert isclose(math.fmod(2.0, 3.0), 2.0)

# modf - split into fractional and integer parts
frac, integer = math.modf(1.5)
assert isclose(frac, 0.5)
assert isclose(integer, 1.0)

frac, integer = math.modf(-1.5)
assert isclose(frac, -0.5)
assert isclose(integer, -1.0)

frac, integer = math.modf(2.0)
assert isclose(frac, 0.0)
assert isclose(integer, 2.0)

frac, integer = math.modf(0.0)
assert isclose(frac, 0.0)
assert isclose(integer, 0.0)

# copysign - return x with the sign of y
assert isclose(math.copysign(1.0, -1.0), -1.0)
assert isclose(math.copysign(-1.0, 1.0), 1.0)
assert isclose(math.copysign(1.0, 1.0), 1.0)
assert isclose(math.copysign(-1.0, -1.0), -1.0)
assert isclose(math.copysign(5.0, -0.0), -5.0)
assert isclose(math.copysign(0.0, -1.0), 0.0)  # sign of zero may vary

# fsum - accurate floating point sum
assert math.fsum([0.1] * 10) == 1.0
assert math.fsum([]) == 0.0
assert math.fsum([1.0, 2.0, 3.0]) == 6.0
assert math.fsum([0.1, 0.2, 0.3]) == 0.6

# gcd - greatest common divisor
assert math.gcd(10, 5) == 5
assert math.gcd(10, 6) == 2
assert math.gcd(10, 7) == 1
assert math.gcd(10, 10) == 10
assert math.gcd(-10, 10) == 10
assert math.gcd(10, -10) == 10
assert math.gcd(-10, -10) == 10
assert math.gcd(0, 5) == 5
assert math.gcd(5, 0) == 5
assert math.gcd(0, 0) == 0

# factorial - n!
assert math.factorial(0) == 1
assert math.factorial(1) == 1
assert math.factorial(2) == 2
assert math.factorial(3) == 6
assert math.factorial(4) == 24
assert math.factorial(5) == 120
assert math.factorial(10) == 3628800

# ============================================================
# Special value tests
# ============================================================

# Test NaN generation from invalid operations
a = -0.1
a = a ** a
assert math.isnan(a)
assert not math.isinf(a)

# Test infinity
assert math.isinf(float("inf"))
assert math.isinf(float("-inf"))
assert not math.isnan(float("inf"))