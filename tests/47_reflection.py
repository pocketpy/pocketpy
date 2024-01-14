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
assert getattr(a, 'b') == 1

try:
    getattr(a, 'xxx')
    exit(1)
except AttributeError:
    pass

assert getattr(a, 'xxx', 1) == 1
