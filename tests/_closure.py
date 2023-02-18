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