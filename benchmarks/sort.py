import time
_0 = time.time()

import random

a = [random.randint(-100000, 100000) for i in range(1500)]
a = sorted(a)

print(round(time.time()-_0, 6), 's')