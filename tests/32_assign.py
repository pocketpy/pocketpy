a = b = 1,2

assert a == b
assert a == (1,2)

a = 1,2

a = b = c = d = '123'

assert a == b == c == d == '123'