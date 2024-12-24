import itertools

assert list(itertools.chain.from_iterable(("abc", "def"))) == [
    "a",
    "b",
    "c",
    "d",
    "e",
    "f",
]
assert list(itertools.chain("abc", "def")) == ["a", "b", "c", "d", "e", "f"]
