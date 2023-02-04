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