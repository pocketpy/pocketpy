# https://docs.python.org/3/library/operator.html#mapping-operators-to-functions

def le(a, b): return a <= b
def lt(a, b): return a < b
def ge(a, b): return a >= b
def gt(a, b): return a > b
def eq(a, b): return a == b
def ne(a, b): return a != b

def and_(a, b): return a & b
def or_(a, b): return a | b
def xor(a, b): return a ^ b
def invert(a): return ~a
def lshift(a, b): return a << b
def rshift(a, b): return a >> b

def is_(a, b): return a is b
def is_not(a, b): return a is not b
def not_(a): return not a
def truth(a): return bool(a)
def contains(a, b): return b in a

def add(a, b): return a + b
def sub(a, b): return a - b
def mul(a, b): return a * b
def truediv(a, b): return a / b
def floordiv(a, b): return a // b
def mod(a, b): return a % b
def pow(a, b): return a ** b
def neg(a): return -a
def matmul(a, b): return a @ b

def getitem(a, b): return a[b]
def setitem(a, b, c): a[b] = c
def delitem(a, b): del a[b]

def iadd(a, b): a += b; return a
def isub(a, b): a -= b; return a
def imul(a, b): a *= b; return a
def itruediv(a, b): a /= b; return a
def ifloordiv(a, b): a //= b; return a
def imod(a, b): a %= b; return a
# def ipow(a, b): a **= b; return a
# def imatmul(a, b): a @= b; return a
def iand(a, b): a &= b; return a
def ior(a, b): a |= b; return a
def ixor(a, b): a ^= b; return a
def ilshift(a, b): a <<= b; return a
def irshift(a, b): a >>= b; return a
