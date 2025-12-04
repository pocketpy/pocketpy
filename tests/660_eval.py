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

def abc():
    g = {}
    exec('a=1', g)
    return g['a']

res = abc()
assert (res==1), res


# test locals and globals
assert eval('a', {'a': 2}) == 2

globals = {'a': 1}
locals = {'a': 1}

exec('a=2', globals, locals)
assert locals == {'a': 2}
assert globals == {'a': 1}

exec('a=2', globals)
assert globals == {'a': 2}

globals = {'a': 2}
locals = {'b': 3}
assert eval('a*b', globals, locals) == 6

code = compile('a*b', '<string>', 'eval')
assert eval(code, globals, locals) == 6

try:
    exec('a*b*c', globals, locals)
    exit(1)
except NameError:
    pass

# https://github.com/pocketpy/pocketpy/issues/339
res = []
code = '\nres.append(x)\ndef f():\n  res.append(x)\nf()\n'
x = 33
exec(code, {'x': 42, 'res': res})
assert res == [42, 42]
assert x == 33

# test removing trailing newlines
assert eval('[1, 2, 3]\n  \n') == [1, 2, 3]