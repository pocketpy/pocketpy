def g(x):
    return x
def f(x):
    return x

assert (g(1), 2) == (1, 2)
assert (
    g(1),
    2
) == (1, 2)

assert f((
    g(1),
    2
)) == (1, 2)