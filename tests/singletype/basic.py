
def compare(a,b):
    d = a-b
    if d > -0.0001 and d < 0.0001:
        return 1
    return 0

s = 'foo'; s += 'bar'
assert s == 'foobar'
assert 1 + 2 * 3 == 7
assert (1 + 2)* 3 == 9
assert compare(1.2*3.5 , 4.2) == 1
assert compare(9.8*(2.5 - 3),-4.9) == 1
assert compare(2.4*8.6,20.64) == 1

assert compare(1.5 + 3,4.5) == 1
assert compare(1.5 + 3.9,5.4) == 1
assert 2 - 1 == 1
assert compare(5.3 - 2.5,2.8) == 1
assert 42 % 40 == 2
assert -15 % 6 == -3     # in python -15 % 6 == 3
assert 2/1 == 2
assert 3//2 == 1
assert 1 - 9 == -8

a = 1
assert -a == -1
assert 'testing'== 'test' + 'ing'

x = 42 
assert x%3 == 0
x = 27 
assert x%8 == 3


assert 2**3 == 8 
assert -2**2 == 4 
assert (-2)**2 == 4
assert compare(0.2**2,0.04) == 1    
x = 4
assert x**4 == 256
assert compare(x**0.5,2) == 1
assert compare(4**(-1.0),0.25) == 1

assert 'abc' * 3 == 'abcabcabc'
assert '' * 1000 == ''
assert 'foo' * 0 == ''


assert 1 < 2
assert 3 > 1
x = 1
y = 8
assert x <= y
assert y >= x
assert x != y

assert 42 in [12, 42, 3.14]
assert 'key' in {'key':'value'}
assert 'a' in 'abc'
assert 'd' not in 'abc'

x = 1
y = 0
assert not x == False
assert not y == True

a = 1
b = 1
c = 0.1
assert (a==b) and (a is not b)       # small int cache
assert a is not c



