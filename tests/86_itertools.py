from itertools import zip_longest

a = [1, 2, 3]
b = [2]

assert list(zip_longest(a, b)) == [(1, 2), (2, None), (3, None)]
