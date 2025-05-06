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

# test lru_cache
from functools import lru_cache
miss_keys = []

@lru_cache(maxsize=3)
def test_f(x: int):
    miss_keys.append(x)
    return x

assert test_f(1) == 1 and miss_keys == [1]
# [1]
assert test_f(2) == 2 and miss_keys == [1, 2]
# [1, 2]
assert test_f(3) == 3 and miss_keys == [1, 2, 3]
# [1, 2, 3]
assert test_f(1) == 1 and miss_keys == [1, 2, 3]
# [2, 3, 1]
assert test_f(2) == 2 and miss_keys == [1, 2, 3]
# [3, 1, 2]
assert test_f(4) == 4 and miss_keys == [1, 2, 3, 4]
# [1, 2, 4]
assert test_f(3) == 3 and miss_keys == [1, 2, 3, 4, 3]
# [2, 4, 3]
assert test_f(2) == 2 and miss_keys == [1, 2, 3, 4, 3]
# [4, 3, 2]
assert test_f(5) == 5 and miss_keys == [1, 2, 3, 4, 3, 5]
# [3, 2, 5]
assert test_f(3) == 3 and miss_keys == [1, 2, 3, 4, 3, 5]
# [2, 5, 3]
assert test_f(1) == 1 and miss_keys == [1, 2, 3, 4, 3, 5, 1]
# [5, 3, 1]
