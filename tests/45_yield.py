def f(n):
    for i in range(n):
        yield i

x = 0
for j in f(5):
    x += j

assert x == 10

a = [i for i in f(6)]

assert a == [0,1,2,3,4,5]

def f(n):
    for i in range(n):
        for j in range(n):
            yield i, j

a = [i for i in f(3)]
assert len(a) == 9
assert a[0] == (0,0)
assert a[1] == (0,1)
assert a[2] == (0,2)
assert a[3] == (1,0)
assert a[4] == (1,1)
assert a[5] == (1,2)
assert a[6] == (2,0)
assert a[7] == (2,1)
assert a[8] == (2,2)

def g():
    yield from [1, 2, 3]

def f():
    yield from g()

a = [i for i in f()]
assert a == [1, 2, 3]