import operator as op

assert op.le(1, 2) == True
assert op.lt(1, 2) == True
assert op.ge(1, 2) == False
assert op.gt(1, 2) == False
assert op.eq(1, 2) == False
assert op.ne(1, 2) == True
assert op.and_(0b01, 0b11) == 0b01
assert op.or_(0b01, 0b11) == 0b11
assert op.xor(0b01, 0b11) == 0b10
assert op.invert(0b01) == -0b10
assert op.lshift(0b01, 1) == 0b10
assert op.rshift(0b10, 1) == 0b01
assert op.is_('a', 'a') == True
assert op.is_not('a', 1) == True
assert op.not_(True) == False
assert op.neg(1) == -1
assert op.truth(1) == True
assert op.contains([1, 2], 1) == True
assert op.add(1, 2) == 3
assert op.sub(1, 2) == -1
assert op.mul(1, 2) == 2
assert op.truediv(1, 2) == 0.5
assert op.floordiv(1, 2) == 0
assert op.mod(1, 2) == 1
assert op.pow(2, 3) == 8

class A:
    def __matmul__(self, other):
        return 'matmul'
assert op.matmul(A(), 1) == 'matmul'

a = [1, 2]
assert op.getitem(a, 0) == 1
op.setitem(a, 0, 3)
assert a == [3, 2]
op.delitem(a, 0)
assert a == [2]

a = 'abc'
assert op.iadd(a, 'def') == 'abcdef'
assert op.isub(8, 3) == 5
assert op.imul(a, 2) == 'abcabc'
assert op.itruediv(8, 2) == 4.0
assert op.ifloordiv(8, 3) == 2
assert op.imod(8, 3) == 2
assert op.iand(0b01, 0b11) == 0b01
assert op.ior(0b01, 0b11) == 0b11
assert op.ixor(0b01, 0b11) == 0b10
assert op.ilshift(0b01, 1) == 0b10
assert op.irshift(0b10, 1) == 0b01
