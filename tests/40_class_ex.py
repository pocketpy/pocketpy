class A:
    def __init__(self, a, b):
        self.a = a
        self.b = b

    def add(self):
        return self.a + self.b

    def sub(self):
        return self.a - self.b
    
a = A(1, 2)
assert a.add() == 3
assert a.sub() == -1

assert A.__base__ is object

class B(A):
    def __init__(self, a, b, c):
        super().__init__(a, b)
        self.c = c

    def add(self):
        return self.a + self.b + self.c

    def sub(self):
        return self.a - self.b - self.c

assert B.__base__ is A    

b = B(1, 2, 3)
assert b.add() == 6
assert b.sub() == -4

class C(B):
    def __init__(self, a, b, c, d):
        super().__init__(a, b, c)
        self.d = d

    def add(self):
        return self.a + self.b + self.c + self.d

    def sub(self):
        return self.a - self.b - self.c - self.d
    
assert C.__base__ is B

c = C(1, 2, 3, 4)
assert c.add() == 10
assert c.sub() == -8

class D(C):
    def __init__(self, a, b, c, d, e):
        super().__init__(a, b, c, d)
        self.e = e

    def add(self):
        return super().add() + self.e

    def sub(self):
        return super().sub() - self.e
    
assert D.__base__ is C

d = D(1, 2, 3, 4, 5)
assert d.add() == 15
assert d.sub() == -13

assert isinstance(1, int)
assert isinstance(1, object)
assert isinstance(C, type)
assert isinstance(C, object)
assert isinstance(d, object)
assert isinstance(d, C)
assert isinstance(d, B)
assert isinstance(d, A)
assert isinstance(object, object)
assert isinstance(type, object)

assert isinstance(1, (float, int))
assert isinstance(1, (float, object))
assert not isinstance(1, (float, str))
assert isinstance(object, (int, type, float))
assert not isinstance(object, (int, float, str))

try:
    isinstance(1, (1, 2))
    exit(1)
except TypeError:
    pass

try:
    isinstance(1, 1)
    exit(1)
except TypeError:
    pass

class A:
    a = 1
    b = 2

assert A.a == 1
assert A.b == 2

class B(A):
    b = 3
    c = 4

# assert B.a == 1  ...bug here
assert B.b == 3
assert B.c == 4

import c

class A(c.void_p):
    pass
    
a = A(0)
assert repr(a).startswith('<void* at')

if 1:
    class TrueClass:
        pass
else:
    class FalseClass:
        pass

assert TrueClass

# staticmethod and classmethod
class A():
    dd = 2

    def __init__(self):
        self.a = 1
        
    @staticmethod
    def static_method(txt):
        return txt
    
    @classmethod
    def class_method(cls, txt):
        return cls.__name__ + txt

assert A.static_method(123) == 123
assert A.class_method('123') == 'A123'
assert A().static_method(123) == 123
assert A().class_method('123') == 'A123'
assert A.dd == 2
assert A().dd == 2

class B(A): pass

assert B.dd == 2
assert B().dd == 2

assert B.static_method(123) == 123
assert B.class_method('123') == 'B123'
assert B().static_method(123) == 123
assert B().class_method('123') == 'B123'
