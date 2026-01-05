try:
    import os
except ImportError:
    print('os is not enabled, skipping test...')
    exit(0)

import sys
if sys.platform == 'win32':
    exe_name = 'main.exe'
else:
    exe_name = './main'
assert os.system(f'{exe_name} --compile python/heapq.py heapq1.pyc') == 0
assert os.path.exists('heapq1.pyc')

import heapq1
import heapq

a = [1, 2, -3, 2, 1, 5, 11, 123] * 10
b = a.copy()

heapq.heapify(a)
heapq1.heapify(b)

assert a == b

os.remove('heapq1.pyc')
