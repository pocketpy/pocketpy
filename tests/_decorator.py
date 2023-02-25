
def cache(f):
    def wrapper(*args):
        if not hasattr(f, 'cache'):
            f.cache = {}
        key = args
        if key not in f.cache:
            f.cache[key] = f(*args)
        return f.cache[key]
    return wrapper

@cache
def fib(n):
    if n < 2:
        return n
    return fib(n-1) + fib(n-2)

assert fib(32) == 2178309