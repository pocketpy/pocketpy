a = [1, 2, 3]
a = iter(a)

total = 0

while True:
    try:
        obj = next(a)
    except StopIteration:
        break
    total += obj

assert total == 6

class Task:
    def __init__(self, n):
        self.n = n

    def __iter__(self):
        self.i = 0
        return self

    def __next__(self):
        if self.i == self.n:
            raise StopIteration
        self.i += 1
        return self.i

a = Task(3)
assert sum(a) == 6

i = iter(Task(5))
assert next(i) == 1
assert next(i) == 2
assert next(i) == 3
assert next(i) == 4
assert next(i) == 5
try:
    next(i)
    exit(1)
except StopIteration:
    pass

a = iter([1])
assert next(a) == 1

try:
    next(a)
    exit(1)
except StopIteration:
    pass


# --- StopIteration carries the right value ------------------------------
# `py_next` reports exhaustion without building a StopIteration object, so the
# object has to be reconstructed faithfully wherever one is actually observable.

it = iter([1])
assert next(it) == 1
try:
    next(it)
    exit(1)
except StopIteration as e:
    assert e.args == ()
    assert e.value is None
    assert repr(e) == 'StopIteration()'

assert next(iter([]), 'dflt') == 'dflt'

def gen_with_return():
    yield 1
    return 42

it = iter(gen_with_return())
assert next(it) == 1
try:
    next(it)
    exit(1)
except StopIteration as e:
    assert e.args == (42,)
    assert e.value == 42

def gen_bare_return():
    yield 1

it = iter(gen_bare_return())
assert next(it) == 1
try:
    next(it)
    exit(1)
except StopIteration as e:
    assert e.value is None

# `yield from` reads the value out of the exhausted sub-iterator
def outer_with_return():
    got = yield from gen_with_return()
    yield got
assert list(outer_with_return()) == [1, 42]

def outer_bare_return():
    got = yield from gen_bare_return()
    yield got
assert list(outer_bare_return()) == [1, None]

class RaisesWithValue:
    def __iter__(self):
        return self
    def __next__(self):
        raise StopIteration('V')

def outer_user_iter():
    got = yield from RaisesWithValue()
    yield got
assert list(outer_user_iter()) == ['V']

# --- every builtin iterator still terminates ---------------------------
assert list(iter([1, 2])) == [1, 2]
assert list(iter((1, 2))) == [1, 2]
assert list(range(3)) == [0, 1, 2]
assert list('ab') == ['a', 'b']
assert ''.join(iter(['a', 'b'])) == 'ab'

d = {'x': 1, 'y': 2}
assert sorted(d.keys()) == ['x', 'y']
assert sorted(d.values()) == [1, 2]
assert sorted(d.items()) == [('x', 1), ('y', 2)]

# a dict mutated mid-iteration must still be reported as an error
try:
    for k in d:
        d['z'] = 3
    exit(1)
except RuntimeError:
    pass
