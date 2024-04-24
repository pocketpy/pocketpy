def Counter(iterable):
    a = {}
    for x in iterable:
        if x in a:
            a[x] += 1
        else:
            a[x] = 1
    return a

from __builtins import _enable_instance_dict

class defaultdict(dict):
    def __init__(self, default_factory, *args):
        super().__init__(*args)
        _enable_instance_dict(self)
        self.default_factory = default_factory

    def __missing__(self, key):
        self[key] = self.default_factory()
        return self[key]

    def __repr__(self) -> str:
        return f"defaultdict({self.default_factory}, {super().__repr__()})"

    def copy(self):
        return defaultdict(self.default_factory, self)

