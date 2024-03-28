assert long(123) == long('123') == 123 == 123L

a = long(2)
assert a ** 0 == 1
assert a ** 60 == 1152921504606846976

assert a + 1 == 3
assert a - 1 == 1
assert a * 2 == 4
assert a // 2 == 1

assert -a == -2

assert 1 + a == 3L
assert 1 - a == -1L
assert 2 * a == 4L
assert 10000000000000000000000L // 3333L == 3000300030003000300L
assert 10000000000000000000000L % 3333L == 100L

# __lshift__ and __rshift__
for i in range(29):
    assert 1L << i == 2 ** i

for i in range(29):
    assert 2L ** i >> i == 1L

assert 12L >> 100 == 0

a = 32764L
s = []
while a != 0:
    a, r = divmod(a, 10L)
    s.append(r)

assert s == [4, 6, 7, 2, 3]

assert 1 < 2L < 3 < 6.6
assert 1L < 2 < 9.6 >= 7 > 2L
assert 1L < 2 < 3 < 6.6