class set:
    def __init__(self, iterable=None):
        iterable = iterable or []
        self._a = {}
        self.update(iterable)

    def add(self, elem):
        self._a[elem] = None
        
    def discard(self, elem):
        self._a.pop(elem, None)

    def remove(self, elem):
        del self._a[elem]
        
    def clear(self):
        self._a.clear()

    def update(self, other):
        for elem in other:
            self.add(elem)

    def __len__(self):
        return len(self._a)
    
    def copy(self):
        return set(self._a.keys())
    
    def __and__(self, other):
        return {elem for elem in self if elem in other}

    def __sub__(self, other):
        return {elem for elem in self if elem not in other}
    
    def __or__(self, other):
        ret = self.copy()
        ret.update(other)
        return ret

    def __xor__(self, other): 
        _0 = self - other
        _1 = other - self
        return _0 | _1

    def union(self, other):
        return self | other

    def intersection(self, other):
        return self & other

    def difference(self, other):
        return self - other

    def symmetric_difference(self, other):      
        return self ^ other
    
    def __eq__(self, other):
        if not isinstance(other, set):
            return NotImplemented
        return len(self ^ other) == 0

    def isdisjoint(self, other):
        return len(self & other) == 0
    
    def issubset(self, other):
        return len(self - other) == 0
    
    def issuperset(self, other):
        return len(other - self) == 0

    def __contains__(self, elem):
        return elem in self._a
    
    def __repr__(self):
        if len(self) == 0:
            return 'set()'
        return '{'+ ', '.join([repr(i) for i in self._a.keys()]) + '}'
    
    def __iter__(self):
        return iter(self._a.keys())