from c import *

p = malloc(10 * sizeof(int_))
p = p.cast(int_)

for i in range(10):
    p[i] = i

for i in range(10):
    assert p[i] == i

free(p)