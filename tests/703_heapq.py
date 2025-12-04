from heapq import heapify, heappop, heappush
from random import randint

a = [randint(0, 100) for i in range(1000)]
b = sorted(a)

heapify(a)
for x in b:
    assert heappop(a) == x