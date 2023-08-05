import c

c_int = c.refl("int")
assert c_int.name() == "int"
assert c_int.__repr__() == '<ctype \'int\'>'
# ------------------------------------------------
c_int_1 = c.refl("int")
c_struct_1 = c_int_1()
assert (c_int_1() == c_int_1())
assert (c_struct_1 == c_struct_1) == True
# ------------------------------------------------
assert c.void_p.from_hex('0x2568b60').hex() == '0x2568b60'

# ------------------------------------------------
class HexAddress:
    def __init__(self, address):
        if not address.startswith("0x"):  # 确保地址以0x开头
            raise ValueError("Address should start with '0x'.")
        self.address = address[2:]  # 去除0x前缀，并保存十六进制字符串

    def __str__(self):
        return "0x" + self.address  

    def __add__(self, other):
        if isinstance(other, int):
            return HexAddress(hex(int(self.address, 16) + other))  # 将字符串地址转为整数进行运算
        elif isinstance(other, HexAddress):
            return HexAddress(hex(int(self.address, 16) + int(other.address, 16)))  # 将字符串地址转为整数进行运算
        else:
            raise TypeError("Unsupported operand type for +: HexAddress and {}".format(type(other)))

    def __sub__(self, other):
        if isinstance(other, int):
            return HexAddress(hex(int(self.address, 16) - other))  # 将字符串地址转为整数进行运算
        elif isinstance(other, HexAddress):
            return HexAddress(hex(int(self.address, 16) - int(other.address, 16)))  # 将字符串地址转为整数进行运算
        else:
            raise TypeError("Unsupported operand type for -: HexAddress and {}".format(type(other)))

c_void_1 = c.malloc(8)

assert (c_void_1 + 8).hex() == c.void_p.from_hex(str(HexAddress(c_void_1.hex()) + 8)).hex()
assert (c_void_1 - 8).hex() == c.void_p.from_hex(str(HexAddress(c_void_1.hex()) - 8)).hex()

# ------------------------------------------------
# 此处测试并不完全
c_void_1 = c.malloc(8)
c_void_1.read_bytes(5)
c_void_1.write_bytes(c_void_1.read_bytes(5))
# ------------------------------------------------
c_void_1 = c.malloc(32)
my_struct2 = c.struct(c_void_1, 32)

data_str = "Hello, World!"
my_struct3 = c.struct(data_str)

data_bytes = bytes([1,2,3])
my_struct4 = c.struct(data_bytes)

try:
   c.struct(True)
   raise Exception('c.struct 的构造方法未能触发 TypeError("expected int, str or bytes")')
except TypeError:
   pass

try:
   c.struct(1,2,3)
   raise Exception('c.struct 的构造方法未能触发 TypeError("expected 1 or 2 arguments")')
except TypeError:
   pass

try:
   x = c.refl("int")['a']
except KeyError:
   pass
# ------------------------------------------------
my_struct1 = c.struct(16)
assert my_struct1.size() == 16

# 对 c.struct 的 copy 方法的测试不完全
assert my_struct1.copy().size() == 16

data_str = "Hello, World!"
my_struct3 = c.struct(data_str)
assert my_struct3.to_string() == data_str

data_bytes = bytes([1,2,3])
my_struct4 = c.struct(data_bytes)
assert my_struct4.to_bytes() == data_bytes


# ------------------------------------------------
# 此处测试并不完全
c_void_1 = c.malloc(16)
my_struct1 = c.struct(16)
c_void_1.read_struct('long')
c_void_1.write_struct(my_struct1)
