# 未完全测试准确性-----------------------------------------------
#       116: 1529:    bind_property(_t(tp_slice), "start", [](VM* vm, ArgsView args){
#     #####: 1530:        return CAST(Slice&, args[0]).start;
#         -: 1531:    });
#       116: 1532:    bind_property(_t(tp_slice), "stop", [](VM* vm, ArgsView args){
#     #####: 1533:        return CAST(Slice&, args[0]).stop;
#         -: 1534:    });
#       116: 1535:    bind_property(_t(tp_slice), "step", [](VM* vm, ArgsView args){
#     #####: 1536:        return CAST(Slice&, args[0]).step;
#         -: 1537:    });
s = slice(1, 2, 3)
assert type(s) is slice
assert s.start == 1
assert s.stop == 2
assert s.step == 3

# 未完全测试准确性-----------------------------------------------
# test slice.__repr__
assert type(repr(slice(1,1,1))) is str


class A():
    def __init__(self):
        self.a = 10
    def method(self):
        pass


my_namedict = A().__dict__

try:
    hash(my_namedict)
    print('未能拦截错误, 在测试 namedict.__hash__')
    exit(1)
except TypeError:
    pass

a = hash(object())  # object is hashable
a = hash(A())       # A is hashable
class B:
    def __eq__(self, o): return True
    def __ne__(self, o): return False

try:
    hash(B())
    print('未能拦截错误, 在测试 B.__hash__')
    exit(1)
except TypeError:
    pass

# 未完全测试准确性-----------------------------------------------
# test namedict.__repr__:
class A():
    def __init__(self):
        self.a = 10
    def method(self):
        pass


my_namedict = A().__dict__
assert type(repr(my_namedict)) is str


# /************ dict ************/
# 未完全测试准确性-----------------------------------------------
# test dict:
assert type(dict([(1,2)])) is dict

try:
    dict([(1, 2, 3)])
    print('未能拦截错误, 在测试 dict')
    exit(1)
except ValueError:
    pass

try:
    dict([(1, 2)], 1)
    print('未能拦截错误, 在测试 dict')
    exit(1)
except TypeError:
    pass

try:
    hash(dict([(1,2)]))
    print('未能拦截错误, 在测试 dict.__hash__')
    exit(1)
except TypeError:
    pass

# test dict.__iter__
for k in {1:2, 2:3, 3:4}.keys():
    assert k in [1,2,3]

# 未完全测试准确性-----------------------------------------------
# test dict.get

assert {1:2, 3:4}.get(1) == 2
assert {1:2, 3:4}.get(2) is None
assert {1:2, 3:4}.get(20, 100) == 100

try:
    {1:2, 3:4}.get(1,1, 1)
    print('未能拦截错误, 在测试 dict.get')
    exit(1)
except TypeError:
    pass

# 未完全测试准确性-----------------------------------------------
# test dict.__repr__
assert type(repr({1:2, 3:4})) is str

# /************ property ************/
class A():
    def __init__(self):
        self._name = '123'

    @property
    def value(self):
        return 2

    def get_name(self):
        '''
        doc string 1
        '''
        return self._name

    def set_name(self, val):
        '''
        doc string 2
        '''
        self._name = val

assert A().value == 2

A.name = property(A.get_name, A.set_name)

class Vector2:
    def __init__(self) -> None:
        self._x = 0

    @property
    def x(self):
        return self._x
    
    @x.setter
    def x(self, val):
        self._x = val

v = Vector2()
assert v.x == 0
v.x = 10
assert v.x == 10

# function.__doc__
def aaa():
    '12345'
    pass
assert aaa.__doc__ == '12345'

# test callable
assert callable(lambda: 1) is True          # function
assert callable(1) is False                 # int
assert callable(object) is True             # type
assert callable(object()) is False
assert callable([].append) is True      # bound method
assert callable([].__getitem__) is True # bound method

class A:
    def __init__(self):
        pass

    def __call__(self):
        pass

    @staticmethod
    def staticmethod():
        pass

    @classmethod
    def classmethod(cls):
        pass

assert callable(A) is True      # type
assert callable(A()) is True    # instance with __call__
assert callable(A.__call__) is True  # bound method
assert callable(A.__init__) is True  # bound method
assert callable(print) is True  # builtin function
assert callable(isinstance) is True  # builtin function
assert callable(A.staticmethod) is True  # staticmethod
assert callable(A.classmethod) is True  # classmethod

assert id(0) is None
assert id(2**62) is None

# test issubclass
assert issubclass(int, int) is True
assert issubclass(int, object) is True
assert issubclass(object, int) is False
assert issubclass(object, object) is True
assert issubclass(int, type) is False
assert issubclass(type, type) is True
assert issubclass(float, int) is False


def f(a, b):
    c = a
    del a
    return sum([b, c])

assert f(1, 2) == 3

# /************ module time ************/
import time
# test time.time
assert type(time.time()) is float

local_t = time.localtime()
assert type(local_t.tm_year) is int
assert type(local_t.tm_mon) is int
assert type(local_t.tm_mday) is int
assert type(local_t.tm_hour) is int
assert type(local_t.tm_min) is int
assert type(local_t.tm_sec) is int
assert type(local_t.tm_wday) is int
assert type(local_t.tm_yday) is int
assert type(local_t.tm_isdst) is int

# test time.sleep
time.sleep(0.1)
# test time.localtime
assert type(time.localtime()) is time.struct_time

# test min/max
assert min(1, 2) == 1
assert min(1, 2, 3) == 1
assert min([1, 2]) == 1
assert min([1, 2], key=lambda x: -x) == 2

assert max(1, 2) == 2
assert max(1, 2, 3) == 3
assert max([1, 2]) == 2
assert max([1, 2, 3], key=lambda x: -x) == 1

assert min([
    (3, 1),
    (1, 2),
    (1, 3),
    (1, 4),
]) == (1, 2)

assert min(1, 2) == 1
assert max(1, 2) == 2

def fn(): pass
assert repr(fn).startswith('<function fn at')

assert repr(round) == '<nativefunc object>'