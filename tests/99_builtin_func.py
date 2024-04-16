# test super:
class TestSuperBase():
    def __init__(self):
        self.base_attr = 1
        
    def base_method(self):
        return self.base_attr
    
    def error(self):
        raise Exception('未能拦截错误')
    

class TestSuperChild1(TestSuperBase):
    def __init__(self):
        super(TestSuperChild1, self).__init__()
    
    def child_method(self):
        return super(TestSuperChild1, self).base_method()
    
    def error_handling(self):
        try:
            super(TestSuperChild1, self).error()
        except:
            pass

class TestSuperChild2(TestSuperBase):
    pass


test_base = TestSuperBase()
# 测试属性
assert test_base.base_attr == 1
# 测试方法
assert test_base.base_method() == 1

test_child1 = TestSuperChild1()
# 测试继承的属性
assert test_child1.base_attr == 1
# 测试继承的方法
assert test_child1.base_method() == 1
# 测试子类添加的方法
assert test_child1.child_method() == 1
# 测试子类的错误拦截
test_child1.error_handling()

test_child2 = TestSuperChild2()
# 测试继承的属性
assert test_child2.base_attr == 1
# 测试继承的方法
assert test_child2.base_method() == 1


class TestSuperNoBaseMethod(TestSuperBase):
    def __init__(self):
        super(TestSuperNoBaseMethod, self).append(1)

try:
    t = TestSuperNoParent()
    print('未能拦截错误')
    exit(1)
except:
    pass

try:
    t = TestSuperNoBaseMethod()
    print('未能拦截错误')
    exit(1)
except:
    pass

class B():
    pass

class C():
    def method(self):
        super(C, self).method()

class D():
    def method(self):
        super(B, self).method()

try:
    c = C()
    c.method()
    print('未能拦截错误')
    exit(1)
except:
    pass

try:
    d = D()
    d.method()
    print('未能拦截错误')
    exit(1)
except:
    pass

# test hash:
# 测试整数类型的输入
assert hash(0) == 0
assert hash(123) == 123
assert hash(-456) == -456

# 测试字符串类型的输入
assert type(hash("hello")) is int

# 测试浮点数类型的输入
assert type(hash(3.14)) is int 
assert type(hash(-2.71828)) is int

# 测试边界情况
assert type(hash(None)) is int 
assert hash(True) == 1
assert hash(False) == 0

# 测试元组
assert type(hash((4, 5, 6, (1234,1122), 2.3983, 'abcd'))) is int

# 测试自定义类和对象的输入
class A():
    pass

a = A()

assert type(hash(A)) is int
assert type(hash(a)) is int

# 测试函数的输入
def f():
    pass
assert type(hash(a)) is int

# 测试不可哈希对象
try:
    hash({1:1})
    print('未能拦截错误')
    exit(1)
except:
    pass

try:
    hash([1])
    print('未能拦截错误')
    exit(1)
except:
    pass

# test chr
l = []
for i in range(128):
    l.append(f'{i} {chr(i)}')
assert l == ['0 \x00', '1 \x01', '2 \x02', '3 \x03', '4 \x04', '5 \x05', '6 \x06', '7 \x07', '8 \x08', '9 \t', '10 \n', '11 \x0b', '12 \x0c', '13 \r', '14 \x0e', '15 \x0f', '16 \x10', '17 \x11', '18 \x12', '19 \x13', '20 \x14', '21 \x15', '22 \x16', '23 \x17', '24 \x18', '25 \x19', '26 \x1a', '27 \x1b', '28 \x1c', '29 \x1d', '30 \x1e', '31 \x1f', '32  ', '33 !', '34 "', '35 #', '36 $', '37 %', '38 &', "39 '", '40 (', '41 )', '42 *', '43 +', '44 ,', '45 -', '46 .', '47 /', '48 0', '49 1', '50 2', '51 3', '52 4', '53 5', '54 6', '55 7', '56 8', '57 9', '58 :', '59 ;', '60 <', '61 =', '62 >', '63 ?', '64 @', '65 A', '66 B', '67 C', '68 D', '69 E', '70 F', '71 G', '72 H', '73 I', '74 J', '75 K', '76 L', '77 M', '78 N', '79 O', '80 P', '81 Q', '82 R', '83 S', '84 T', '85 U', '86 V', '87 W', '88 X', '89 Y', '90 Z', '91 [', '92 \\', '93 ]', '94 ^', '95 _', '96 `', '97 a', '98 b', '99 c', '100 d', '101 e', '102 f', '103 g', '104 h', '105 i', '106 j', '107 k', '108 l', '109 m', '110 n', '111 o', '112 p', '113 q', '114 r', '115 s', '116 t', '117 u', '118 v', '119 w', '120 x', '121 y', '122 z', '123 {', '124 |', '125 }', '126 ~', '127 \x7f']

assert type(bin(1234)) is str

# test __repr__:
class A():
    def __init__(self):
        self.attr = 0

repr(A())

try:
    range(1,2,3,4)
    print('未能拦截错误, 在测试 range')
    exit(1)
except:
    pass

# /************ int ************/
try:
    int('asad')
    print('未能拦截错误, 在测试 int')
    exit(1)
except:
    pass

try:
    int(123, 16)
    print('未能拦截错误, 在测试 int')
    exit(1)
except:
    pass

assert type(10//11) is int

assert type(11%2) is int

try:
    float('asad')
    print('未能拦截错误, 在测试 float')
    exit(1)
except:
    pass

try:
    float([])
    print('未能拦截错误, 在测试 float')
    exit(1)
except:
    pass

# /************ str ************/
# test str.__rmul__:
assert type(12 * '12') is str

# 未完全测试准确性-----------------------------------------------
# test str.index:
assert type('25363546'.index('63')) is int
try:
    '25363546'.index('err')
    print('未能拦截错误, 在测试 str.index')
    exit(1)
except:
    pass


# 未完全测试准确性-----------------------------------------------
# test str.find:
assert '25363546'.find('63') == 3
assert '25363546'.find('err') == -1


# /************ list ************/
try:
    list(1,2)
    print('未能拦截错误, 在测试 list')
    exit(1)
except:
    pass

# 未完全测试准确性----------------------------------------------
# test list.index:
assert type([1,2,3,4,5].index(4)) is int
try:
    [1,2,3,4,5].index(6)
    print('未能拦截错误, 在测试 list.index')
    exit(1)
except:
    pass



# 未完全测试准确性----------------------------------------------
# test list.remove:
try:
    [1,2,3,4,5].remove(6)
    print('未能拦截错误, 在测试 list.remove')
    exit(1)
except:
    pass


# 未完全测试准确性----------------------------------------------
# test list.pop:
try:
    [1,2,3,4,5].pop(1,2,3,4)
    print('未能拦截错误, 在测试 list.pop')
    exit(1)
except:
    pass



# 未完全测试准确性-----------------------------------------------
# test list.__rmul__:
assert type(12 * [12]) is list


# /************ tuple ************/
# 未完全测试准确性-----------------------------------------------
#       180:  783:    _vm->bind_constructor<-1>("tuple", [](VM* vm, ArgsView args) {
#        32:  784:        if(args.size() == 1+0) return VAR(Tuple(0));
#        32:  785:        if(args.size() == 1+1){
#        32:  786:            List list = CAST(List, vm->py_list(args[1]));
#        32:  787:            return VAR(Tuple(std::move(list)));
#        32:  788:        }
#     #####:  789:        vm->TypeError("tuple() takes at most 1 argument");
#     #####:  790:        return vm->None;
#        32:  791:    });
#         -:  792:
# test tuple:
try:
    tuple(1,2)
    print('未能拦截错误, 在测试 tuple')
    exit(1)
except:
    pass

assert (1,2,3).__contains__(5) == False

assert (1,2,2,3,3,3).count(3) == 3
assert (1,2,2,3,3,3).count(0) == 0

assert repr(True) == 'True'
assert repr(False) == 'False'

assert True & True == 1

assert True | True == 1

assert (True ^ True) == 0

assert (True == True) == 1

assert type(hash(bytes([0x41, 0x42, 0x43]))) is int


# 未完全测试准确性-----------------------------------------------
# test bytes.__repr__:
assert type(repr(bytes([0x41, 0x42, 0x43]))) is str


# /************ slice ************/
# 未完全测试准确性-----------------------------------------------
#       116:  953:    _vm->bind_constructor<4>("slice", [](VM* vm, ArgsView args) {
#     #####:  954:        return VAR(Slice(args[1], args[2], args[3]));
#         -:  955:    });
# test slice:
assert type(slice(0.1, 0.2, 0.3)) is slice


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

# /************ mappingproxy ************/
# test mappingproxy.keys:
class A():
    def __init__(self):
        self.a = 10
    def method(self):
        pass


my_mappingproxy = A().__dict__
assert type(my_mappingproxy.keys()) is list


# 未完全测试准确性-----------------------------------------------
# test mappingproxy.values:
class A():
    def __init__(self):
        self.a = 10
    def method(self):
        pass


my_mappingproxy = A().__dict__
assert type(my_mappingproxy.values()) is list


class A():
    def __init__(self):
        self.a = 10
    def method(self):
        pass


my_mappingproxy = A().__dict__
assert type(len(my_mappingproxy)) is int


class A():
    def __init__(self):
        self.a = 10
    def method(self):
        pass


my_mappingproxy = A().__dict__

try:
    hash(my_mappingproxy)
    print('未能拦截错误, 在测试 mappingproxy.__hash__')
    exit(1)
except TypeError:
    pass

a = hash(object())  # object is hashable
a = hash(A())       # A is hashable
class B:
    def __eq__(self, o): return True

try:
    hash(B())
    print('未能拦截错误, 在测试 B.__hash__')
    exit(1)
except TypeError:
    pass

# 未完全测试准确性-----------------------------------------------
# test mappingproxy.__repr__:
class A():
    def __init__(self):
        self.a = 10
    def method(self):
        pass


my_mappingproxy = A().__dict__
assert type(repr(my_mappingproxy)) is str


# /************ dict ************/
# 未完全测试准确性-----------------------------------------------
# test dict:
assert type(dict([(1,2)])) is dict

try:
    dict([(1, 2, 3)])
    print('未能拦截错误, 在测试 dict')
    exit(1)
except:
    pass

try:
    dict([(1, 2)], 1)
    print('未能拦截错误, 在测试 dict')
    exit(1)
except:
    pass

try:
    hash(dict([(1,2)]))
    print('未能拦截错误, 在测试 dict.__hash__')
    exit(1)
except:
    pass

# test dict.__iter__
for k in {1:2, 2:3, 3:4}:
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
except:
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
assert type(aaa.__doc__) is str


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
    (1, 2),
    (1, 3),
    (1, 4),
]) == (1, 2)

assert min(1, 2) == 1
assert max(1, 2) == 2


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

assert callable(A) is True      # type
assert callable(A()) is True    # instance with __call__
assert callable(A.__call__) is True  # bound method
assert callable(A.__init__) is True  # bound method
assert callable(print) is True  # builtin function
assert callable(isinstance) is True  # builtin function


assert id(0) is None
assert id(2**62) is not None

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

dir_int = dir(int)
assert dir_int[:4] == ['__add__', '__and__', '__base__', '__eq__']