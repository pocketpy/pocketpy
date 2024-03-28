try:
    import os
    import io
    print("[`os` Test Enabled]")
except ImportError:
    exit(0)

a = open('123.txt', 'wt')
a.write('123')
a.write('456')
a.close()

with open('123.txt', 'rt') as f:
    assert f.read() == '123456'

with open('123.txt', 'rt') as f:
    assert f.read(3) == '123'
    assert f.tell() == 3
    assert f.read(3) == '456'
    assert f.tell() == 6
    assert f.read(3) == ''      # EOF
    assert f.tell() == 6

with open('123.txt', 'rb') as f:
    assert f.read(2) == b'12'
    assert f.tell() == 2
    assert f.read(2) == b'34'
    assert f.tell() == 4
    assert f.read(2) == b'56'
    assert f.tell() == 6
    assert f.read(2) == b''     # EOF
    assert f.tell() == 6

# test fseek
with open('123.txt', 'rt') as f:
    f.seek(0, io.SEEK_END)
    assert f.tell() == 6
    assert f.read() == ''
    f.seek(3, io.SEEK_SET)
    assert f.tell() == 3
    assert f.read() == '456'
    assert f.tell() == 6

with open('123.txt', 'a') as f:
    f.write('测试')

# default mode is 'r'
with open('123.txt') as f:
    assert f.read() == '123456' + '测试'

assert os.path.exists('123.txt')
os.remove('123.txt')
assert not os.path.exists('123.txt')


with open('123.bin', 'wb') as f:
    f.write('123'.encode())
    f.write('测试'.encode())

def f_():
    with open('123.bin', 'rb') as f:
        b = f.read()
        assert isinstance(b, bytes)
        assert b == '123测试'.encode()

f_()

assert os.path.exists('123.bin')
os.remove('123.bin')
assert not os.path.exists('123.bin')