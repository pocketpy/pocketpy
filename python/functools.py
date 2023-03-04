def cache(f):
    def wrapper(*args):
        if not hasattr(f, 'cache'):
            f.cache = {}
        key = args
        if key not in f.cache:
            f.cache[key] = f(*args)
        return f.cache[key]
    return wrapper