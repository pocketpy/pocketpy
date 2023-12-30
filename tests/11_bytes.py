a = '12345'
assert a.encode() == b'12345'

assert b'\xff\xee' != b'1234'
assert b'\xff\xee' == b'\xff\xee'

a = '测试123'
assert a == a.encode().decode()