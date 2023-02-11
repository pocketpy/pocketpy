class A:
    def __getitem__(self, i):
        raise KeyError(i)

try:
    a = A()
    b = a[1]
except:
    print("PASS 01")

try:
    a = {'1': 3, 4: None}
    x = a[1]
except:
    print("PASS 02")
assert True

def f():
    try:
        raise KeyError('foo')
    except A:   # will fail to catch
        assert False
    except:
        print("PASS 03")
    assert True

f()

def f1():
    try:
        assert 1 + 2 == 3
        try:
            a = {1: 2, 3: 4}
            x = a[0]
        except A:
            exit(1)
    except B:
        exit(1)
    exit(1)

try:
    f1()
except KeyError:
    print("PASS 04")


assert True, "Msg"
try:
    assert False, "Msg"
except AssertionError:
    print("PASS 05")
