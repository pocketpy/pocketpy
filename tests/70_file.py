try:
    import os
    import io
except ImportError:
    exit(0)

a = open('123.txt', 'wt')
a.write('123')
a.write('456')
a.close()

with open('123.txt', 'rt') as f:
    assert f.read() == '123456'

with open('123.txt', 'a') as f:
    f.write('测试')

with open('123.txt', 'r') as f:
    assert f.read() == '123456' + '测试'

assert os.path.exists('123.txt')
os.remove('123.txt')
assert not os.path.exists('123.txt')


with open('123.bin', 'wb') as f:
    f.write('123'.encode())
    f.write('测试'.encode())

with open('123.bin', 'rb') as f:
    b = f.read()
    assert isinstance(b, bytes)
    assert b == '123测试'.encode()

assert os.path.exists('123.bin')
os.remove('123.bin')
assert not os.path.exists('123.bin')