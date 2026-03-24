from cmath import isclose, sqrt, nan, inf, nanj, infj, log
import math

c1 = complex(3, 4)
c2 = complex(2, 4.5)

assert isclose(c1 + 5, complex(8, 4))
assert isclose(c1 + c2, complex(5, 8.5))
assert isclose(c1 - c2, complex(1, -0.5))
assert isclose(c1*4, complex(12, 16))
assert isclose(c1*c2, complex(-12, 21.5))
assert isclose(c2/c1, complex(0.96, 0.22))
assert isclose(c2**2, complex(-16.25, 18))

assert 1+2j == complex(1, 2) == 2j+1

assert isclose(1+2j + 3+4j, 4+6j)

assert isclose(1+2j - 3+4j, -2+6j)

assert (1+2j).real == 1
assert (1+2j).imag == 2

assert isclose((1+2j)*(3+4j), -5+10j)
assert isclose((1+2j)*3, 3+6j)

assert isclose((1+2j)**2, -3+4j)

assert (1+2j).conjugate() == 1-2j

res = sqrt(1+2j)
assert isclose(res, 1.272019649514069+0.7861513777574233j)

assert {1+2j: 1}[1+2j] == 1

assert repr(1+2j) == '(1.0+2.0j)'
assert repr(1+0j) == '(1.0+0.0j)'
assert repr(-1-3j) == '(-1.0-3.0j)'
assert repr(1-3j) == '(1.0-3.0j)'


assert repr(math.nan) == repr(nan) == 'nan'
assert repr(-math.nan) == repr(-nan) == 'nan'
assert repr(math.inf) == repr(inf) == 'inf'
assert repr(-math.inf) == repr(-inf) == '-inf'
assert repr(nanj) == '(0.0+nanj)', nanj
assert repr(-nanj) == '(-0.0+nanj)', -nanj
assert repr(infj) == '(0.0+infj)', infj
assert repr(-infj) == '(-0.0-infj)', -infj

assert math.log(1) == 0.0
assert isclose(log(10+5j), 2.4141568686511508+0.4636476090008061j)

# __rtruediv__, __rpow__, __pos__, __bool__, coercions, __hash__
assert isclose(10 / c1, complex(1.2, -1.6))
assert isclose(2 ** complex(3, 0), complex(8, 0))
ref_i = complex(math.cos(math.log(2)), math.sin(math.log(2)))
assert isclose(2 ** 1j, ref_i)
assert isclose((1 + 1j) ** (0.5 + 0.5j), complex(0.6777725052404345, 0.4306022701168375))

assert not complex(0, 0)
assert complex(0, 1)
assert +c1 == c1

try:
    float(1 + 2j)
    assert False, "expect TypeError for float(complex)"
except TypeError:
    pass

try:
    int(1 + 2j)
    assert False, "expect TypeError for int(complex)"
except TypeError:
    pass

assert len({c1, complex(3, 4)}) == 1
