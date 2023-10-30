def Counter(iterable):
    a = {}
    for x in iterable:
        if x in a:
            a[x] += 1
        else:
            a[x] = 1
    return a

class defaultdict:
    def __init__(self, default_factory) -> None:
        self.default_factory = default_factory
        self._a = {}

    def __getitem__(self, key):
        if key not in self._a:
            self._a[key] = self.default_factory()
        return self._a[key]
        
    def __setitem__(self, key, value):
        self._a[key] = value

    def __delitem__(self, key):
        del self._a[key]

    def __repr__(self) -> str:
        return f"defaultdict({self.default_factory}, {self._a})"
    
    def __eq__(self, __o: object) -> bool:
        if not isinstance(__o, defaultdict):
            return False
        if self.default_factory != __o.default_factory:
            return False
        return self._a == __o._a
    
    def __iter__(self):
        return iter(self._a)

    def __contains__(self, key):
        return key in self._a
    
    def __len__(self):
        return len(self._a)

    def keys(self):
        return self._a.keys()
    
    def values(self):
        return self._a.values()
    
    def items(self):
        return self._a.items()

    def pop(self, *args):
        return self._a.pop(*args)

    def clear(self):
        self._a.clear()

    def copy(self):
        new_dd = defaultdict(self.default_factory)
        new_dd._a = self._a.copy()
        return new_dd
    
    def get(self, key, default):
        return self._a.get(key, default)
    
    def update(self, other):
        self._a.update(other)
