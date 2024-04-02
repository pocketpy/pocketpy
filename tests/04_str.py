assert 'testing' == 'test' + 'ing'
assert 'testing' != 'test' + 'ing2'
assert 'testing' < 'test' + 'ing2'
assert 'testing' <= 'test' + 'ing2'
assert 'testing5' > 'test' + 'ing1'
assert 'testing5' >= 'test' + 'ing1'

# test + *=
assert 'abc' + 'def' == 'abcdef'
assert 'abc' * 3 == 'abcabcabc'

assert repr('\\\n\t\'\r\b\x48') == r"'\\\n\t\'\r\bH'"

a = ''
b = 'test'
c ='test'
assert a == str()
assert len(a) == 0
assert len(b) == 4
assert b == c

# upper and lower does not work for utf-8
assert ''.lower() == '' and ''.upper() == ''
assert 'already+lower '.lower() == 'already+lower '
assert 'ALREADY+UPPER '.upper() == 'ALREADY+UPPER '
assert 'tEST+InG'.lower() == 'test+ing'
assert 'tEST+InG'.upper() == 'TEST+ING'

s = "football"
q = "abcd"
r = "zoo"
t = "this is string example....wow!!!"
assert s[0] == 'f'
assert s[1:4] == 'oot'
assert s[:-1] == 'footbal'
assert s[:10] == 'football'
assert s[-3] == 'a'
assert t[-5:] == 'ow!!!'
assert t[3:-3] == 's is string example....wow'
assert s > q;assert s < r
assert s.replace("o","") == "ftball"
assert s.replace("o","O",1) == "fOotball"
assert s.replace("foo","ball",1) == "balltball"
assert s.startswith('f') == True;assert s.endswith('o') == False
assert t.startswith('this') == True;


assert t.split('w') == ['this is string example....', 'o', '!!!']
assert "a,b,c".split(',') == ['a', 'b', 'c']
assert 'a,'.split(',') == ['a']
assert 'foo!!bar!!baz'.split('!!') == ['foo', 'bar', 'baz']
assert ' 4 3 '.split() == ['4', '3']
assert '  4 3  '.split(' ') == ['4', '3']

x = 'aa bb cccc'
assert x.split('cc') == ['aa bb ']

assert '111'.count('1') == 3
assert '111'.count('11') == 1
assert '1111'.count('11') == 2
assert '11'.count('') == 3
assert ''.count('1') == 0
assert ''.count('') == 1

t = "*****this is **string** example....wow!!!*****"
s = "123abcrunoob321"
assert t.strip( '*' ) == "this is **string** example....wow!!!"
assert s.strip( '12' ) == "3abcrunoob3"

assert t.strip( '*' ) == "this is **string** example....wow!!!"
assert s.strip( '12' ) == "3abcrunoob3"

assert '测试123'.strip('测试') == '123'
assert '测试123测试'.strip('测试') == '123'
assert '123测试'.strip('2') == '123测试'
assert '测试123'.strip('测') == '试123'
assert '测试123'.strip('试') == '测试123'

assert '测试123测试'.lstrip('测试') == '123测试'
assert '测试123测试'.rstrip('测试') == '测试123'

assert 'abc'.lstrip('a') == 'bc'
assert 'abc'.lstrip('b') == 'abc'
assert 'abc'.lstrip('c') == 'abc'
assert 'abc'.rstrip('a') == 'abc'
assert 'abc'.rstrip('b') == 'abc'
assert 'abc'.rstrip('c') == 'ab'

assert 'abc'.lstrip('abc') == ''
assert 'abc'.rstrip('abc') == ''
assert 'abc'.strip('abc') == ''

s = ' asd\n  asd \n'
assert s.strip() == 'asd\n  asd'

s1 = "-"
s2 = ""
seq = ["r","u","n","o","o","b"]
assert s1.join( seq ) == "r-u-n-o-o-b"
assert s2.join( seq ) == "runoob"

assert 'x'.zfill(5) == '0000x'
assert '568'.zfill(1) == '568'

def test(*seq):
    return s1.join(seq)
assert test("r", "u", "n", "o", "o", "b") == "r-u-n-o-o-b"

def f():
    for i in range(5):
        yield str(i)
assert '|'.join(f()) == '0|1|2|3|4'

num = 6
assert str(num) == '6'

# test Lo group names

测试 = "test"
assert 测试 == "test"

assert "Hello, {}!".format("World") == "Hello, World!"
assert "{} {} {}".format("I", "love", "Python") == "I love Python"
assert "{0} {1} {2}".format("I", "love", "Python") == "I love Python"
assert "{2} {1} {0}".format("I", "love", "Python") == "Python love I"
assert "{0}{1}{0}".format("abra", "cad") == "abracadabra"

assert "{k}={v}".format(k="key", v="value") == "key=value"
assert "{k}={k}".format(k="key") == "key=key"
assert "{0}={1}".format('{0}', '{1}') == "{0}={1}"
assert "{{{0}}}".format(1) == "{1}"
assert "{0}{1}{1}".format(1, 2, 3) == "122"
try:
    "{0}={1}}".format(1, 2)
    exit(1)
except ValueError:
    pass
assert "{{{}xxx{}x}}".format(1, 2) == "{1xxx2x}"
assert "{{abc}}".format() == "{abc}"

# 3rd slice
a = "Hello, World!"
assert a[::-1] == "!dlroW ,olleH"
assert a[::2] == "Hlo ol!"
assert a[2:5:2] == "lo"
assert a[5:2:-1] == ",ol"
assert a[5:2:-2] == ",l"

b = list(a)
assert b == ['H', 'e', 'l', 'l', 'o', ',', ' ', 'W', 'o', 'r', 'l', 'd', '!']
assert b[::-1] == ['!', 'd', 'l', 'r', 'o', 'W', ' ', ',', 'o', 'l', 'l', 'e', 'H']
assert b[::2] == ['H', 'l', 'o', ' ', 'o', 'l', '!']
assert b[2:5:2] == ['l', 'o']
assert b[5:2:-1] == [',', 'o', 'l']
assert b[5:2:-2] == [',', 'l']

a = '123'
assert a.rjust(5) == '  123'
assert a.rjust(5, '0') == '00123'
try:
    a.rjust(5, '00')
    exit(1)
except TypeError:
    pass
assert a.ljust(5) == '123  '
assert a.ljust(5, '0') == '12300'
try:
    a.ljust(5, '00')
    exit(1)
except TypeError:
    pass

assert '\x30\x31\x32' == '012'

a = 'abcd'
assert list(a) == ['a', 'b', 'c', 'd']
a = '测试'
assert list(a) == ['测', '试']
a = 'a测b试c'
assert list(a) == ['a', '测', 'b', '试', 'c']
a = 'a测b试'
assert list(a) == ['a', '测', 'b', '试']
a = '测b试c'
assert list(a) == ['测', 'b', '试', 'c']
a = '测b'
assert list(a) == ['测', 'b']
a = 'b'
assert list(a) == ['b']
a = '测'
assert list(a) == ['测']

assert '\b\b\b' == '\x08\x08\x08'
stack=[1,2,3,4]; assert f"{stack[2:]}" == '[3, 4]'

assert repr('\x1f\x1e\x1f') == '\'\\x1f\\x1e\\x1f\''


assert hex(-42) == '-0x2a'
assert hex(42) == '0x2a'

assert hex(0) == '0x0'
assert hex(1) == '0x1'
assert hex(15) == '0xf'
assert hex(16) == '0x10'
assert hex(255) == '0xff'
assert hex(256) == '0x100'
assert hex(257) == '0x101'
assert hex(17) == '0x11'

import c
assert repr(c.NULL) == '<void* at 0x0>'
assert repr(c.void_p(1)) == '<void* at 0x1>'
assert repr(c.void_p(15)) == '<void* at 0xf>'
assert repr(c.void_p(16)) == '<void* at 0x10>'
assert repr(c.void_p(255)) == '<void* at 0xff>'
assert repr(c.void_p(256)) == '<void* at 0x100>'
assert repr(c.void_p(257)) == '<void* at 0x101>'
assert repr(c.void_p(17)) == '<void* at 0x11>'

# random hex test
import random


def test(__min, __max):
    for _ in range(100):
        num = random.randint(__min, __max)
        hex_num = hex(num)
        assert eval(hex_num) == num
        if num >= 0:
            assert repr(c.void_p(num)) == f'<void* at 0x{hex_num[2:]}>'

test(0, 100)
test(0, 100000)
test(-100, 100)
test(-100000, 100000)
test(-2**30, 2**30)


a = '123'
assert a.index('2') == 1
assert a.index('1') == 0
assert a.index('3') == 2

assert a.index('2', 1) == 1
assert a.index('1', 0) == 0

try:
    a.index('1', 1)
    exit(1)
except ValueError:
    pass

try:
    a.index('1', -1)
    exit(1)
except ValueError:
    pass

assert a.find('1') == 0
assert a.find('1', 1) == -1

try:
    a.find('1', -1)
    exit(1)
except ValueError:
    pass
