# def cache(f):
#     def wrapper(*args):
#         if not hasattr(f, '__cache__'):
#             f.__cache__ = {}
#         key = args
#         if key not in f.__cache__:
#             f.__cache__[key] = f(*args)
#         return f.__cache__[key]
#     return wrapper

class cache:
    def __init__(self, f):
        self.f = f
        self.cache = {}

    def __call__(self, *args):
        if args not in self.cache:
            self.cache[args] = self.f(*args)
        return self.cache[args]