from __builtins import next

def zip_longest(a, b):
    a = iter(a)
    b = iter(b)
    while True:
        ai = next(a)
        bi = next(b)
        if ai is StopIteration and bi is StopIteration:
            break
        if ai is StopIteration:
            ai = None
        if bi is StopIteration:
            bi = None
        yield ai, bi
