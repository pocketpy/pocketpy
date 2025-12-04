a = [
    1,2,3,
    4,5,6
]

assert sum(a) == 21

c = [
    i for i in range(10)
    if i % 2 == 0
]

assert sum(c) == 20

d = (
    1,2,3
)

assert sum(d) == 6

b = {
    'a': 1,
    'b': 2,
    'c': 3
}

assert sum(b.values()) == 6