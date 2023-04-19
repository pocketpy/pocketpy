a = [1, 2, 3]
a = iter(a)

total = 0

while True:
    obj = next(a)
    if obj is StopIteration:
        break
    total += obj

assert total == 6
