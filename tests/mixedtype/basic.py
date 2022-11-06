def compare(a,b):
    d = a-b
    if d > -0.0001 and d < 0.0001:
        return 1
    return 0


assert compare(32 + 32.0,64) == 1
assert compare(8855 / 3.2,2767.1875) == 1
#assert 6412//6.5 == 986.0  #TypeError: unsupported operand type(s) for //
assert compare(1054.5*7985,8420182.5) == 1
#assert 4 % 2.0 == 0.0  #TypeError: unsupported operand type(s) for %
l = [3.2,5,10,8.9]
assert 2.3 + l[0] == 5.5
assert 3 + l[1] == 8
assert compare(3/l[2],0.3) == 1
assert 3 // l[1] == 0
assert l[2] % 3 == 1
assert compare(3*l[3],26.7) == 1
assert 'a' * l[1] == 'aaaaa'

assert compare(2.9**2,8.41) == 1
assert compare(2.5**(-1),0.4) == 1 

assert 2.5 > 2
assert 1.6 < 100
assert 1.0 == 1
x = 2.6
y = 5
l = [5.4,8,'40',3.14]
assert x <= y
assert y >= x
assert x != y
assert y < l[0]

str = ['s','bb']
s = 'jack' + str[0]
assert s == 'jacks'
assert str[1] * 3 == 'bbbbbb' 