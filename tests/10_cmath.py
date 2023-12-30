assert 1+2j == complex(1, 2) == 2j+1

assert 1+2j + 3+4j == 4+6j

assert 1+2j - 3+4j == -2+6j

assert (1+2j).real == 1
assert (1+2j).imag == 2

assert (1+2j)*(3+4j) == -5+10j
assert (1+2j)*3 == 3+6j

import cmath

res = cmath.sqrt(1+2j)
assert round(res.real, 3) == 1.272, res.real
assert round(res.imag, 3) == 0.786, res.imag
