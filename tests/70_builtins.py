assert round(23.2) == 23
assert round(23.8) == 24
assert round(-23.2) == -23
assert round(-23.8) == -24

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