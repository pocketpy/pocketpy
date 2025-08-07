try:
    a = [1, 2, 3]
    a.index(999)
    exit(1)
except ValueError:
    pass

# test some python magics
class TestMagics:
    def __init__(self):
        self.d = {}

    def __getitem__(self, index):
        return self.d[index]
    
    def __setitem__(self, index, value):
        self.d[index] = value

    def __contains__(self, index):
        return index in self.d
    
    def __delitem__(self, index):
        del self.d[index]

a = TestMagics()
a['1'] = 3
assert '1' in a
assert '2' not in a
assert a['1'] == 3
del a['1']
assert '1' not in a

# slice extras
class A:
    def __getitem__(self, index):
        return index

assert slice(1, 2, None) == slice(1, 2, None)
assert slice(1, 3, None) != slice(1, 2, None)

assert A()[1] == 1
assert A()[1:2, 3] == (slice(1, 2, None), 3)
assert A()[1:2, 3:4] == (slice(1, 2, None), slice(3, 4, None))
assert A()[1:2, 3:4, 5] == (slice(1, 2, None), slice(3, 4, None), 5)
assert A()[:, :] == (slice(None, None, None), slice(None, None, None))
assert A()[::, :] == (slice(None, None, None), slice(None, None, None))
assert A()[::, :2] == (slice(None, None, None), slice(None, 2, None))
assert A()['b':'c':1, :] == (slice('b', 'c', 1), slice(None, None, None))
assert A()[1:2, :A()[3:4, ::-1]] == (slice(1, 2, None), slice(None, (slice(3, 4, None), slice(None, None, -1)), None))

# test right associative
assert 2**2**3 == 256
assert (2**2**3)**2 == 65536

class Number:        
    def __divmod__(self, other):
        return 3, 4
    
    def __round__(self, *args):
        return args

assert divmod(Number(), 0) == (3, 4)
assert round(Number()) == tuple()
assert round(Number(), 1) == (1,)

class Z:
    def __new__(cls, x):
        return cls, x

class B(Z):
    def __new__(cls, x):
        assert super() is Z
        return super().__new__(cls, x)

assert Z(1) == (Z, 1)
assert B(1) == (B, 1)

from pkpy import TValue

class fixed(TValue[int]):
    def __new__(cls, value: str):
        assert super() is TValue[int]
        return super().__new__(cls, int(value))
    
assert fixed('123').value == 123

# context bug
class Context:
  def __enter__(self):
    return 1
  def __exit__(self, *_):
    pass

for _ in range(5):
    with Context() as x:
        assert x == 1

# bad dict hash
class A:
    def __eq__(self, o): return False
    def __ne__(self, o): return True
    def __hash__(self): return 1

bad_dict = {A(): 1, A(): 2, A(): 3, A(): 4}
assert len(bad_dict) == 4

