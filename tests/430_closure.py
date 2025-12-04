# only one level nested closure is implemented

def f0(a, b):
    def f1():
        return a + b
    return f1

a = f0(1, 2)
b = f0(3, 4)
assert a() == 3
assert b() == 7


def f0(a, b):
    def f1():
        a = 5   # use this first
        return a + b
    return f1

a = f0(1, 2)
assert a() == 7

def f3(x, y):
    return lambda z: x + y + z

a = f3(1, 2)
assert a(3) == 6

# closure ex
def f(n):
    def g(x):
        if x==n:
            return n
        return g(x+1)
    return g(0)

assert f(10) == 10

# class closure
class A:
    def g(self, x):
        def f(y):
            return x + y
        return f
    
assert A().g(1)(2) == 3

# closure with yield
def g(x):
    def fx(y):
        yield x
        yield y
        return x + y
    return fx
    
gen = g(1)(2)
assert next(gen) == 1
assert next(gen) == 2
try:
    next(gen)
    assert False
except StopIteration as e:
    assert e.value == 3
