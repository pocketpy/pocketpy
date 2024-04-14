# test reduce

from functools import reduce, partial

# test reduce
assert reduce(lambda x, y: x + y, [1, 2, 3, 4, 5]) == 15
assert reduce(lambda x, y: x * y, [1, 2, 3, 4, 5]) == 120
assert reduce(lambda x, y: x * y, [1, 2, 3, 4, 5], 10) == 1200

# test partial
def add(a, b):
    return a + b

add_1 = partial(add, 1)

assert add_1(2) == 3
assert add_1(3) == 4

def sub(a, b=1):
    return a - b

sub_10 = partial(sub, b=10)
assert sub_10(20) == 10
assert sub_10(30) == 20

