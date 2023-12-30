assert 1+2j == complex(1, 2) == 2j+1

assert 1+2j + 3+4j == 4+6j

assert 1+2j - 3+4j == -2+6j

assert (1+2j).real == 1
assert (1+2j).imag == 2

assert (1+2j)*(3+4j) == -5+10j
assert (1+2j)*3 == 3+6j

import cmath

assert (1+2j)**2 == -3+4j

assert (1+2j).conjugate() == 1-2j

res = cmath.sqrt(1+2j)
assert res == 1.272019649514069+0.7861513777574233j
