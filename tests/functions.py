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

def fact(n):
    if n == 1:
        return 1
    return n * fact(n - 1)
assert fact(5)==120    

