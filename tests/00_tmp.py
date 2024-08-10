from cmath import isclose, sqrt

res = sqrt(1+2j)
assert isclose(res, 1.272019649514069+0.7861513777574233j)

a = 1+2j
{a: 1}