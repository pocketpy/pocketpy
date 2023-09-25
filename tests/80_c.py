import c

c_int = c.refl("int")
assert c_int.size() == c.sizeof("int")
array = c.malloc(c.sizeof("int") * 10)
array.set_base_offset("int")

assert array.get_base_offset() == c_int.size()

for i in range(10):
    array.offset(i).write_int(i)

x = c_int()
x.addr().write_int(0)
for i in range(10):
    i = array.offset(i).read_int()
    x.addr().write_int(
        x.addr().read_int() + i
    )

assert x.addr().read_int() == (0+9)*10//2

c.memset(array, 0, c.sizeof("int") * 10)

for i in range(10):
    assert array.offset(i).read_char() == 0

array2 = c.malloc(c.sizeof("int") * 10)
array2.set_base_offset("int")
c.memcpy(array2, array, c.sizeof("int") * 10)
for i in range(10):
    assert array2.offset(i).read_char() == 0

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
