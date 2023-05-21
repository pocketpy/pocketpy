def cache(f):
    def wrapper(*args):
        if not hasattr(f, '__cache__'):
            f.__cache__ = {}
        key = args
        if key not in f.__cache__:
            f.__cache__[key] = f(*args)
        return f.__cache__[key]
    return wrapper