from cmath import isclose, sqrt

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
