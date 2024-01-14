a = []

for i in range(10):         # [0]
    for j in range(10):     # [0-0]
        -> test
        print(2)
    == test ==
    a.append(i)
    for k in range(5):      # [0-1]
        for t in range(7):  # [0-1-0]
            pass

assert a == list(range(10))

b = False

for i in range(10):         # [1]
    for j in range(10):     # [1-0]
        -> out
        b = True
== out ==
assert not b

sum = 0
i = 1

== loop ==
sum += i
i += 1
if i <= 100:
    -> loop

assert sum == 5050

for i in range(4):
    _ = 0
# if there is no op here, the block check will fail
while i: --i
