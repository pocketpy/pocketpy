a = []

for i in range(10):         # [0]
    for j in range(10):     # [0-0]
        $goto test 
        print(2)
    $label test
    a.append(i)
    for k in range(5):      # [0-1]
        for t in range(7):  # [0-1-0]
            pass

assert a == list(range(10))

b = False

for i in range(10):         # [1]
    for j in range(10):     # [1-0]
        $goto out
        b = True
$label out
assert not b