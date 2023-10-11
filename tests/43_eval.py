assert eval('1+1') == 2
assert eval('[1,2,3]') == [1,2,3]

def f(x):
    return eval('x')

assert f(1) == 1


a = 0
assert eval('a') == 0

exec('a = 1')
assert a == 1

def f(a):
    exec('a = 3')
    return a

assert f(2) == 3

exec(
    "exec('a = eval(\"3 + 5\")')"
)
assert a == 8

def f():
    b = 1
    exec(
        "exec('b = eval(\"3 + 5\")')"
    )
    assert b == 8

class G: pass

def abc():
    g = G()
    exec('a=1', g.__dict__)
    return g.a

assert abc() == 1