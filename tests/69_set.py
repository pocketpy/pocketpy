a = {1, 2, 3}
a |= {2, 3, 4}

assert a == {1, 2, 3, 4}

a = {1, 2, 3}
a &= {2, 3, 4}

assert a == {2, 3}

a = {1, 2, 3}
a ^= {2, 3, 4}

assert a == {1, 4}

a = {1, 2, 3}
a -= {2, 3, 4}

assert a == {1}

a = {1, 2, 3}
a |= {2, 3, 4}

assert a == {1, 2, 3, 4}

a = set([1, 2, 3])
a |= set([2, 3, 4])

assert a == {1, 2, 3, 4}

a.add(5)
assert a == {1, 2, 3, 4, 5}

a.remove(5)
assert a == {1, 2, 3, 4}

a.discard(4)
assert a == {1, 2, 3}

a.discard(4)
assert a == {1, 2, 3}

assert a.union({2, 3, 4}) == {1, 2, 3, 4}
assert a.intersection({2, 3, 4}) == {2, 3}
assert a.difference({2, 3, 4}) == {1}
assert a.symmetric_difference({2, 3, 4}) == {1, 4}

assert a | {2, 3, 4} == {1, 2, 3, 4}
assert a & {2, 3, 4} == {2, 3}
assert a - {2, 3, 4} == {1}
assert a ^ {2, 3, 4} == {1, 4}

a.update({2, 3, 4})
assert a == {1, 2, 3, 4}

assert 3 in a
assert 5 not in a

assert len(a) == 4
a.clear()

assert len(a) == 0
assert a == set()

b = {1, 2, 3}
c = b.copy()

assert b == c
assert b is not c
b.add(4)
assert b == {1, 2, 3, 4}
assert c == {1, 2, 3}

assert type({}) is dict

assert {1,2}.issubset({1,2,3})
assert {1,2,3}.issuperset({1,2})
assert {1,2,3}.isdisjoint({4,5,6})
assert not {1,2,3}.isdisjoint({2,3,4})

# unpacking builder
a = {1, 2, 3}
b = {*a, 4, 5, *a, *a}
assert b == {1, 2, 3, 4, 5}

a = set()
b = {*a, 1, 2, 3, *a, *a}
assert b == {1, 2, 3}