from py_compile import compile
import os

if not os.path.exists('tmp'):
    os.mkdir('tmp')

compile('python/heapq.py', 'tmp/heapq1.pyc')
assert os.path.exists('tmp/heapq1.pyc')

os.chdir('tmp')
import heapq1
import heapq

a = [1, 2, -3, 2, 1, 5, 11, 123] * 10
b = a.copy()

heapq.heapify(a)
heapq1.heapify(b)

assert a == b

os.remove('heapq1.pyc')
