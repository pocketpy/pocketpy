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

def f(n):
    for i in range(n):
        yield i
        for j in range(i):
            yield j

t = f(3)
assert list(t) == [0, 1, 0, 2, 0, 1]

def f(n):
    for i in range(n):
        if i == n-1:
            raise ValueError
        yield i

t = f(3)
t = iter(t)
assert next(t) == 0
assert next(t) == 1

try:
    next(t)
    exit(1)
except ValueError:
    pass

try:
    next(t)
    exit(1)
except StopIteration:
    pass

def f():
    yield 1
    yield 2
    return
    yield 3

assert list(f()) == [1, 2]

src = '''
def g():
    yield 1
    yield 2
    return 3
    yield 4
'''

try:
    exec(src)
    exit(1)
except SyntaxError:
    pass
