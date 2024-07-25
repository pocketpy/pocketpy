try:
    raise 1
except TypeError:
    pass

try:
    assert False
    exit(1)
except AssertionError:
    pass

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

# finally, only
def finally_only():
    try:
        raise KeyError
    finally:
        return True
    
assert finally_only() is True

def finally_only_2():
    try:
        pass
    finally:
        return True
    
assert finally_only_2() is True
    
# finally, no exception
def finally_no_exception():
    ok = False
    try:
        pass
    except KeyError:
        exit(1)
    finally:
        ok = True
    return ok

assert finally_no_exception()

# finally, match
def finally_match():
    ok = False
    try:
        raise KeyError
    except KeyError:
        pass
    finally:
        ok = True
    return ok

assert finally_match()

# finally, no match
ok = False
def finally_no_match():
    global ok
    try:
        raise KeyError
    except IndexError:
        exit(1)
    finally:
        ok = True

try:
    finally_no_match()
except KeyError:
    assert ok
    exit(0)

exit(1)
