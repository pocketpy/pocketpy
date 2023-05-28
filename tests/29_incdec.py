a = 1

++a
assert a == 2
++a; ++a; --a;
assert a == 3

def f(a):
    ++a
    ++a
    --a
    return a

assert f(3) == 4
assert f(-2) == -1