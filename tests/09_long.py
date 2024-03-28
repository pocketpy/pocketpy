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

assert 10000000000000000000000L // 3333L == 3000300030003000300L
assert 10000000000000000000000L % 3333L == 100L
assert 2L ** 100 // 3L ** 50 == 1765780L
assert 2L ** 200 // 3L ** 100 == 3117982410207L
assert 2L ** 500 // 3L ** 200 == 12323863745843010927046405923587284941366070573310012484L
assert 2L ** 500 % 3L ** 200 == 242990057207501525999897657892105676264485903550870122812212566096970021710762636168532352280892L