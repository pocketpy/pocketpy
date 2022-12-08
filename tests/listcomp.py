a = [i for i in range(10)]
assert a == list(range(10))

a = [i for i in range(10) if i % 2 == 0]
assert a == [0, 2, 4, 6, 8]

a = [i**3 for i in range(10) if i % 2 == 0]
assert a == [0, 8, 64, 216, 512]

a = [1, 2, 3, 4]
assert a.pop() == 4
assert a == [1, 2, 3]
assert a.pop(0) == 1
assert a == [2, 3]
assert a.pop(-2) == 2
assert a == [3]

a = [1, 2, 3, 4]
assert reversed(a) == [4, 3, 2, 1]
assert a == [1, 2, 3, 4]
a = (1, 2, 3, 4)
assert reversed(a) == [4, 3, 2, 1]
assert a == (1, 2, 3, 4)
a = '1234'
assert reversed(a) == ['4', '3', '2', '1']
assert a == '1234'

assert reversed([]) == []
assert reversed('') == []
assert reversed('测试') == ['试', '测']