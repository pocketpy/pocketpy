try:
    a = [1, 2, 3]
    a.index(999)
    exit(1)
except ValueError:
    pass

# test some python magics
class A:
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

a = A()
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
