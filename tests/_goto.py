a = []

for i in range(10):
    for j in range(10):
        goto .test 
        print(2)
    label .test
    a.append(i)

assert a == list(range(10))

b = False

for i in range(10):
    for j in range(10):
        goto .out
        b = True
label .out
assert not b