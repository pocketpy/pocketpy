def f(n):
    if n == 900:
        return -1
    return f(n + 1)

assert f(0) == -1
