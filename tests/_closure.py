# only one level nested closure is implemented

def f0(a, b):
    def f1():
        return a + b
    return f1

a = f0(1, 2)
assert a() == 3


def f0(a, b):
    def f1():
        a = 5   # use this first
        return a + b
    return f1

a = f0(1, 2)
assert a() == 7