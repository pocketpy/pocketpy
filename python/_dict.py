class dict:
    def __init__(self, mapping=None):
        self._capacity = 16
        self._a = [None] * self._capacity
        self._len = 0

        if mapping is not None:
            for k,v in mapping:
                self[k] = v
        
    def __len__(self):
        return self._len

    def __probe(self, key):
        i = hash(key) % self._capacity
        while self._a[i] is not None:
            if self._a[i][0] == key:
                return True, i
            i = (i + 1) % self._capacity
        return False, i

    def __getitem__(self, key):
        ok, i = self.__probe(key)
        if not ok:
            raise KeyError(repr(key))
        return self._a[i][1]

    def __contains__(self, key):
        ok, i = self.__probe(key)
        return ok

    def __setitem__(self, key, value):
        ok, i = self.__probe(key)
        if ok:
            self._a[i][1] = value
        else:
            self._a[i] = [key, value]
            self._len += 1
            if self._len > self._capacity * 0.67:
                self._capacity *= 2
                self.__rehash()

    def __delitem__(self, key):
        ok, i = self.__probe(key)
        if not ok:
            raise KeyError(repr(key))
        self._a[i] = None
        self._len -= 1

    def __rehash(self):
        old_a = self._a
        self._a = [None] * self._capacity
        self._len = 0
        for kv in old_a:
            if kv is not None:
                self[kv[0]] = kv[1]

    def get(self, key, default=None):
        ok, i = self.__probe(key)
        if ok:
            return self._a[i][1]
        return default

    def keys(self):
        for kv in self._a:
            if kv is not None:
                yield kv[0]

    def values(self):
        for kv in self._a:
            if kv is not None:
                yield kv[1]

    def items(self):
        for kv in self._a:
            if kv is not None:
                yield kv[0], kv[1]

    def clear(self):
        self._a = [None] * self._capacity
        self._len = 0

    def update(self, other):
        for k, v in other.items():
            self[k] = v

    def copy(self):
        d = dict()
        for kv in self._a:
            if kv is not None:
                d[kv[0]] = kv[1]
        return d

    def __repr__(self):
        a = [repr(k)+': '+repr(v) for k,v in self.items()]
        return '{'+ ', '.join(a) + '}'

    def __json__(self):
        a = []
        for k,v in self.items():
            if type(k) is not str:
                raise TypeError('json keys must be strings, got ' + repr(k) )
            a.append(k.__json__()+': '+v.__json__())
        return '{'+ ', '.join(a) + '}'
    
    def __eq__(self, __o: object) -> bool:
        if type(__o) is not dict:
            return False
        if len(self) != len(__o):
            return False
        for k in self.keys():
            if k not in __o:
                return False
            if self[k] != __o[k]:
                return False
        return True

    def __ne__(self, __o: object) -> bool:
        return not self.__eq__(__o)