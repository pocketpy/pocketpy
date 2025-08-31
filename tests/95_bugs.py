# multi-loop generator
out = []
args = map(lambda x: x+1, [1, 2, 3])
for x in args:
    for y in args:
        out.append((x, y))
assert out == [(2, 3), (2, 4)]

# multi loop bug
out = []
a = [1, 2]
for i in a:
    for j in a:
        out.append((i, j))
assert (out == [(1, 1), (1, 2), (2, 1), (2, 2)]), out

# https://github.com/pocketpy/pocketpy/issues/37

mp = map(lambda x:  x**2, [1, 2, 3, 4, 5]  )
assert list(mp) == [1, 4, 9, 16, 25]


assert not 3>4

def f(x):
    if x>1:
        return 1

assert f(2) == 1
assert f(0) == None

a = [1, 2]
b = [3, 4]
assert a.append == a.append
assert a.append is not a.append
assert a.append is not b.append
assert a.append != b.append

inq = 0
if not inq:
    assert True
else:
    assert False

a = object()
b = object()
if a is   not b:
    assert True
if a  ==  0:
    assert False

def g(x):
    return x
def f(x):
    return x

assert (g(1), 2) == (1, 2)
assert (
    g(1),
    2
) == (1, 2)

assert f((
    g(1),
    2
)) == (1, 2)

def f():
    for i in range(4):
        _ = 0
    while i: i-=1
f()

# class A: a=b=1
# class A: a, b = 1, 2

bmi = 0.0

def test(a):
    if a:
        bmi = 1.4
    return f'{bmi:.2f}'

assert test(1) == '1.40'

try:
    x = test(0)
    print('x:', x)
    exit(1)
except UnboundLocalError:
    pass


g = 1
def f():
    global g
    g += 1

f(); f()
assert g == 3


def f(**kw):
    x = 1
    y = 2
    return kw, x, y
assert f(x=4, z=1) == ({'x': 4, 'z': 1}, 1, 2)

def g(**kw):
    x, y = 1, 2
    return kw

ret = g(
    a=1, b=2, c=3, d=4, e=5, f=6, g=7, h=8, i=9,
    j=10, k=11, l=12, m=13, n=14, o=15, p=16, q=17,
    r=18, s=19, t=20, u=21, v=22, w=23, x=24, y=25,
    z=26
)
assert ret == {chr(i+97): i+1 for i in range(26)}

assert g(**ret) == ret
assert g(**g(**ret)) == ret

# other known issues:
# 1. d.extend(d) if d is deque

g = 0
def test():
    global g
    g += 1
    return g

a = [1, 10, 3]
a[test()] += 1
assert (a == [1, 11, 3]), a
assert (g == 1), g

assert list.__new__(list) == []
assert a.__new__ == list.__new__


class A:
    x: list[int] = [i for i in range(1, 4)]

assert A.x == [1, 2, 3]

# stable sort
a = [(0, 1), (1, 1), (1, 2)]
b = sorted(a, key=lambda x: x[0])
if b != [(0, 1), (1, 1), (1, 2)]:
    assert False, b

# https://github.com/pocketpy/pocketpy/issues/367
a = 10 if False else 5
assert a == 5

a, b, c = (1, 2, 3) if True else (4, 5, 6)
assert a == 1
assert b == 2
assert c == 3

# https://github.com/pocketpy/pocketpy/issues/376
xs = [0]
res = []
for x in xs:
    res.append(x)
    if x == 100:
        break
    xs.append(x+1)

assert res == list(range(101))
assert xs == res

# call property
from vmath import vec2

a = vec2(1, 2)
try:
    x = a.x()
    exit(1)
except TypeError:
    pass