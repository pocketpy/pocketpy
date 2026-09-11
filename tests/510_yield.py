def g():
    yield 1
    yield 2
    yield

a = g()
assert next(a) == 1
assert next(a, None) == 2
assert next(a) == None

try:
    next(a)
    exit(1)
except StopIteration:
    pass

assert next(a, 3) == 3
assert next(a, 4) == 4

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


def f():
    for i in range(5):
        yield str(i)
assert '|'.join(f()) == '0|1|2|3|4'


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

def g():
    yield 1
    yield 2
    return 3
    yield 4

assert StopIteration().value == None
assert StopIteration(3).value == 3

try:
    iter = g()
    assert next(iter) == 1
    assert next(iter) == 2
    next(iter)  # raises StopIteration
    print('UNREACHABLE!!')
    exit(1)
except StopIteration as e:
    assert e.value == 3

def f():
    a = yield from g()
    yield a

assert list(f()) == [1, 2, 3]
# --- PEP 479: a StopIteration escaping a generator body becomes RuntimeError ---
# NOTE: the builtin `iter` is shadowed above, so build iterators via generators
def _exhausted():
    return
    yield

def raises_stop_iteration():
    yield 1
    raise StopIteration

try:
    list(raises_stop_iteration())
    exit(1)
except RuntimeError as e:
    assert str(e) == 'generator raised StopIteration', str(e)

it = raises_stop_iteration()
assert next(it) == 1
try:
    next(it)
    exit(1)
except RuntimeError:
    pass

# the same applies when it comes from an exhausted inner iterator
def drains_inner():
    inner = _exhausted()
    yield 1
    next(inner)

try:
    list(drains_inner())
    exit(1)
except RuntimeError:
    pass

# a generator that catches it itself is unaffected
def catches_it():
    try:
        next(_exhausted())
        exit(1)
    except StopIteration:
        yield 'caught'

assert list(catches_it()) == ['caught']

# `yield from` over a sub-generator that finishes normally is unaffected
def sub_with_return():
    yield 'a'
    return 'R'

def delegates():
    got = yield from sub_with_return()
    yield got

assert list(delegates()) == ['a', 'R']
