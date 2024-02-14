import random as r, sys as s

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
