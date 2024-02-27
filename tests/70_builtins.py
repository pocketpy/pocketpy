assert round(23.2) == 23
assert round(23.8) == 24
assert round(-23.2) == -23
assert round(-23.2, 0) == -23.0
assert round(-23.8) == -24
assert round(23.2, 0) == 23.0
assert round(23.8, 0) == 24.0
assert round(-23.2, 0) == -23.0
assert round(-23.8, 0) == -24.0
# round with precision
assert round(23.2, 1) == 23.2
assert round(23.8, 1) == 23.8
assert round(-23.2, 1) == -23.2
assert round(-23.8, 1) == -23.8
assert round(3.14159, 4) == 3.1416
assert round(3.14159, 3) == 3.142
assert round(3.14159, 2) == 3.14
assert round(3.14159, 1) == 3.1
assert round(3.14159, 0) == 3
assert round(-3.14159, 4) == -3.1416
assert round(-3.14159, 3) == -3.142
assert round(-3.14159, 2) == -3.14
assert round(-3.14159, 1) == -3.1
assert round(-3.14159, 0) == -3
assert round(11, 0) == 11
assert round(11, 1) == 11
assert round(11, 234567890) == 11

a = [1,2,3,-1]
assert sorted(a) == [-1,1,2,3]
assert sorted(a, reverse=True) == [3,2,1,-1]

assert abs(0) == 0
assert abs(1.0) == 1.0
assert abs(-1.0) == 1.0
assert abs(1) == 1
assert abs(-1) == 1

assert any([1])
assert any([1,False,True])
assert not any([])
assert not any([False])

assert all([])
assert all([True])
assert all([True, 1])
assert not all([False])
assert not all([True, False])
assert not all([False, False])

assert list(enumerate([1,2,3])) == [(0,1), (1,2), (2,3)]
assert list(enumerate([1,2,3], 1)) == [(1,1), (2,2), (3,3)]