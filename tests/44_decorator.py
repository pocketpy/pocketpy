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

class B:
    def __init__(self):
        self._x = 1

    def _x_setter(self, v):
        self._x = v

B.x = property(
        lambda self: self._x,
        B._x_setter
    )

b = B()
assert b.x == 1
b.x = 2
assert b.x == 2