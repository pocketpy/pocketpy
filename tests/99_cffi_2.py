import c

assert c.NULL == c.void_p(0)
# ------------------------------------------------
# 此处测试并不完全
c_void_1 = c.malloc(8)
c_void_1.read_bytes(5)
c_void_1.write_bytes(c_void_1.read_bytes(5))
# ------------------------------------------------
c_void_1 = c.malloc(32)
my_struct2 = c_void_1.read_struct(32)
assert my_struct2.size() == 32

data_bytes = bytes([1,2,3])
my_struct4 = c.struct(data_bytes)

try:
   c.struct(True)
   raise Exception('c.struct 的构造方法未能触发 TypeError("expected int or bytes")')
except TypeError:
   pass

try:
   c.struct(1,2,3)
   raise Exception('c.struct 的构造方法未能触发 TypeError("expected 1 or 2 arguments")')
except TypeError:
   pass
# ------------------------------------------------
my_struct1 = c.struct(16)
assert my_struct1.size() == 16

# 对 c.struct 的 copy 方法的测试不完全
assert my_struct1.copy().size() == 16

data_bytes = bytes([1,2,3])
my_struct4 = c.struct(data_bytes)
assert my_struct4.addr().read_bytes(
    my_struct4.size()
) == data_bytes


# ------------------------------------------------
# 此处测试并不完全
c_void_1 = c.malloc(16)
my_struct1 = c.struct(16)
c_void_1.write_struct(my_struct1)
assert c_void_1.read_struct(16) == my_struct1

from c import array, int_
a = array(10, item_size=4)
assert a.item_count == 10
assert a.item_size == 4

_ = hash(a)
a[4] = int_(123)
assert a[4] == int_(123)