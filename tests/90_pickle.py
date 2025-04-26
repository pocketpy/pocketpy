import pickle as pkl

def test(data): # type: ignore
    print('-'*50)
    b = pkl.dumps(data)
    print(b)
    o = pkl.loads(b)
    print(o)
    assert data == o
    return o

test(None)                      # PKL_NONE
test(...)                       # PKL_ELLIPSIS
test(1)                         # PKL_INT8
test(277)                       # PKL_INT16
test(-66666)                    # PKL_INT32
test(0xffffffffffff)            # PKL_INT64
test(1.0)                       # PKL_FLOAT32
test(1.12312434234)             # PKL_FLOAT64
test(True)                      # PKL_TRUE
test(False)                     # PKL_FALSE
test("hello")                   # PKL_STRING
test(b"hello")                  # PKL_BYTES

from vmath import vec2, vec3, vec2i, vec3i

test(vec2(2/3, 1.0))            # PKL_VEC2
test(vec3(2/3, 1.0, 3.0))       # PKL_VEC3
test(vec2i(1, 2))               # PKL_VEC2I
test(vec3i(1, 2, 3))            # PKL_VEC3I

test(vec3i)                     # PKL_TYPE

print('-'*50)
from array2d import array2d
a = array2d[int | bool | vec2i].fromlist([
    [1, 2, vec2i.LEFT],
    [4, True, 6]
])
a_encoded = pkl.dumps(a)
print(a_encoded)
a_decoded = pkl.loads(a_encoded)
assert isinstance(a_decoded, array2d)
assert a_decoded.width == 3 and a_decoded.height == 2
assert (a == a_decoded).all()
print(a_decoded)

test([1, 2, 3])                 # PKL_LIST
test((1, 2, 3))                 # PKL_TUPLE
test({1: 2, 3: 4})              # PKL_DICT

# test complex data
test([1, '2', True])
test([1, '2', 3.0, True])
test([1, '2', True, {'key': 4}])
test([1, '2', 3.0, True, {'k1': 4, 'k2': [b'xxxx']}])

# test memo
a = [1, 2, 3, 4, 5, 6, 745]
b = [a] * 10
c = test(b)
assert b == c
assert b is not c
assert c[0] is c[1] and c[1] is c[2]

s1 = 'hello'
s2 = 'world'
a = [s1, s2] * 10
b = test(a)
assert b == a
assert b is not a
assert b[0] is b[2]
assert b[1] is b[3]

from pkpy import TValue

class Base(TValue[int]):
    def __eq__(self, other):
        return self.value == other.value
    
    def __ne__(self, other):
        return self.value != other.value
    
class TVal(Base): pass # type: ignore

test(TVal(1))

old_bytes = pkl.dumps(TVal(1))
print(old_bytes)

# re-define the class so it will have a new type id
class TVal(Base): pass
# see if we can still load the old data
decoded = pkl.loads(old_bytes)
assert decoded == TVal(1)
print(pkl.dumps(decoded))

# test array2d with TValue
a = array2d[TVal].fromlist([
    [TVal(1), TVal(2)],
    [TVal(3), 1]])
test(a)

# test __reduce__

class A:
    def __init__(self, seed):
        self.seed = seed
        self.x = seed
        self.y = seed + 1
        self.z = seed + 2
    def __eq__(self, other):
        return (self.x, self.y, self.z) == (other.x, other.y, other.z)
    def __ne__(self, other):
        return (self.x, self.y, self.z) != (other.x, other.y, other.z)
    def __repr__(self):
        return f"A({self.seed}, x={self.x}, y={self.y}, z={self.z})"
    def __reduce__(self):
        print('__reduce__() called')
        return A, (self.seed,)

test([A(1)]*10)

class Simple:
    def __init__(self, x):
        self.field1 = x
        self.field2 = [...]
    def __eq__(self, other): return self.field1 == other.field1
    def __ne__(self, other): return self.field1 != other.field1

test(Simple(1))
test([Simple(2)]*10)

from dataclasses import dataclass

@dataclass
class Data:
    a: int
    b: str = '2'
    c: float = 3.0

test(Data(1))

exit()

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

from vmath import vec2

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