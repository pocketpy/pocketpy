import random

for _ in range(100):
    i = random.randint(1, 10)
    assert i <= 10
    assert i >= 1