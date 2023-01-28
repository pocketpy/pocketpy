assert eval('1+1') == 2
assert eval('[1,2,3]') == [1,2,3]

def f(x):
    return eval('x')

assert f(1) == 1


a = 0
assert eval('a') == 0

exec('a = 1')
assert a == 1

def f(x):
    exec('a = x')
    return a

assert f(2) == 2

exec(
    "exec('a = eval(\"3 + 5\")')"
)

assert a == 8