import math
import pkpy

config = pkpy.configmacros
if config["PK_ENABLE_DETERMINISM"] == 0:
    exit(0)
    
def assertEqual(a, b):
    if a == b:
        return
    print(f'{a} != {b}')
    raise AssertionError

# test constants
assertEqual(math.pi, 3.14159265358979323846)
assertEqual(math.e, 2.7182818284590452354)
assert math.inf, math.inf
assert math.nan != math.nan

# test ceil
assertEqual(math.ceil(math.pi), 4.0)
assertEqual(math.ceil(-math.e), -2.0)
assertEqual(math.ceil(math.inf), math.inf)

# test floor
assertEqual(math.floor(math.pi), 3.0)
assertEqual(math.floor(-math.e), -3.0)

# test trunc
assertEqual(math.trunc(math.pi), 3.0)

# test fabs
assertEqual(math.fabs(math.pi), 3.14159265358979323846)
assertEqual(math.fabs(-math.pi), 3.14159265358979323846)

# test gcd
assertEqual(math.gcd(10, 5), 5)
assertEqual(math.gcd(10, 6), 2)
assertEqual(math.gcd(10, 7), 1)
assertEqual(math.gcd(10, 10), 10)
assertEqual(math.gcd(-10, 10), 10)

# test isfinite, isinf, isnan
assertEqual(math.isfinite(math.pi), 1)
assertEqual(math.isfinite(math.inf), 0)
assertEqual(math.isfinite(math.nan), 0)
assertEqual(math.isinf(math.pi), 0)
assertEqual(math.isinf(math.inf), 1)
assertEqual(math.isinf(math.nan), 0)
assertEqual(math.isnan(math.pi), 0)
assertEqual(math.isnan(math.inf), 0)
assertEqual(math.isnan(math.nan), 1)

# test exp
assertEqual(math.exp(0), 1.0)
assertEqual(math.exp(1), math.e)
assertEqual(math.exp(1.5), 4.481689070338065 - 8.881784197001252e-16)
assertEqual(math.exp(3), 20.08553692318767 - 3.552713678800501e-15)
assertEqual(math.exp(-3), 0.04978706836786394 + 6.938893903907228e-18)
assertEqual(math.exp(-2.253647), 0.1050155336754953 - 1.387778780781446e-17)
assertEqual(math.exp(4.729036), 113.1863980522005 - 4.263256414560601e-14)

# test log series
assertEqual(math.log(0), -math.inf)
assertEqual(math.log(1), 0.0)
assertEqual(math.log(2), 0.69314718055994530942)
assertEqual(math.log(math.e), 1.0)
assertEqual(math.log(10), 2.30258509299404568402)
assertEqual(math.log(28.897124), 3.363742074595449)
assertEqual(math.log2(math.e), 1.4426950408889634074)
assertEqual(math.log2(78.781291), 6.299781153677818)
assertEqual(math.log10(math.e), 0.43429448190325182765)
assertEqual(math.log10(56.907822), 1.755171964426069 + 4.440892098500626e-16)

# test pow
assertEqual(math.pow(2,2), 4.0)
assertEqual(math.pow(1.41421356237309504880, 2), 2.0 + 4.440892098500626e-16)
assertEqual(math.pow(0.70710678118654752440, 2), 0.5000000000000001)
assertEqual(math.pow(-1.255782,-3), -0.5049603042167915)
assertEqual(math.pow(6.127042, 4.071529), 1604.407546456745 + 2.273736754432321e-13)

# test sqrt
assertEqual(math.sqrt(2), 1.41421356237309504880)
assertEqual(math.sqrt(math.pi), 1.772453850905516 - 2.220446049250313e-16)
assertEqual(math.sqrt(125.872509), 11.21929182257062)
assertEqual(math.sqrt(1225.296280), 35.00423231553579)

# test cos, sin, tan
assertEqual(math.cos(0), 1.0)
assertEqual(math.cos(math.pi/2), 6.123233995736766e-17)
assertEqual(math.cos(math.pi), -1.0)
assertEqual(math.cos(-11.352808), 0.3496839289707818 - 5.551115123125783e-17)
assertEqual(math.cos(7.294708), 0.530570640518482)
assertEqual(math.sin(0), 0.0)
assertEqual(math.sin(math.pi/2), 1.0)
assertEqual(math.sin(math.pi), 1.224646799147353e-16 + 2.465190328815662e-32)
assertEqual(math.sin(-2.837592), -0.2993398018896187)
assertEqual(math.sin(9.294782), 0.1296301374714747)
assertEqual(math.tan(0), 0.0)
assertEqual(math.tan(math.pi/2), 1.633123935319537e+16)
assertEqual(math.tan(math.pi), -1.224646799147353e-16 - 2.465190328815662e-32)
assertEqual(math.tan(-4.812975), 9.908188146466314)
assertEqual(math.tan(1.875814), -3.176189742032396 - 4.440892098500626e-16)

# test acos, asin, atan
assertEqual(math.acos(0), 1.570796326794897 - 4.440892098500626e-16)
assertEqual(math.acos(1), 0.0)
assertEqual(math.acos(-0.758293), 2.431486995121896 + 4.440892098500626e-16)
assertEqual(math.acos(0.024758), 1.546035796825635)
assertEqual(math.asin(0), 0.0)
assertEqual(math.asin(1), 1.570796326794897 - 4.440892098500626e-16)
assertEqual(math.asin(-0.225895), -0.2278616865773913 + 2.775557561562891e-17)
assertEqual(math.asin(0.955658), 1.271886195819423 + 4.440892098500626e-16)
assertEqual(math.atan(0), 0.0)
assertEqual(math.atan(1), 0.7853981633974483)
assertEqual(math.atan(-3.758927), -1.310785284610617 - 4.440892098500626e-16)
assertEqual(math.atan(35.789293), 1.542862277280122)

# test atan2
assertEqual(math.atan2(math.pi/4, math.pi/4), 0.7853981633974483)
assertEqual(math.atan2(-math.pi/4, math.pi/4), -0.7853981633974483)
assertEqual(math.atan2(-math.pi/4, -math.pi/4), -2.356194490192345)
assertEqual(math.atan2(math.pi/4, -math.pi/4), 2.356194490192345)
assertEqual(math.atan2(1.573823, 0.685329), 1.160103682924653)
assertEqual(math.atan2(-0.899663, 0.668972), -0.9314162757114095)
assertEqual(math.atan2(-0.762894, -0.126497), -1.735113347173296 - 4.440892098500626e-16)
assertEqual(math.atan2(0.468463, -0.992734), 2.700683410692374 - 4.440892098500626e-16)

# test fsum, sum
fsum_sin = math.fsum([math.sin(i) for i in range(5000)])
fsum_cos = math.fsum([math.cos(i) for i in range(5000, 9999)])
assertEqual(fsum_sin, 1.267667771014267 + 2.220446049250313e-16)
assertEqual(fsum_cos, 1.949547793618193 - 4.440892098500626e-16)
assertEqual(fsum_sin + fsum_cos, 3.21721556463246)
sum_sin = sum([math.sin(i) for i in range(5000)])
sum_cos = sum([math.cos(i) for i in range(5000, 9999)])
assertEqual(sum_sin, 1.267667771014264 - 2.220446049250313e-16)
assertEqual(sum_cos, 1.949547793618197 - 4.440892098500626e-16)
assertEqual(sum_sin + sum_cos, 3.21721556463246 + 4.440892098500626e-16)

# test fmod
assertEqual(math.fmod(-2.0, 3.0), -2.0)
assertEqual(math.fmod(2.0, 3.0), 2.0)
assertEqual(math.fmod(4.0, 3.0), 1.0)
assertEqual(math.fmod(-4.0, 3.0), -1.0)

# test modf
x, y = math.modf(math.pi)
assertEqual(x, 0.14159265358979323846 - 1.110223024625157e-16)
assertEqual(y, 3.0)

x, y = math.modf(-math.e)
assertEqual(x, -0.7182818284590451)
assertEqual(y, -2.0)

# test factorial
assertEqual(math.factorial(0), 1)
assertEqual(math.factorial(1), 1)
assertEqual(math.factorial(2), 2)
assertEqual(math.factorial(3), 6)
assertEqual(math.factorial(4), 24)
assertEqual(math.factorial(5), 120)