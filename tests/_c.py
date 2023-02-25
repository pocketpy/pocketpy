from c import *

p = malloc(10 * sizeof(int32))
p = cast(p, int32)

for i in range(10):
    p[i] = i

for i in range(10):
    assert p[i] == i

free(p)