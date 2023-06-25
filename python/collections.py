class _LinkedListNode:
    def __init__(self, prev, next, value) -> None:
        self.prev = prev
        self.next = next
        self.value = value

class deque:
    def __init__(self, iterable=None) -> None:
        self.head = _LinkedListNode(None, None, None)
        self.tail = _LinkedListNode(None, None, None)
        self.head.next = self.tail
        self.tail.prev = self.head
        self.size = 0
        if iterable is not None:
            for value in iterable:
                self.append(value)

    def append(self, value):
        node = _LinkedListNode(self.tail.prev, self.tail, value)
        self.tail.prev.next = node
        self.tail.prev = node
        self.size += 1
    
    def appendleft(self, value):
        node = _LinkedListNode(self.head, self.head.next, value)
        self.head.next.prev = node
        self.head.next = node
        self.size += 1

    def pop(self):
        assert self.size > 0
        node = self.tail.prev
        node.prev.next = self.tail
        self.tail.prev = node.prev
        self.size -= 1
        return node.value
    
    def popleft(self):
        assert self.size > 0
        node = self.head.next
        node.next.prev = self.head
        self.head.next = node.next
        self.size -= 1
        return node.value
    
    def copy(self):
        new_list = deque()
        for value in self:
            new_list.append(value)
        return new_list
    
    def __len__(self):
        return self.size
    
    def __iter__(self):
        node = self.head.next
        while node is not self.tail:
            yield node.value
            node = node.next

    def __repr__(self) -> str:
        a = list(self)
        return f"deque({a})"
    
    def __eq__(self, __o: object) -> bool:
        if not isinstance(__o, deque):
            return False
        if len(self) != len(__o):
            return False
        t1, t2 = self.head.next, __o.head.next
        while t1 is not self.tail:
            if t1.value != t2.value:
                return False
            t1, t2 = t1.next, t2.next
        return True

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

    def pop(self, key):
        return self._a.pop(key)
