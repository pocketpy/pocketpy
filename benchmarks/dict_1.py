# test deletion
rnd = 0
keys = []
while True:
    keys.append(rnd)
    rnd = ((rnd * 5) + 1) & 1023
    if rnd == 0:
        break

assert len(keys) == 1024

a = {k: k for k in keys}

for i in range(10000):
    if i % 2 == 0:
        # del all keys
        for k in keys:
            del a[k]
        assert len(a) == 0
    else:
        # add keys back
        for k in keys:
            a[k] = k
        assert len(a) == len(keys)

assert len(a) == len(keys)
assert list(a.keys()) == keys   # order matters
