def f(a, b, *args):
    return a + b + sum(args)

assert f(1, 2, 3, 4) == 10

a = [5, 6, 7, 8]
assert f(*a) == 26

def g(*args):
    return f(*args)

assert g(1, 2, 3, 4) == 10
assert g(*a) == 26

def f(a, b, *args, c=16):
    return a + b + sum(args) + c

assert f(1, 2, 3, 4) == 26
assert f(1, 2, 3, 4, c=32) == 42

assert f(*a, c=-26) == 0


a, b = 1, 2
a, b = b, a
assert a == 2
assert b == 1

a, *b = 1, 2, 3, 4
assert a == 1
assert b == [2, 3, 4]

a, *b = [1]
assert a == 1
assert b == []

# test perfect forwarding
def f0(a, b, *args):
    return a + b + sum(args)

x = f0(1, 2, 3, 4)
assert x == 10

a = [1, 2, 3, 4]
x = f0(*a)
assert x == 10

def f1(a):
    return a

try:
    x = f1(*[1, 2, 3, 4])
    exit(1)
except TypeError:
    pass


def g(*args, **kwargs):
    return args, kwargs

def f(a, b, *args, c=1, **kwargs):
    return g(a, b, *args, c=c, **kwargs)

args, kwargs = f(1, 2, 3, 4, c=5, d=6, e=-6.0)
assert args == (1, 2, 3, 4)
assert kwargs == {'c': 5, 'd': 6, 'e': -6.0}