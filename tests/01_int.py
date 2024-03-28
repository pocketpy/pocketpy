# test int literals
assert 0xffff == 65535
assert 0xAAFFFF == 11206655
assert 0x7fffffff == 2147483647
assert -0xffff == -65535
assert -0xAAFFFF == -11206655
assert -0x7fffffff == -2147483647
# test 64-bit
assert 2**60-1 + 546 - 0xfffffffffffff == 1148417904979477026

# test oct literals
assert 0o1234 == 668
assert 0o17777777777 == 2147483647
assert -0o1234 == -668
assert -0o17777777777 == -2147483647

# test binary literals
assert 0b10010 == 18
assert -0b10010 == -18
assert 0b11111111111111111111111111111111 == 4294967295
assert -0b11111 == -31

# test == != >= <= < >
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

# test __str__, __repr__
assert str(1) == '1'
assert repr(1) == '1'

# test bit_length
assert (1).bit_length() == 1
assert (2).bit_length() == 2
assert (3).bit_length() == 2

assert (-1).bit_length() == 1
assert (-2).bit_length() == 2
assert (-3).bit_length() == 2

assert (123123123123123).bit_length() == 47
assert (-3123123123).bit_length() == 32

# test int()
assert int() == 0
assert int(True) == 1
assert int(False) == 0

assert int(1) == 1
assert int(1.0) == 1
assert int(1.1) == 1
assert int(1.9) == 1
assert int(-1.9) == -1
assert int(1.5) == 1
assert int(-1.5) == -1
assert int("123") == 123

assert int("0x123", 16) == 291
assert int("0o123", 8) == 83
assert int("-0x123", 16) == -291
assert int("-0o123", 8) == -83
assert int("-123") == -123
assert int("+123") == 123

# test >> << & | ^
assert 12 >> 1 == 6
assert 12 << 1 == 24
assert 12 & 1 == 0
assert 12 | 1 == 13
assert 12 ^ 1 == 13

# test high precision int pow
assert 7**21 == 558545864083284007
assert 2**60 == 1152921504606846976
assert -2**60 == -1152921504606846976
assert 4**13 == 67108864
assert (-4)**13 == -67108864

assert ~3 == -4
assert ~-3 == 2
assert ~0 == -1

try:
    1 // 0
    exit(1)
except ZeroDivisionError:
    pass

try:
    1 % 0
    exit(1)
except ZeroDivisionError:
    pass

try:
    2**60 // 0
    exit(1)
except ZeroDivisionError:
    pass

try:
    2**60 % 0
    exit(1)
except ZeroDivisionError:
    pass

try:
    divmod(1, 0)
    exit(1)
except ZeroDivisionError:
    pass

try:
    divmod(2**60, 0)
    exit(1)
except ZeroDivisionError:
    pass

assert not 1 < 2 > 3

try:
    x = eval("231231312312312312312312312312312312314354657553423345632")
    print(f"eval should fail, but got {x!r}")
    exit(1)
except SyntaxError:
    pass

assert int("-5") == -5
assert int("-4") == -4
assert int("-3") == -3
assert int("-2") == -2
assert int("-1") == -1
assert int("0") == 0
assert int("1") == 1
assert int("2") == 2
assert int("3") == 3
assert int("4") == 4
assert int("5") == 5
assert int("6") == 6
assert int("7") == 7
assert int("8") == 8
assert int("9") == 9
assert int("10") == 10
assert int("11") == 11
assert int("12") == 12
assert int("13") == 13
assert int("14") == 14
assert int("15") == 15
assert int("16") == 16

