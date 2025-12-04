from enum import Enum

class A(Enum):
    a = 1
    b = '2'
    c = None

assert str(A.a) == 'A.a'
assert repr(A.a) == '<A.a: 1>'

assert str(A.b) == 'A.b'
assert repr(A.b) == "<A.b: '2'>"

assert str(A.c) == 'A.c'
assert repr(A.c) == '<A.c: None>'

assert A.a == A.a
assert A.a != A.b
assert A.a != A.c

assert A.a.name == 'a'
assert A.a.value == 1

assert A.b.name == 'b'
assert A.b.value == '2'

assert A.c.name == 'c'
assert A.c.value is None
