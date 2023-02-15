# generate assert test for int

assert 0xffff == 65535
assert 0xAAFFFF == 11206655
assert 0x7fffffff == 2147483647

# test == != >= <= < >
# generate 2 cases for each operator
assert -1 == -1
assert -1 != 1
assert -1 >= -1
assert -1 <= -1
assert -1 < 1
assert -1 > -2

# test + - * % ** //
assert -1 + 1 == 0
assert -1 - 1 == -2
assert 4 * -1 == -4
assert 5 % 2 == 1
assert 2 ** 3 == 8
assert 4 // 2 == 2
assert 5 // 2 == 2

# test += -= *= //=
x = 3
x += 1
assert x == 4
x -= 1
assert x == 3
x *= 2
assert x == 6
x //= 2
assert x == 3

# generate assert test for float

def eq(a, b):
    dt = a - b
    return dt > -0.001 and dt < 0.001

# test + - * / **
assert eq(1.5 + 3, 4.5)
assert eq(1.5 + 3.9, 5.4)
assert eq(5.3 - 2.5, 2.8)
assert eq(0.2**2, 0.04)
assert eq(4**(-1.0), 0.25)
assert eq(2/1, 2)
assert eq(3/2.0, 1.5)
assert eq(1/9, 0.11111)

# test += -= *= /=
x = 3.0
x += 1
assert eq(x, 4.0)
x -= 1
assert eq(x, 3.0)
x *= 2
assert eq(x, 6.0)
x /= 1.8
assert eq(x, 3.3333)

# generate assert test for bool

assert True == True
assert True != False
assert False == False
assert False != True

# test and/or/not
assert True and True
assert not (True and False)
assert True or True
assert True or False
assert not False
assert not (not True)

assert bool(0) == False
assert bool(1) == True
assert bool([]) == False
assert bool("abc") == True
assert bool([1,2]) == True
assert bool('') == False

# generate assert test for str

assert 'testing' == 'test' + 'ing'
assert 'testing' != 'test' + 'ing2'
assert 'testing' < 'test' + 'ing2'
assert 'testing5' > 'test' + 'ing1'

# test + *=
assert 'abc' + 'def' == 'abcdef'
assert 'abc' * 3 == 'abcabcabc'

# generate assert test for list

assert [1, 2, 3] == [1, 2, 3]
assert [1, 2, 3] != [1, 2, 4]

# test + *=
assert [1, 2, 3] + [4, 5, 6] == [1, 2, 3, 4, 5, 6]
assert [1, 2, 3] * 3 == [1, 2, 3, 1, 2, 3, 1, 2, 3]

# test ?:
a = 5
assert ((a > 3) ? 1 : 0) == 1
assert ((a < 3) ? 1 : 0) == 0

assert eq(round(3.1415926, 2), 3.14)
assert eq(round(3.1415926, 3), 3.142)
assert eq(round(3.1415926, 4), 3.1416)
assert eq(round(-3.1415926, 2), -3.14)
assert eq(round(-3.1415926, 3), -3.142)
assert eq(round(-3.1415926, 4), -3.1416)
assert round(23.2) == 23
assert round(23.8) == 24
assert round(-23.2) == -23
assert round(-23.8) == -24

assert (x := 1) == 1
assert (x := 1, y := 2 ) == (1, 2)
z = (a := 1, b := 2)
print(z)
assert a == 1
assert b == 2

assert (x := (a := 1, b := 2)) == (1, 2)
assert x == 1
assert (x := 0) + 1 == 1
assert (x := 1, y := 2, 3) == (1,2,3)