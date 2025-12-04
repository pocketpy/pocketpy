assert type(1) is int
assert type(1.0) is float
assert type(object) is type
assert type(type) is type

assert hasattr(object, '__base__')
assert hasattr(1, '__add__')
assert hasattr(int, '__add__')

assert type(1).__add__(1, 2) == 3
assert getattr(1, '__add__')(2) == 3

a = object()
setattr(a, 'b', 1)
assert a.b == 1
assert hasattr(a, 'b')
assert getattr(a, 'b') == 1
assert getattr(a, 'c', ...) == ...
delattr(a, 'b')
assert not hasattr(a, 'b')

try:
    getattr(a, 'xxx')
    exit(1)
except AttributeError:
    pass

assert getattr(a, 'xxx', 1) == 1

class A:
    def __init__(self, x):
        self.x = x

    def __getattr__(self, name):
        if not name:
            raise AttributeError
        return name, None
    
a = A(1)
assert a.x == 1
assert a.y == ('y', None)
assert a.zzz == ('zzz', None)

assert getattr(a, 'x') == 1
assert getattr(a, 'zzz') == ('zzz', None)

assert hasattr(a, 'x')
assert hasattr(a, 'y')
assert hasattr(a, 'zzz')

assert not hasattr(a, '')

