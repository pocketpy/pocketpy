a = {i: i**2 for i in range(10)}
assert a == {0: 0, 1: 1, 2: 4, 3: 9, 4: 16, 5: 25, 6: 36, 7: 49, 8: 64, 9: 81}

a = {i: i**2 for i in range(10) if i % 2 == 0}
assert a == {0: 0, 2: 4, 4: 16, 6: 36, 8: 64}

b = {k:v for k,v in a.items()}
assert b == a