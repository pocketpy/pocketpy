# test super:
class TestSuperBase():
    def __init__(self):
        self.base_attr = 1
        
    def base_method(self):
        return self.base_attr
    
    def error(self):
        raise RuntimeError('未能拦截错误')
    

class TestSuperChild1(TestSuperBase):
    def __init__(self):
        super(TestSuperChild1, self).__init__()
    
    def child_method(self):
        return super(TestSuperChild1, self).base_method()
    
    def error_handling(self):
        try:
            super(TestSuperChild1, self).error()
        except RuntimeError:
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

class TestSuperNoParent():
    def method(self):
        super(TestSuperNoParent, self).method()

try:
    t = TestSuperNoParent().method()
    print('未能拦截错误2')
    exit(2)
except AttributeError:
    pass

try:
    t = TestSuperNoBaseMethod()
    print('未能拦截错误3')
    exit(3)
except AttributeError:
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
    print('未能拦截错误4')
    exit(4)
except AttributeError:
    pass

try:
    d = D()
    d.method()
    print('未能拦截错误5')
    exit(5)
except TypeError:
    pass

# test hash:
# 测试整数类型的输入
assert type(hash(0)) is int
assert type(hash(123)) is int
assert type(hash(-456)) is int

# 测试字符串类型的输入
assert type(hash("hello")) is int

# 测试浮点数类型的输入
assert type(hash(3.14)) is int 
assert type(hash(-2.71828)) is int

# 测试边界情况
# assert type(hash(None)) is int 
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
    print('未能拦截错误6')
    exit(6)
except TypeError:
    pass

try:
    hash([1])
    print('未能拦截错误7')
    exit(7)
except TypeError:
    pass

# test chr
actual = []
for i in range(128):
    actual.append(f'{i} {chr(i)}')
expected = ['0 \x00', '1 \x01', '2 \x02', '3 \x03', '4 \x04', '5 \x05', '6 \x06', '7 \x07', '8 \x08', '9 \t', '10 \n', '11 \x0b', '12 \x0c', '13 \r', '14 \x0e', '15 \x0f', '16 \x10', '17 \x11', '18 \x12', '19 \x13', '20 \x14', '21 \x15', '22 \x16', '23 \x17', '24 \x18', '25 \x19', '26 \x1a', '27 \x1b', '28 \x1c', '29 \x1d', '30 \x1e', '31 \x1f', '32  ', '33 !', '34 "', '35 #', '36 $', '37 %', '38 &', "39 '", '40 (', '41 )', '42 *', '43 +', '44 ,', '45 -', '46 .', '47 /', '48 0', '49 1', '50 2', '51 3', '52 4', '53 5', '54 6', '55 7', '56 8', '57 9', '58 :', '59 ;', '60 <', '61 =', '62 >', '63 ?', '64 @', '65 A', '66 B', '67 C', '68 D', '69 E', '70 F', '71 G', '72 H', '73 I', '74 J', '75 K', '76 L', '77 M', '78 N', '79 O', '80 P', '81 Q', '82 R', '83 S', '84 T', '85 U', '86 V', '87 W', '88 X', '89 Y', '90 Z', '91 [', '92 \\', '93 ]', '94 ^', '95 _', '96 `', '97 a', '98 b', '99 c', '100 d', '101 e', '102 f', '103 g', '104 h', '105 i', '106 j', '107 k', '108 l', '109 m', '110 n', '111 o', '112 p', '113 q', '114 r', '115 s', '116 t', '117 u', '118 v', '119 w', '120 x', '121 y', '122 z', '123 {', '124 |', '125 }', '126 ~', '127 \x7f']
assert len(actual) == len(expected)
for i in range(len(actual)):
    assert (actual[i] == expected[i]), (actual[i], expected[i])

# assert type(bin(1234)) is str

# test __repr__:
class A():
    def __init__(self):
        self.attr = 0

repr(A())

try:
    range(1,2,3,4)
    print('未能拦截错误8, 在测试 range')
    exit(8)
except TypeError:
    pass

# /************ int ************/
try:
    int('asad')
    print('未能拦截错误9, 在测试 int')
    exit(9)
except ValueError:
    pass

try:
    int(123, 16)
    print('未能拦截错误10, 在测试 int')
    exit(10)
except TypeError:
    pass

assert type(10//11) is int

assert type(11%2) is int

try:
    float('asad')
    print('未能拦截错误11, 在测试 float')
    exit(11)
except ValueError:
    pass

try:
    float([])
    print('未能拦截错误12, 在测试 float')
    exit(12)
except TypeError:
    pass

# /************ str ************/
# test str.__rmul__:
assert type(12 * '12') is str

# 未完全测试准确性-----------------------------------------------
# test str.index:
assert type('25363546'.index('63')) is int
try:
    '25363546'.index('err')
    print('未能拦截错误13, 在测试 str.index')
    exit(13)
except ValueError as e:
    assert str(e) == "substring not found"


# 未完全测试准确性-----------------------------------------------
# test str.find:
assert '25363546'.find('63') == 3
assert '25363546'.find('err') == -1


# /************ list ************/
try:
    list(1,2)
    print('未能拦截错误14, 在测试 list')
    exit(14)
except TypeError:
    pass

# 未完全测试准确性----------------------------------------------
# test list.index:
assert type([1,2,3,4,5].index(4)) is int
try:
    [1,2,3,4,5].index(6)
    print('未能拦截错误15, 在测试 list.index')
    exit(15)
except ValueError as e:
    assert str(e) == "list.index(x): x not in list"



# 未完全测试准确性----------------------------------------------
# test list.remove:
try:
    [1,2,3,4,5].remove(6)
    print('未能拦截错误16, 在测试 list.remove')
    exit(16)
except ValueError as e:
    assert str(e) == "list.remove(x): x not in list"


# 未完全测试准确性----------------------------------------------
# test list.pop:
try:
    [1,2,3,4,5].pop(1,2,3,4)
    print('未能拦截错误17, 在测试 list.pop')
    exit(17)
except TypeError:
    pass



# 未完全测试准确性-----------------------------------------------
# test list.__rmul__:
assert type(12 * [12]) is list


# /************ tuple ************/
# test tuple:
try:
    tuple(1,2)
    print('未能拦截错误18, 在测试 tuple')
    exit(18)
except TypeError:
    pass

assert [1,2,2,3,3,3].count(3) == 3
assert [1,2,2,3,3,3].count(0) == 0
assert 3 in (1, 3, 4)
assert 5 not in (1, 3, 4)

assert repr(True) == 'True'
assert repr(False) == 'False'

assert True & True == True

assert True | False == True

assert (True ^ True) == False

assert (True == True) == True

assert type(hash(bytes([0x41, 0x42, 0x43]))) is int


# 未完全测试准确性-----------------------------------------------
# test bytes.__repr__:
assert type(repr(bytes([0x41, 0x42, 0x43]))) is str


# /************ slice ************/
assert type(slice(0.1, 0.2, 0.3)) is slice
