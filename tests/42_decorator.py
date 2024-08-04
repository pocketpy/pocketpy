from functools import cache

class A:
    def __init__(self, x):
        self._x = x

    @property
    def x(self):
        return self._x
    
    def __call__(self, b):
        return self.x + b
    
a = A(1)
assert a.x == 1
assert a(2) == 3

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

@cache
def fib(n):
    # print(f'fib({n})')
    if n < 2:
        return n
    return fib(n-1) + fib(n-2)

assert fib(32) == 2178309

def wrapped(cls):
    return int

@wrapped
@wrapped
@wrapped
@wrapped
class A:
    def __init__(self) -> None:
        pass

assert A('123') == 123

# validate the decorator order
res = []

def w(x):
    res.append('w')
    return x

def w1(x):
    res.append('w1')
    return x

def w2(x):
    res.append('w2')
    return x

@w
@w1
@w2
def f():
    pass

f()
assert res == ['w2', 'w1', 'w']
