import sys

sys.setrecursionlimit(10000)

def f(n):
    if n == 8000:
        return -1
    return f(n + 1)

assert f(0) == -1
