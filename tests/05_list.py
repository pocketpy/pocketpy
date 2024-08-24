# generate assert test for list

assert [1, 2, 3] == [1, 2, 3]
assert [1, 2, 3] != [1, 2, 4]

# test + *=
assert [1, 2, 3] + [4, 5, 6] == [1, 2, 3, 4, 5, 6]          
assert [1, 2, 3] * 3 == [1, 2, 3, 1, 2, 3, 1, 2, 3]

l = [1,2,3,4]
assert l[2] == 3
assert l[-1] == 4
assert l[:32] == [1,2,3,4]
assert l[32:] == []
assert l[1:4] == [2,3,4]
assert l[-1:-3] == []
assert l[-3:-1] == [2,3]

l1 = [1];l2 = l1;l1.append(2);l3 = [1,1,2]
assert l2[1] == 2
assert l1 == l2
assert l1*3 == [1,2,1,2,1,2]
assert 2*l1 == [1,2,1,2]
assert l3.count(1) == 2

member = ['Tom', 'Sunny', 'Honer', 'Lily']
teacher = [1,2,3]
assert len(member + teacher) == 7
assert member[0] == 'Tom'
assert member[-2] == 'Honer'
assert member[0:3] == ['Tom', 'Sunny', 'Honer']

member.remove('Sunny')
assert member == ['Tom', 'Honer', 'Lily']
member.pop()
assert member == ['Tom', 'Honer']
del member[0]
assert member == ['Honer']
member.append('Jack')
assert member == ['Honer','Jack']
member.extend(teacher)
assert member == ['Honer','Jack',1,2,3]
member.insert(1,'Tom')
assert member == ['Honer','Tom','Jack',1,2,3]
member.clear()
assert member == []
member = teacher.copy()
assert member == [1,2,3]

l = []
l.insert(0, 'l')
l.insert(1, 'l')
l.insert(0, 'h')
l.insert(3, 'o')
l.insert(1, 'e')
assert l == ['h', 'e', 'l', 'l', 'o']
assert l[-2] == 'l'

b = [1, 2, 3]
assert b[
    1
] == 2

assert b[0
] == 1

assert b[0] == 1
assert b[
    0] == 1

a = [1, 2, 3]
assert a.index(2) == 1
assert a.index(1) == 0
assert a.index(3) == 2

assert a.index(2, 1) == 1
assert a.index(1, 0) == 0

a, b = [1, 2]
assert a == 1 and b == 2
assert [1, 2].__getitem__(0) == 1

assert list(range(1, 5)) == [1, 2, 3, 4]
assert list(range(1, 5, 2)) == [1, 3]
assert list(range(5, 1, -1)) == [5, 4, 3, 2]
assert list(range(5, 1, -2)) == [5, 3]

# test sort
a = [8, 2, 4, 2, 9]
assert a.sort() == None
assert a == [2, 2, 4, 8, 9]

a = []
assert a.sort() == None
assert a == []

a = [0, 0, 0, 0, 1, 1, 3, -1]
assert a.sort() == None
assert a == [-1, 0, 0, 0, 0, 1, 1, 3]

a = [3, 2, 2, 1]
a.sort()
assert a == [1, 2, 2, 3]

# test reverse
a = [3, 2, 2, 1]
a.sort(reverse=True)
assert a == [3, 2, 2, 1]

a = [1, 3, 2, 2]
a.sort(reverse=True)
assert a == [3, 2, 2, 1]

# test key
key = lambda x: -x
assert key(1) == -1
a = [1, 3, 2, 2]
a.sort(key=key)
assert a == [3, 2, 2, 1]

a = [1, 3, 2, 2]
a.sort(key=key, reverse=True)
assert a == [1, 2, 2, 3]

# test sorted
a = [8, 2, 4, 2, 9]
assert sorted(a) == [2, 2, 4, 8, 9]
assert sorted(a, reverse=True) == [9, 8, 4, 2, 2]

def key(x): return -x;
assert sorted(a, key=key) == [9, 8, 4, 2, 2]

assert sorted(a, key=key, reverse=True) == [2, 2, 4, 8, 9]
assert a == [8, 2, 4, 2, 9]

# test unpack ex
a, *b = [1,2,3,4]
assert a == 1
assert b == [2,3,4]

a, b, *c = [1, 2]
assert a == 1
assert b == 2
assert c == []

a, b, c, *d = [1, 2, 3, 4, 5]
assert d == [4, 5]

# test repr
assert repr([1, 2, 3]) == "[1, 2, 3]"
assert repr([1, [2, 3], 4]) == "[1, [2, 3], 4]"
assert repr([1, [2, [3, 4]], 5]) == "[1, [2, [3, 4]], 5]"
assert repr([]) == "[]"

# test in and not in
assert 1 in [1, 2, 3]
assert 4 not in [1, 2, 3]

b = [(1, 2), (3, 3), (5, 1)]
b.sort(key=lambda x:x[1])
assert b == [(5, 1), (1, 2), (3,3)]

# test cyclic reference
# a = []
# a.append(0)
# a.append([1, 2, a])

# assert repr(a) == "[0, [1, 2, [...]]]"

# unpacking builder (not supported)
# a = [1, 2, 3]
# b = [*a, 4, 5]
# assert b == [1, 2, 3, 4, 5]

# a = []
# b = [*a, 1, 2, 3, *a, *a]
# assert b == [1, 2, 3]