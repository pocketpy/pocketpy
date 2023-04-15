def fib(n):
    if n < 2:
        return n
    return fib(n-1) + fib(n-2)

assert fib(32) == 2178309

# from dis import dis
# dis(fib)
# 7049155 calls