def f(n):
    def g(x):
        if x==n:
            return n
        return g(x+1)
    return g(0)

assert f(10) == 10