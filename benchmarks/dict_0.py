# test basic get/set
import random
random.seed(7)

a = {str(i): i for i in range(100)}
a['existed'] = 0
a['missed'] = 0

for i in range(1000000):
    key = str(random.randint(-100, 100))
    if key in a:
        a['existed'] += 1
    else:
        a['missed'] += 1

existed = a['existed']
missed = a['missed']

assert abs(existed - missed) < 10000

# rnd = random.Random(7)
# assert rnd.randint(1, 100) == 16
# assert rnd.randint(1, 100) == 93
# assert rnd.randint(1, 100) == 22