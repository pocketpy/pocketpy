a = 1
assert a == 1
assert *&a == 1
b = &a
*b = 2
assert a == 2

def swap(a,b):
    t = *a
    *a = *b
    *b = t

def f():
    a,b = 5,6
    swap(&a,&b)
    assert a == 6
    assert b == 5

f()