import c

array = c.malloc(c.sizeof("int") * 10)

for i in range(10):
    off = c.sizeof("int") * i
    (array+off).write_int(i)

x = c.int_(0)
for i in range(10):
    off = c.sizeof("int") * i
    i = (array+off).read_int()
    x.write_int(x.read_int() + i)

assert x.read_int() == (0+9)*10//2

c.memset(array, 0, c.sizeof("int") * 10)

for i in range(10):
    off = c.sizeof("int") * i
    assert (array+off).read_char() == 0

array2 = c.malloc(c.sizeof("int") * 10)
c.memcpy(array2, array, c.sizeof("int") * 10)
for i in range(10):
    off = c.sizeof("int") * i
    assert (array+off).read_char() == 0

c.free(array)
c.free(array2)

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
