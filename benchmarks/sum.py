def f(n):
    if n == 0:
        return 0
    return n + f(n-1)

assert f(900) == 405450