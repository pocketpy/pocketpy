## Function Tests.

def f1():
    return 'f1'
assert f1() == 'f1'
def f2(a, b, c, d): 
    return c
assert f2('a', 'b', 'c', 'd') == 'c'
def f3(a,b):
    return a - b
assert f3(1,2) == -1

def f4(a,b):
    return a + f3(a, b)

assert f4(1,2) == 0

def fact(n):
    if n == 1:
        return 1
    return n * fact(n - 1)
assert fact(5)==120    

def f(a=1, b=-1):
    return a + b

assert f() == 0
assert f(1, 2) == 3
assert f(-5) == -6
assert f(b=5) == 6
assert f(a=5) == 4
assert f(b=5, a=5) == 10

def f(*args):
    return 10 * sum(args)

assert f(1, 2, 3) == 60

def f(x, *args, y=3):
    i = 0
    for j in args:
        i += j
    return i * y

assert f(10, 1, 2, 3) == 18

def f(a, b, *c, d=2, e=5):
    return a + b + d + e + sum(c)

def g(*args, **kwargs):
    return f(*args, **kwargs)

assert f(1, 2, 3, 4) == 17
assert f(1, 2, 3, 4, 5, 6, 7, 8, 9, 10) == 62
assert f(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, d=1, e=2) == 58
assert f(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, e=1, d=2) == 58
assert f(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, d=1) == 61
assert f(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, e=1) == 58
assert f(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20) == 217
assert f(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, d=1, e=2) == 213

assert g(1, 2, 3, 4) == 17
assert g(1, 2, 3, 4, 5, 6, 7, 8, 9, 10) == 62
assert g(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, d=1, e=2) == 58
assert g(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, e=1, d=2) == 58
assert g(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, d=1) == 61
assert g(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, e=1) == 58

a = 1
b = 2

def f():
    global a, b
    a = 3
    b = 4

f()
assert a == 3
assert b == 4

def g(a, b, *args, c=1, d=2, **kwargs):
    S = a + b + c + d + sum(args)
    return S, kwargs

S, kwargs = g(1, 2, 3, 4, 5, c=4, e=5, f=6)
# a = 1
# b = 2
# c = 4
# d = 2
# sum(args) = 3 + 4 + 5 = 12
# S = 1 + 2 + 4 + 2 + 12 = 21

assert S == 21
assert kwargs == {'e': 5, 'f': 6}

# test tuple defaults

def f(a=(1,)):
    return a
assert f() == (1,)

def f(a=(1,2)):
    return a
assert f() == (1,2)

def f(a=(1,2,3)):
    return a
assert f() == (1,2,3)

def f(a=(1,2,3,)):
    return a
assert f() == (1,2,3)

def f(a=(1,(2,3))):
    return a
assert f() == (1,(2,3))

def f(a=((1,2),3), b=(4,)):
    return a, b

assert f() == (((1,2),3), (4,))

def f(a, b):
    return a + b

try:
    f(a=1)
    exit(1)
except TypeError:
    pass

try:
    f(1)
    exit(1)
except TypeError:
    pass

try:
    f(1, 2, 3)
    exit(1)
except TypeError:
    pass

# empty function
def f(a, b, c):
    pass

assert f(1, 2, 3) == None

class A:
    def f(self, a, b, c):
        pass
    
assert A().f(1, 2, 3) == None
