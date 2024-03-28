a = '12345'
assert a.encode() == b'12345'

# test add
assert b'123' + b'456' == b'123456'
assert b'' + b'123' == b'123'
assert b'123' + b'' == b'123'
assert b'' + b'' == b''

assert b'\xff\xee' != b'1234'
assert b'\xff\xee' == b'\xff\xee'

a = '测试123'
assert a == a.encode().decode()