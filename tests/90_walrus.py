# Test walrus operator (:=)

# Basic usage in if statement
x = [1, 2, 3, 4, 5]
if (n := len(x)) > 3:
    assert n == 5

# Usage in while loop
data = [1, 2, 3, 0, 4, 5]
results = []
i = 0
while (val := data[i]) != 0:
    results.append(val)
    i += 1
assert results == [1, 2, 3]

# Usage in list comprehension filter
values = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
even_squares = [y for x in values if (y := x * x) % 2 == 0]
assert even_squares == [4, 16, 36, 64, 100]

# Walrus in expression context
a = 10
b = (a := a + 5) * 2
assert a == 15
assert b == 30

# Nested parenthesized walrus
result = (x := (y := 3) + 1)
assert x == 4
assert y == 3

# Test function 
def test_walrus_in_function():
    result = (x := (y := 3) + 1)
    assert x == 4
    assert y == 3
    assert result == 4

test_walrus_in_function()
