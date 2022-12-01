import random as r, sys as s

for _ in range(100):
    i = r.randint(1, 10)
    assert i <= 10
    assert i >= 1


from sys import version as v

assert type(v) is str

class Context:
    def __init__(self):
        self.x = 0

    def __enter__(self):
        self.x = 1

    def __exit__(self):
        self.x = 2

with Context() as c:
    assert c.x == 1

assert c.x == 2