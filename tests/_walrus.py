assert ( x := 1) == 1
assert ( y := 1) + 2 == 3

a = (y := 4) + 5
assert a == 9
assert y == 4