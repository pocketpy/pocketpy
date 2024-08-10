a = 0

a += 2
assert a == 2

a -= 1
assert a == 1

a *= 2
assert a == 2

a //= 2
assert a == 1

a |= 0xff
assert a == 0xff

a &= 0x0f
assert a == 0x0f

a = 8

a %= 3
assert a == 2

a ^= 0xf0
assert a == 242

# incremental set
class A: pass

for i in range(ord('a'), ord('z')+1):
    setattr(A, chr(i), i)

assert A.a == ord('a')
assert A.z == ord('z')
