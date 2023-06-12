from pickle import dumps, loads, _wrap, _unwrap

def test(x, y):
    _0 = _wrap(x)
    _1 = _unwrap(y)
    assert _0 == y, f"{_0} != {y}"
    assert _1 == x, f"{_1} != {x}"
    assert x == loads(dumps(x))

test(1, 1)
test(1.0, 1.0)
test("hello", "hello")
test(True, True)
test(False, False)
test(None, None)

test([1, 2, 3], ["list", [1, 2, 3]])
test((1, 2, 3), ["tuple", [1, 2, 3]])
test({1: 2, 3: 4}, ["dict", [[1, 2], [3, 4]]])

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
    
foo = Foo(1, 2)
test(foo, ["__main__.Foo", None, {"x": 1, "y": 2}])

from linalg import vec2

test(vec2(1, 2), ["linalg.vec2", [1, 2], None])

a = {1, 2, 3, 4}
test(a, ['set', None, {'_a': ['dict', [[1, None], [2, None], [3, None], [4, None]]]}])

a = bytes([1, 2, 3, 4])
assert loads(dumps(a)) == a