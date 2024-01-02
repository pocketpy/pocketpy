try:
    for i in range(5):
        raise KeyError(i)
    exit(1)
except KeyError:
    pass

x = 0
for i in range(5):
    try:
        for j in range(5):
            while True:
                raise KeyError(i)
                x += 3
    except KeyError:
        x += i
assert x == 10

class A:
    def __getitem__(self, i):
        raise KeyError(i)

try:
    a = A()
    b = a[1]
    exit(1)
except:
    pass

try:
    a = {'1': 3, 4: None}
    x = a[1]
    exit(1)
except:
    pass
assert True

def f():
    try:
        raise KeyError('foo')
    except IndexError:   # will fail to catch
        exit(1)
    except:
        pass
    assert True

f()

def f1():
    try:
        assert 1 + 2 == 3
        try:
            a = {1: 2, 3: 4}
            x = a[0]
        except RuntimeError:
            exit(1)
    except IndexError:
        exit(1)
    exit(1)

try:
    f1()
    exit(1)
except KeyError:
    pass


assert True, "Msg"
try:
    assert False, "Msg"
    exit(1)
except AssertionError:
    pass

def f(a: list):
    try:
        raise ValueError
        exit(1)
    except:
        pass
    a[0] = 1
a = [0]
f(a)
assert a == [1]

try:
    a = [][3]
except IndexError as e:
    assert str(e) == '3 not in [0, 0)'
    assert repr(e).startswith('IndexError(')

try:
    a = {}[2]
except IndexError as e:
    exit(1)
except Exception as e:
    assert type(e) is KeyError
    assert str(e) == '2'
    assert repr(e).startswith('KeyError(')
except:
    exit(1)


class MyException(Exception):
    pass

class MyException2(MyException):
    pass

try:
    raise MyException2
except MyException as e:
    ok = True
except Exception:
    exit(1)
assert ok

ok = False
try:
    eval('1+')
except SyntaxError as e:
    assert type(e) is SyntaxError
    ok = True
assert ok
