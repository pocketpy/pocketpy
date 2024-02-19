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

l = (1,2,3,4)
assert l[2] == 3
assert l[-1] == 4
assert l[:32] == (1,2,3,4)
assert l[32:] == tuple([])
assert l[1:4] == (2,3,4)
assert l[-1:-3] == tuple([])
assert l[-3:-1] == (2,3)

l1 = [1];l2 = l1;l1.append(2);l3 = [1,1,2]
assert l2[1] == 2
assert l1 == l2
assert l1*3 == [1,2,1,2,1,2]
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

# test sort
a = [8, 2, 4, 2, 9]
assert sorted(a) == [2, 2, 4, 8, 9]
assert sorted(a, reverse=True) == [9, 8, 4, 2, 2]

assert sorted(a, key=lambda x:-x, reverse=True) == [2, 2, 4, 8, 9]
assert a == [8, 2, 4, 2, 9]

b = [(1, 2), (3, 3), (5, 1)]
b.sort(key=lambda x:x[1])
assert b == [(5, 1), (1, 2), (3,3)]

# unpacking builder
a = [1, 2, 3]
b = [*a, 4, 5]
assert b == [1, 2, 3, 4, 5]

a = []
b = [*a, 1, 2, 3, *a, *a]
assert b == [1, 2, 3]

assert b[
    1
] == 2

assert b[0
] == 1

assert b[0] == 1
assert b[
    0] == 1

a = []
a.append(0)
a.append([1, 2, a])

assert repr(a) == "[0, [1, 2, [...]]]"

# slice extras
class A:
    def __getitem__(self, index):
        return index
    
assert A()[1:2, 3] == (slice(1, 2, None), 3)
assert A()[1:2, 3:4] == (slice(1, 2, None), slice(3, 4, None))
assert A()[1:2, 3:4, 5] == (slice(1, 2, None), slice(3, 4, None), 5)
assert A()[:, :] == (slice(None, None, None), slice(None, None, None))
assert A()[::, :] == (slice(None, None, None), slice(None, None, None))
assert A()[::, :2] == (slice(None, None, None), slice(None, 2, None))
assert A()['b':'c':1, :] == (slice('b', 'c', 1), slice(None, None, None))
assert A()[1:2, :A()[3:4, ::-1]] == (slice(1, 2, None), slice(None, (slice(3, 4, None), slice(None, None, -1)), None))

a = [1, 2, 3]
assert a.index(2) == 1
assert a.index(1) == 0
assert a.index(3) == 2

assert a.index(2, 1) == 1
assert a.index(1, 0) == 0

try:
    a.index(1, 1)
    exit(1)
except ValueError:
    pass
