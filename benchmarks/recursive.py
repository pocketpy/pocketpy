import sys

sys.setrecursionlimit(5000)

def f(n):
    if n == 4000:
        return -1
    return f(n + 1)

assert f(0) == -1
