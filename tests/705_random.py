import random as r

r.seed(10)

for _ in range(100):
    i = r.randint(1, 10)
    assert i <= 10
    assert i >= 1
    i = r.random()
    assert 0.0 <= i <= 1.0
    i = r.uniform(3.0, 9.5)
    assert 3.0 <= i <= 9.5

a = [1, 2, 3, 4]
r.shuffle(a)

for i in range(10):
    assert r.choice(a) in a

for i in range(10):
    assert r.choice(tuple(a)) in a

for i in range(10):
    assert r.choice('hello') in 'hello'

for i in range(10):
    assert r.randint(1, 1) == 1

# test choices
x = (1,)
res = r.choices(x, k=4)
assert (res == [1, 1, 1, 1]), res

w = (1, 2, 3)
assert r.choices([1, 2, 3], (0.0, 0.0, 0.5)) == [3]

try:
    r.choices([1, 2, 3], (0.0, 0.0, 0.5, 0.5))
    exit(1)
except ValueError:
    pass

try:
    r.choices([])
    exit(1)
except IndexError:
    pass

seq = [1, 2, 3, 4]
weights = [0.1, 0.2, 0.2, 0.5]
k = 1000
res = r.choices(seq, weights, k=k)
assert len(res) == k and isinstance(res, list)

max_error = 0.03
for i in range(len(seq)):
    actual_w = res.count(seq[i]) / k
    assert abs(actual_w - weights[i]) < max_error

# test seed
from random import randint, seed
seed(7)
a = randint(1, 100)
b = randint(-2**60, 1)
c = randint(50, 100)

assert (a, b, c) == (16, -418020281577586157, 76)

seed(7)
assert a == randint(1, 100)
assert b == randint(-2**60, 1)
assert c == randint(50, 100)

import random
assert random.Random(7).randint(1, 100) == a

# test getstate/setstate
r = random.Random(7)
for _ in range(5):
    r.random()

state = r.getstate()
assert isinstance(state, bytes)
a = [r.randint(0, 1000) for _ in range(10)]
r.setstate(state)
assert a == [r.randint(0, 1000) for _ in range(10)]

# a state can be moved between generators
other = random.Random(123)
other.setstate(r.getstate())
assert other.random() == r.random()

# `Random(state)` is equivalent to `setstate`
assert random.Random(other.getstate()).random() == other.random()

for bad in [b'', b'123', state[:-1]]:
    try:
        random.Random().setstate(bad)
        exit(1)
    except ValueError:
        pass

try:
    random.Random().setstate(7)
    exit(1)
except TypeError:
    pass

# `mti` must stay within [0, 624+1]
tmp = list(state)
for mti in ([0xFF, 0xFF, 0xFF, 0xFF], [0x72, 0x02, 0, 0]):
    try:
        random.Random().setstate(bytes(tmp[:-4] + mti))
        exit(1)
    except ValueError:
        pass

# module-level generator exposes the same api
random.seed(456)
state = random.getstate()
a = [random.random() for _ in range(5)]
random.setstate(state)
assert a == [random.random() for _ in range(5)]

# test pickle
import pickle

r = random.Random(7)
for _ in range(5):
    r.random()

r2 = pickle.loads(pickle.dumps(r))
assert isinstance(r2, random.Random) and r2 is not r
assert [r.random() for _ in range(10)] == [r2.random() for _ in range(10)]

# an unseeded generator round-trips as unseeded
fresh = random.Random()
assert pickle.loads(pickle.dumps(fresh)).getstate() == fresh.getstate()

# shared references are preserved
res = pickle.loads(pickle.dumps([r, r]))
assert res[0] is res[1]
