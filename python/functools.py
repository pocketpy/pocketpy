class cache:
    def __init__(self, f):
        self.f = f
        self.cache = {}

    def __call__(self, *args):
        if args not in self.cache:
            self.cache[args] = self.f(*args)
        return self.cache[args]
    
def reduce(function, sequence, initial=...):
    it = iter(sequence)
    if initial is ...:
        try:
            value = next(it)
        except StopIteration:
            raise TypeError("reduce() of empty sequence with no initial value")
    else:
        value = initial
    for element in it:
        value = function(value, element)
    return value

class partial:
    def __init__(self, f, *args, **kwargs):
        self.f = f
        if not callable(f):
            raise TypeError("the first argument must be callable")
        self.args = args
        self.kwargs = kwargs

    def __call__(self, *args, **kwargs):
        kwargs.update(self.kwargs)
        return self.f(*self.args, *args, **kwargs)

