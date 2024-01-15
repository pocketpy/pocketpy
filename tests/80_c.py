import c

a = c.malloc(100)
c.free(a)

a = c.malloc(100)
c.memset(a, 0, 100)
b = c.malloc(100)
b = c.memcpy(b, a, 100)

bp = c.p_cast(a, c.int_p)

assert c.p_value(c.NULL) == 0
assert c.NULL == c.NULL
assert c.NULL != a

for i in range(10):
    bp[i] = i
    assert bp[i] == i
    (bp+i).write(i)
    assert (bp+i).read() == i

i = c.float_(10)
assert i.sizeof() == 4
j = i.copy()
assert i == j
assert i is not j

####################

class Vec2(c.struct):
    def __new__(cls, x: float, y: float):
        obj = c.struct.__new__(cls, 8)
        obj.write_float(x, 0)
        obj.write_float(y, 4)
        return obj

    @property
    def x(self) -> float:
        return self.read_float(0)
    
    @property
    def y(self) -> float:
        return self.read_float(4)
    
    def __repr__(self) -> str:
        return f"Vec2({self.x}, {self.y})"
    
a = Vec2(1, 2)
assert isinstance(a, c.struct)
assert type(a) is Vec2
assert repr(a) == "Vec2(1.0, 2.0)"

a = c.struct(10)
p = c.p_cast(a.addr(), c.char_p)
p.write_string("Hello!")
assert p[0] == ord("H")
assert p[1] == ord("e")
assert p[2] == ord("l")
assert p[3] == ord("l")
assert p[4] == ord("o")
assert p[5] == ord("!")
assert p[6] == 0
assert p.read_string() == "Hello!"

s = c.struct(67)
for i in range(67):
    s.write_char(i, i)

s_hex = s.hex()
s_r = c.struct.fromhex(s_hex)
assert (s == s_r and s is not s_r), (s_hex, s_r.hex())
assert s_hex == s_r.hex()

