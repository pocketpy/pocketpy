def fib(n):
    if n < 2:
        return n
    return fib(n-1) + fib(n-2)

assert fib(36) == 14930352
