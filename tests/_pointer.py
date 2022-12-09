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

a = [1, 2, 3]
b = &a
b->append(4)
assert a == [1, 2, 3, 4]

def add(a, b):
    return a+b

p = &add
assert p->__call__(1, 2) == 3
assert p->__call__.__call__.__call__.__call__.__call__(3, 4) == 7

fun = lambda :6
p = &fun
assert p->__call__() == 6
assert p->__call__.__call__.__call__.__call__.__call__() == 6