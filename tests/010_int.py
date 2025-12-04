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

# test __str__, __repr__
assert str(1) == '1'
assert repr(1) == '1'

assert 1 < 2 < 3
assert 4 > 3 >= 3
assert not 1 < 2 > 3

assert abs(0) == 0
assert abs(1) == 1
assert abs(-1) == 1

# test negative % and //
assert -10 % -10 == 0
assert -10 // -10 == 1
assert -10 % -9 == -1
assert -10 // -9 == 1
assert -10 % -8 == -2
assert -10 // -8 == 1
assert -10 % -7 == -3
assert -10 // -7 == 1
assert -10 % -6 == -4
assert -10 // -6 == 1
assert -10 % -5 == 0
assert -10 // -5 == 2
assert -10 % -4 == -2
assert -10 // -4 == 2
assert -10 % -3 == -1
assert -10 // -3 == 3
assert -10 % -2 == 0
assert -10 // -2 == 5
assert -10 % -1 == 0
assert -10 // -1 == 10
assert -10 % 1 == 0
assert -10 // 1 == -10
assert -10 % 2 == 0
assert -10 // 2 == -5
assert -10 % 3 == 2
assert -10 // 3 == -4
assert -10 % 4 == 2
assert -10 // 4 == -3
assert -10 % 5 == 0
assert -10 // 5 == -2
assert -10 % 6 == 2
assert -10 // 6 == -2
assert -10 % 7 == 4
assert -10 // 7 == -2
assert -10 % 8 == 6
assert -10 // 8 == -2
assert -10 % 9 == 8
assert -10 // 9 == -2
assert -9 % -10 == -9
assert -9 // -10 == 0
assert -9 % -9 == 0
assert -9 // -9 == 1
assert -9 % -8 == -1
assert -9 // -8 == 1
assert -9 % -7 == -2
assert -9 // -7 == 1
assert -9 % -6 == -3
assert -9 // -6 == 1
assert -9 % -5 == -4
assert -9 // -5 == 1
assert -9 % -4 == -1
assert -9 // -4 == 2
assert -9 % -3 == 0
assert -9 // -3 == 3
assert -9 % -2 == -1
assert -9 // -2 == 4
assert -9 % -1 == 0
assert -9 // -1 == 9
assert -9 % 1 == 0
assert -9 // 1 == -9
assert -9 % 2 == 1
assert -9 // 2 == -5
assert -9 % 3 == 0
assert -9 // 3 == -3
assert -9 % 4 == 3
assert -9 // 4 == -3
assert -9 % 5 == 1
assert -9 // 5 == -2
assert -9 % 6 == 3
assert -9 // 6 == -2
assert -9 % 7 == 5
assert -9 // 7 == -2
assert -9 % 8 == 7
assert -9 // 8 == -2
assert -9 % 9 == 0
assert -9 // 9 == -1
assert -8 % -10 == -8
assert -8 // -10 == 0
assert -8 % -9 == -8
assert -8 // -9 == 0
assert -8 % -8 == 0
assert -8 // -8 == 1
assert -8 % -7 == -1
assert -8 // -7 == 1
assert -8 % -6 == -2
assert -8 // -6 == 1
assert -8 % -5 == -3
assert -8 // -5 == 1
assert -8 % -4 == 0
assert -8 // -4 == 2
assert -8 % -3 == -2
assert -8 // -3 == 2
assert -8 % -2 == 0
assert -8 // -2 == 4
assert -8 % -1 == 0
assert -8 // -1 == 8
assert -8 % 1 == 0
assert -8 // 1 == -8
assert -8 % 2 == 0
assert -8 // 2 == -4
assert -8 % 3 == 1
assert -8 // 3 == -3
assert -8 % 4 == 0
assert -8 // 4 == -2
assert -8 % 5 == 2
assert -8 // 5 == -2
assert -8 % 6 == 4
assert -8 // 6 == -2
assert -8 % 7 == 6
assert -8 // 7 == -2
assert -8 % 8 == 0
assert -8 // 8 == -1
assert -8 % 9 == 1
assert -8 // 9 == -1
assert -7 % -10 == -7
assert -7 // -10 == 0
assert -7 % -9 == -7
assert -7 // -9 == 0
assert -7 % -8 == -7
assert -7 // -8 == 0
assert -7 % -7 == 0
assert -7 // -7 == 1
assert -7 % -6 == -1
assert -7 // -6 == 1
assert -7 % -5 == -2
assert -7 // -5 == 1
assert -7 % -4 == -3
assert -7 // -4 == 1
assert -7 % -3 == -1
assert -7 // -3 == 2
assert -7 % -2 == -1
assert -7 // -2 == 3
assert -7 % -1 == 0
assert -7 // -1 == 7
assert -7 % 1 == 0
assert -7 // 1 == -7
assert -7 % 2 == 1
assert -7 // 2 == -4
assert -7 % 3 == 2
assert -7 // 3 == -3
assert -7 % 4 == 1
assert -7 // 4 == -2
assert -7 % 5 == 3
assert -7 // 5 == -2
assert -7 % 6 == 5
assert -7 // 6 == -2
assert -7 % 7 == 0
assert -7 // 7 == -1
assert -7 % 8 == 1
assert -7 // 8 == -1
assert -7 % 9 == 2
assert -7 // 9 == -1
assert -6 % -10 == -6
assert -6 // -10 == 0
assert -6 % -9 == -6
assert -6 // -9 == 0
assert -6 % -8 == -6
assert -6 // -8 == 0
assert -6 % -7 == -6
assert -6 // -7 == 0
assert -6 % -6 == 0
assert -6 // -6 == 1
assert -6 % -5 == -1
assert -6 // -5 == 1
assert -6 % -4 == -2
assert -6 // -4 == 1
assert -6 % -3 == 0
assert -6 // -3 == 2
assert -6 % -2 == 0
assert -6 // -2 == 3
assert -6 % -1 == 0
assert -6 // -1 == 6
assert -6 % 1 == 0
assert -6 // 1 == -6
assert -6 % 2 == 0
assert -6 // 2 == -3
assert -6 % 3 == 0
assert -6 // 3 == -2
assert -6 % 4 == 2
assert -6 // 4 == -2
assert -6 % 5 == 4
assert -6 // 5 == -2
assert -6 % 6 == 0
assert -6 // 6 == -1
assert -6 % 7 == 1
assert -6 // 7 == -1
assert -6 % 8 == 2
assert -6 // 8 == -1
assert -6 % 9 == 3
assert -6 // 9 == -1
assert -5 % -10 == -5
assert -5 // -10 == 0
assert -5 % -9 == -5
assert -5 // -9 == 0
assert -5 % -8 == -5
assert -5 // -8 == 0
assert -5 % -7 == -5
assert -5 // -7 == 0
assert -5 % -6 == -5
assert -5 // -6 == 0
assert -5 % -5 == 0
assert -5 // -5 == 1
assert -5 % -4 == -1
assert -5 // -4 == 1
assert -5 % -3 == -2
assert -5 // -3 == 1
assert -5 % -2 == -1
assert -5 // -2 == 2
assert -5 % -1 == 0
assert -5 // -1 == 5
assert -5 % 1 == 0
assert -5 // 1 == -5
assert -5 % 2 == 1
assert -5 // 2 == -3
assert -5 % 3 == 1
assert -5 // 3 == -2
assert -5 % 4 == 3
assert -5 // 4 == -2
assert -5 % 5 == 0
assert -5 // 5 == -1
assert -5 % 6 == 1
assert -5 // 6 == -1
assert -5 % 7 == 2
assert -5 // 7 == -1
assert -5 % 8 == 3
assert -5 // 8 == -1
assert -5 % 9 == 4
assert -5 // 9 == -1
assert -4 % -10 == -4
assert -4 // -10 == 0
assert -4 % -9 == -4
assert -4 // -9 == 0
assert -4 % -8 == -4
assert -4 // -8 == 0
assert -4 % -7 == -4
assert -4 // -7 == 0
assert -4 % -6 == -4
assert -4 // -6 == 0
assert -4 % -5 == -4
assert -4 // -5 == 0
assert -4 % -4 == 0
assert -4 // -4 == 1
assert -4 % -3 == -1
assert -4 // -3 == 1
assert -4 % -2 == 0
assert -4 // -2 == 2
assert -4 % -1 == 0
assert -4 // -1 == 4
assert -4 % 1 == 0
assert -4 // 1 == -4
assert -4 % 2 == 0
assert -4 // 2 == -2
assert -4 % 3 == 2
assert -4 // 3 == -2
assert -4 % 4 == 0
assert -4 // 4 == -1
assert -4 % 5 == 1
assert -4 // 5 == -1
assert -4 % 6 == 2
assert -4 // 6 == -1
assert -4 % 7 == 3
assert -4 // 7 == -1
assert -4 % 8 == 4
assert -4 // 8 == -1
assert -4 % 9 == 5
assert -4 // 9 == -1
assert -3 % -10 == -3
assert -3 // -10 == 0
assert -3 % -9 == -3
assert -3 // -9 == 0
assert -3 % -8 == -3
assert -3 // -8 == 0
assert -3 % -7 == -3
assert -3 // -7 == 0
assert -3 % -6 == -3
assert -3 // -6 == 0
assert -3 % -5 == -3
assert -3 // -5 == 0
assert -3 % -4 == -3
assert -3 // -4 == 0
assert -3 % -3 == 0
assert -3 // -3 == 1
assert -3 % -2 == -1
assert -3 // -2 == 1
assert -3 % -1 == 0
assert -3 // -1 == 3
assert -3 % 1 == 0
assert -3 // 1 == -3
assert -3 % 2 == 1
assert -3 // 2 == -2
assert -3 % 3 == 0
assert -3 // 3 == -1
assert -3 % 4 == 1
assert -3 // 4 == -1
assert -3 % 5 == 2
assert -3 // 5 == -1
assert -3 % 6 == 3
assert -3 // 6 == -1
assert -3 % 7 == 4
assert -3 // 7 == -1
assert -3 % 8 == 5
assert -3 // 8 == -1
assert -3 % 9 == 6
assert -3 // 9 == -1
assert -2 % -10 == -2
assert -2 // -10 == 0
assert -2 % -9 == -2
assert -2 // -9 == 0
assert -2 % -8 == -2
assert -2 // -8 == 0
assert -2 % -7 == -2
assert -2 // -7 == 0
assert -2 % -6 == -2
assert -2 // -6 == 0
assert -2 % -5 == -2
assert -2 // -5 == 0
assert -2 % -4 == -2
assert -2 // -4 == 0
assert -2 % -3 == -2
assert -2 // -3 == 0
assert -2 % -2 == 0
assert -2 // -2 == 1
assert -2 % -1 == 0
assert -2 // -1 == 2
assert -2 % 1 == 0
assert -2 // 1 == -2
assert -2 % 2 == 0
assert -2 // 2 == -1
assert -2 % 3 == 1
assert -2 // 3 == -1
assert -2 % 4 == 2
assert -2 // 4 == -1
assert -2 % 5 == 3
assert -2 // 5 == -1
assert -2 % 6 == 4
assert -2 // 6 == -1
assert -2 % 7 == 5
assert -2 // 7 == -1
assert -2 % 8 == 6
assert -2 // 8 == -1
assert -2 % 9 == 7
assert -2 // 9 == -1
assert -1 % -10 == -1
assert -1 // -10 == 0
assert -1 % -9 == -1
assert -1 // -9 == 0
assert -1 % -8 == -1
assert -1 // -8 == 0
assert -1 % -7 == -1
assert -1 // -7 == 0
assert -1 % -6 == -1
assert -1 // -6 == 0
assert -1 % -5 == -1
assert -1 // -5 == 0
assert -1 % -4 == -1
assert -1 // -4 == 0
assert -1 % -3 == -1
assert -1 // -3 == 0
assert -1 % -2 == -1
assert -1 // -2 == 0
assert -1 % -1 == 0
assert -1 // -1 == 1
assert -1 % 1 == 0
assert -1 // 1 == -1
assert -1 % 2 == 1
assert -1 // 2 == -1
assert -1 % 3 == 2
assert -1 // 3 == -1
assert -1 % 4 == 3
assert -1 // 4 == -1
assert -1 % 5 == 4
assert -1 // 5 == -1
assert -1 % 6 == 5
assert -1 // 6 == -1
assert -1 % 7 == 6
assert -1 // 7 == -1
assert -1 % 8 == 7
assert -1 // 8 == -1
assert -1 % 9 == 8
assert -1 // 9 == -1
assert 0 % -10 == 0
assert 0 // -10 == 0
assert 0 % -9 == 0
assert 0 // -9 == 0
assert 0 % -8 == 0
assert 0 // -8 == 0
assert 0 % -7 == 0
assert 0 // -7 == 0
assert 0 % -6 == 0
assert 0 // -6 == 0
assert 0 % -5 == 0
assert 0 // -5 == 0
assert 0 % -4 == 0
assert 0 // -4 == 0
assert 0 % -3 == 0
assert 0 // -3 == 0
assert 0 % -2 == 0
assert 0 // -2 == 0
assert 0 % -1 == 0
assert 0 // -1 == 0
assert 0 % 1 == 0
assert 0 // 1 == 0
assert 0 % 2 == 0
assert 0 // 2 == 0
assert 0 % 3 == 0
assert 0 // 3 == 0
assert 0 % 4 == 0
assert 0 // 4 == 0
assert 0 % 5 == 0
assert 0 // 5 == 0
assert 0 % 6 == 0
assert 0 // 6 == 0
assert 0 % 7 == 0
assert 0 // 7 == 0
assert 0 % 8 == 0
assert 0 // 8 == 0
assert 0 % 9 == 0
assert 0 // 9 == 0
assert 1 % -10 == -9
assert 1 // -10 == -1
assert 1 % -9 == -8
assert 1 // -9 == -1
assert 1 % -8 == -7
assert 1 // -8 == -1
assert 1 % -7 == -6
assert 1 // -7 == -1
assert 1 % -6 == -5
assert 1 // -6 == -1
assert 1 % -5 == -4
assert 1 // -5 == -1
assert 1 % -4 == -3
assert 1 // -4 == -1
assert 1 % -3 == -2
assert 1 // -3 == -1
assert 1 % -2 == -1
assert 1 // -2 == -1
assert 1 % -1 == 0
assert 1 // -1 == -1
assert 1 % 1 == 0
assert 1 // 1 == 1
assert 1 % 2 == 1
assert 1 // 2 == 0
assert 1 % 3 == 1
assert 1 // 3 == 0
assert 1 % 4 == 1
assert 1 // 4 == 0
assert 1 % 5 == 1
assert 1 // 5 == 0
assert 1 % 6 == 1
assert 1 // 6 == 0
assert 1 % 7 == 1
assert 1 // 7 == 0
assert 1 % 8 == 1
assert 1 // 8 == 0
assert 1 % 9 == 1
assert 1 // 9 == 0
assert 2 % -10 == -8
assert 2 // -10 == -1
assert 2 % -9 == -7
assert 2 // -9 == -1
assert 2 % -8 == -6
assert 2 // -8 == -1
assert 2 % -7 == -5
assert 2 // -7 == -1
assert 2 % -6 == -4
assert 2 // -6 == -1
assert 2 % -5 == -3
assert 2 // -5 == -1
assert 2 % -4 == -2
assert 2 // -4 == -1
assert 2 % -3 == -1
assert 2 // -3 == -1
assert 2 % -2 == 0
assert 2 // -2 == -1
assert 2 % -1 == 0
assert 2 // -1 == -2
assert 2 % 1 == 0
assert 2 // 1 == 2
assert 2 % 2 == 0
assert 2 // 2 == 1
assert 2 % 3 == 2
assert 2 // 3 == 0
assert 2 % 4 == 2
assert 2 // 4 == 0
assert 2 % 5 == 2
assert 2 // 5 == 0
assert 2 % 6 == 2
assert 2 // 6 == 0
assert 2 % 7 == 2
assert 2 // 7 == 0
assert 2 % 8 == 2
assert 2 // 8 == 0
assert 2 % 9 == 2
assert 2 // 9 == 0
assert 3 % -10 == -7
assert 3 // -10 == -1
assert 3 % -9 == -6
assert 3 // -9 == -1
assert 3 % -8 == -5
assert 3 // -8 == -1
assert 3 % -7 == -4
assert 3 // -7 == -1
assert 3 % -6 == -3
assert 3 // -6 == -1
assert 3 % -5 == -2
assert 3 // -5 == -1
assert 3 % -4 == -1
assert 3 // -4 == -1
assert 3 % -3 == 0
assert 3 // -3 == -1
assert 3 % -2 == -1
assert 3 // -2 == -2
assert 3 % -1 == 0
assert 3 // -1 == -3
assert 3 % 1 == 0
assert 3 // 1 == 3
assert 3 % 2 == 1
assert 3 // 2 == 1
assert 3 % 3 == 0
assert 3 // 3 == 1
assert 3 % 4 == 3
assert 3 // 4 == 0
assert 3 % 5 == 3
assert 3 // 5 == 0
assert 3 % 6 == 3
assert 3 // 6 == 0
assert 3 % 7 == 3
assert 3 // 7 == 0
assert 3 % 8 == 3
assert 3 // 8 == 0
assert 3 % 9 == 3
assert 3 // 9 == 0
assert 4 % -10 == -6
assert 4 // -10 == -1
assert 4 % -9 == -5
assert 4 // -9 == -1
assert 4 % -8 == -4
assert 4 // -8 == -1
assert 4 % -7 == -3
assert 4 // -7 == -1
assert 4 % -6 == -2
assert 4 // -6 == -1
assert 4 % -5 == -1
assert 4 // -5 == -1
assert 4 % -4 == 0
assert 4 // -4 == -1
assert 4 % -3 == -2
assert 4 // -3 == -2
assert 4 % -2 == 0
assert 4 // -2 == -2
assert 4 % -1 == 0
assert 4 // -1 == -4
assert 4 % 1 == 0
assert 4 // 1 == 4
assert 4 % 2 == 0
assert 4 // 2 == 2
assert 4 % 3 == 1
assert 4 // 3 == 1
assert 4 % 4 == 0
assert 4 // 4 == 1
assert 4 % 5 == 4
assert 4 // 5 == 0
assert 4 % 6 == 4
assert 4 // 6 == 0
assert 4 % 7 == 4
assert 4 // 7 == 0
assert 4 % 8 == 4
assert 4 // 8 == 0
assert 4 % 9 == 4
assert 4 // 9 == 0
assert 5 % -10 == -5
assert 5 // -10 == -1
assert 5 % -9 == -4
assert 5 // -9 == -1
assert 5 % -8 == -3
assert 5 // -8 == -1
assert 5 % -7 == -2
assert 5 // -7 == -1
assert 5 % -6 == -1
assert 5 // -6 == -1
assert 5 % -5 == 0
assert 5 // -5 == -1
assert 5 % -4 == -3
assert 5 // -4 == -2
assert 5 % -3 == -1
assert 5 // -3 == -2
assert 5 % -2 == -1
assert 5 // -2 == -3
assert 5 % -1 == 0
assert 5 // -1 == -5
assert 5 % 1 == 0
assert 5 // 1 == 5
assert 5 % 2 == 1
assert 5 // 2 == 2
assert 5 % 3 == 2
assert 5 // 3 == 1
assert 5 % 4 == 1
assert 5 // 4 == 1
assert 5 % 5 == 0
assert 5 // 5 == 1
assert 5 % 6 == 5
assert 5 // 6 == 0
assert 5 % 7 == 5
assert 5 // 7 == 0
assert 5 % 8 == 5
assert 5 // 8 == 0
assert 5 % 9 == 5
assert 5 // 9 == 0
assert 6 % -10 == -4
assert 6 // -10 == -1
assert 6 % -9 == -3
assert 6 // -9 == -1
assert 6 % -8 == -2
assert 6 // -8 == -1
assert 6 % -7 == -1
assert 6 // -7 == -1
assert 6 % -6 == 0
assert 6 // -6 == -1
assert 6 % -5 == -4
assert 6 // -5 == -2
assert 6 % -4 == -2
assert 6 // -4 == -2
assert 6 % -3 == 0
assert 6 // -3 == -2
assert 6 % -2 == 0
assert 6 // -2 == -3
assert 6 % -1 == 0
assert 6 // -1 == -6
assert 6 % 1 == 0
assert 6 // 1 == 6
assert 6 % 2 == 0
assert 6 // 2 == 3
assert 6 % 3 == 0
assert 6 // 3 == 2
assert 6 % 4 == 2
assert 6 // 4 == 1
assert 6 % 5 == 1
assert 6 // 5 == 1
assert 6 % 6 == 0
assert 6 // 6 == 1
assert 6 % 7 == 6
assert 6 // 7 == 0
assert 6 % 8 == 6
assert 6 // 8 == 0
assert 6 % 9 == 6
assert 6 // 9 == 0
assert 7 % -10 == -3
assert 7 // -10 == -1
assert 7 % -9 == -2
assert 7 // -9 == -1
assert 7 % -8 == -1
assert 7 // -8 == -1
assert 7 % -7 == 0
assert 7 // -7 == -1
assert 7 % -6 == -5
assert 7 // -6 == -2
assert 7 % -5 == -3
assert 7 // -5 == -2
assert 7 % -4 == -1
assert 7 // -4 == -2
assert 7 % -3 == -2
assert 7 // -3 == -3
assert 7 % -2 == -1
assert 7 // -2 == -4
assert 7 % -1 == 0
assert 7 // -1 == -7
assert 7 % 1 == 0
assert 7 // 1 == 7
assert 7 % 2 == 1
assert 7 // 2 == 3
assert 7 % 3 == 1
assert 7 // 3 == 2
assert 7 % 4 == 3
assert 7 // 4 == 1
assert 7 % 5 == 2
assert 7 // 5 == 1
assert 7 % 6 == 1
assert 7 // 6 == 1
assert 7 % 7 == 0
assert 7 // 7 == 1
assert 7 % 8 == 7
assert 7 // 8 == 0
assert 7 % 9 == 7
assert 7 // 9 == 0
assert 8 % -10 == -2
assert 8 // -10 == -1
assert 8 % -9 == -1
assert 8 // -9 == -1
assert 8 % -8 == 0
assert 8 // -8 == -1
assert 8 % -7 == -6
assert 8 // -7 == -2
assert 8 % -6 == -4
assert 8 // -6 == -2
assert 8 % -5 == -2
assert 8 // -5 == -2
assert 8 % -4 == 0
assert 8 // -4 == -2
assert 8 % -3 == -1
assert 8 // -3 == -3
assert 8 % -2 == 0
assert 8 // -2 == -4
assert 8 % -1 == 0
assert 8 // -1 == -8
assert 8 % 1 == 0
assert 8 // 1 == 8
assert 8 % 2 == 0
assert 8 // 2 == 4
assert 8 % 3 == 2
assert 8 // 3 == 2
assert 8 % 4 == 0
assert 8 // 4 == 2
assert 8 % 5 == 3
assert 8 // 5 == 1
assert 8 % 6 == 2
assert 8 // 6 == 1
assert 8 % 7 == 1
assert 8 // 7 == 1
assert 8 % 8 == 0
assert 8 // 8 == 1
assert 8 % 9 == 8
assert 8 // 9 == 0
assert 9 % -10 == -1
assert 9 // -10 == -1
assert 9 % -9 == 0
assert 9 // -9 == -1
assert 9 % -8 == -7
assert 9 // -8 == -2
assert 9 % -7 == -5
assert 9 // -7 == -2
assert 9 % -6 == -3
assert 9 // -6 == -2
assert 9 % -5 == -1
assert 9 // -5 == -2
assert 9 % -4 == -3
assert 9 // -4 == -3
assert 9 % -3 == 0
assert 9 // -3 == -3
assert 9 % -2 == -1
assert 9 // -2 == -5
assert 9 % -1 == 0
assert 9 // -1 == -9
assert 9 % 1 == 0
assert 9 // 1 == 9
assert 9 % 2 == 1
assert 9 // 2 == 4
assert 9 % 3 == 0
assert 9 // 3 == 3
assert 9 % 4 == 1
assert 9 // 4 == 2
assert 9 % 5 == 4
assert 9 // 5 == 1
assert 9 % 6 == 3
assert 9 // 6 == 1
assert 9 % 7 == 2
assert 9 // 7 == 1
assert 9 % 8 == 1
assert 9 // 8 == 1
assert 9 % 9 == 0
assert 9 // 9 == 1