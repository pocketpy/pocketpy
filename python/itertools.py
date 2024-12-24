class chain:
    def __init__(self, *iterable):
        self.iterable = iterable

    @classmethod
    def from_iterable(cls, iterable):
        for it in iterable:
            yield from it

    def __iter__(self):
        for it in self.iterable:
            yield from it
