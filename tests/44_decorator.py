from functools import cache

@cache
@cache
@cache
def fib(n):
    if n < 2:
        return n
    return fib(n-1) + fib(n-2)

assert fib(32) == 2178309

class A:
    def __init__(self, x):
        self._x = x

    @property
    def x(self):
        return self._x
    
a = A(1)
assert a.x == 1