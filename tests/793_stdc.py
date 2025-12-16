from stdc import *

assert sizeof(Int8) == sizeof(UInt8) == 1
assert sizeof(Int16) == sizeof(UInt16) == 2
assert sizeof(Int32) == sizeof(UInt32) == 4
assert sizeof(Int64) == sizeof(UInt64) == 8

assert sizeof(Float) == 4
assert sizeof(Double) == 8

assert sizeof(Bool) == 1
assert sizeof(Pointer) in (4, 8)

x = Int32(42)
assert x.value == 42
x.value = 100
assert x.value == 100

Int32.read(addressof(x), 0) == 100
Int32.write(addressof(x), 0, 200)
assert x.value == 200

# test array
arr = Int32.array(3)
arr[0] = 10
arr[1] = 20
arr[2] = 30
assert arr[0] == 10
assert arr[1] == 20
assert arr[2] == 30

# test malloc, memset, memcpy
p = malloc(3 * sizeof(Int32))
memset(p, 0, 3 * sizeof(Int32))
memcpy(p, addressof(arr), 3 * sizeof(Int32))
for i in range(3):
    assert arr[i] == Int32.read(p, i)

assert memcmp(p, addressof(arr), 3 * sizeof(Int32)) == 0

# test free
free(p)

# test float
y = Double.array(3)
y[0] = 1.1
y[1] = 2.2
y[2] = 3.3
assert Double.read(addressof(y), 0) == 1.1
assert Double.read(addressof(y), 1) == 2.2
assert Double.read(addressof(y), 2) == 3.3

# test read_cstr and write_cstr
a = Char.array(20)
write_cstr(addressof(a), "hello")
assert read_cstr(addressof(a)) == "hello"

a[3] = 0
assert read_cstr(addressof(a)) == "hel"

# test read_bytes and write_bytes
assert read_bytes(addressof(a), 5) == b'hel\x00o'
