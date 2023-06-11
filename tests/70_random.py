import random as r, sys as s

for _ in range(100):
    i = r.randint(1, 10)
    assert i <= 10
    assert i >= 1

a = [1, 2, 3, 4]
b = (1, 2, 3)
r.shuffle(a)
r.choice(a)
r.choice(b)

assert 0.0 <= r.random() <= 1.0

r.seed(10)
assert r.randint(1, 1000) == 266
assert r.randint(1, 1000) == 126
