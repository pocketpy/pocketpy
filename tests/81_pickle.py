from pickle import dumps, loads, _wrap, _unwrap

def test(x):
    y = dumps(x)
    # print(y.decode())
    ok = x == loads(y)
    if not ok:
        _0 = _wrap(x)
        _1 = _unwrap(_0)
        print('='*50)
        print(_0)
        print('-'*50)
        print(_1)
        print('='*50)
        assert False

test(1)
test(1.0)
test("hello")
test(True)
test(False)
test(None)

test([1, 2, 3])
test((1, 2, 3))
test({1: 2, 3: 4})

class Foo:
    def __init__(self, x, y):
        self.x = x
        self.y = y

    def __eq__(self, __value: object) -> bool:
        if not isinstance(__value, Foo):
            return False
        return self.x == __value.x and self.y == __value.y
    
    def __repr__(self) -> str:
        return f"Foo({self.x}, {self.y})"
    
test(Foo(1, 2))
test(Foo([1, True], 'c'))

from linalg import vec2

test(vec2(1, 2))

a = {1, 2, 3, 4}
test(a)

a = bytes([1, 2, 3, 4])
test(a)

a = [1, 2]
d = {'k': a, 'j': a}
c = loads(dumps(d))

assert c['k'] is c['j']
assert c == d

# test circular references
from collections import deque

a = deque([1, 2, 3])
test(a)

a = [int, float, Foo]
test(a)