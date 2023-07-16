a = '12345'
assert a.encode() == b'12345'

assert b'\xff\xee' != b'1234'
assert b'\xff\xee' == b'\xff\xee'
