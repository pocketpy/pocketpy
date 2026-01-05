from py_compile import compile

try:
    import os
except ImportError:
    print('os is not enabled, skipping test...')
    exit(0)

compile('python/heapq.py', 'heapq1.pyc')
assert os.path.exists('heapq1.pyc')

import heapq1
import heapq

a = [1, 2, -3, 2, 1, 5, 11, 123] * 10
b = a.copy()

heapq.heapify(a)
heapq1.heapify(b)

assert a == b

os.remove('heapq1.pyc')
