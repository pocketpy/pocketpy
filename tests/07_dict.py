emptyDict = dict()
assert len(emptyDict) == 0
tinydict = {'Name': 'Tom', 'Age': 7, 'Class': 'First'}
assert tinydict['Name'] == 'Tom';assert tinydict['Age'] == 7
tinydict['Age'] = 8;tinydict['School'] = "aaa"
assert tinydict['Age'] == 8;assert tinydict['School'] == "aaa"
del tinydict['Name']
assert len(tinydict) == 3
tinydict.clear()
assert len(tinydict) == 0

dict1 = {'user':'circle','num':[1,2,3]}
dict2 = dict1.copy()
for k,v in dict1.items():
    assert dict2[k] == v

tinydict = {'Name': 'circle', 'Age': 7}
tinydict2 = {'Sex': 'female' }
assert len(tinydict) == 2
assert len(tinydict2) == 1
tinydict.update(tinydict2)
updated_dict = {'Name': 'circle', 'Age': 7, 'Sex': 'female'}
for k,v in tinydict.items():
    assert updated_dict[k] == v
assert len(tinydict) == 3
assert tinydict == updated_dict

dishes = {'eggs': 2, 'sausage': 1, 'bacon': 1, 'spam': 500}
# dict is now ordered
assert dishes.keys() == ('eggs', 'sausage', 'bacon', 'spam')
assert dishes.values() == (2, 1, 1, 500)

d={1:"a",2:"b",3:"c"}
result=[]
for k,v in d.items():
    result.append(k)
    result.append(v)
assert len(result) == 6

del d[2]
assert len(d) == 2
assert d.keys() == (1, 3)
assert d.values() == ('a', 'c')
del d[1]
del d[3]
assert len(d) == 0

# test __eq__
d1 = {1:2, 3:4}
d2 = {3:4, 1:2}
d3 = {1:2, 3:4, 5:6}
assert d1 == d2
assert d1 != d3

a = dict([(1, 2), (3, 4)])
assert a == {1: 2, 3: 4}

assert a.pop(1) == 2
assert a == {3: 4}
assert a.pop(3) == 4
assert a == {}

# unpacking builder
a = {1:2, 3:4}
b = {**a, 5:6, **a}
assert b == {1: 2, 3: 4, 5: 6}

a = {}
b = {**a, 1:2, 3:4}
assert b == {1: 2, 3: 4}

a = {1:2, 3:4, 7:8}
b = {**a, 1:5, 3:6}
c = {**a, **b}
assert c == {1: 5, 3: 6, 7: 8}

a = {}
for i in range(1000):
    a[i] = i
assert len(a) == 1000
for i in range(1000):
    del a[i]
assert len(a) == 0

a = {
    str(i): i
    for i in range(10)
}

for i, s in enumerate(a):
    assert s == str(i)


a = {'g': 0}

a['ball_3'] = 0
a['ball_4'] = 0
assert a.keys() == ('g', 'ball_3', 'ball_4')
del a['ball_3']
assert a.keys() == ('g', 'ball_4')
del a['ball_4']
assert a.keys() == ('g',)
del a['g']
assert len(a) == 0

# ultra test!!
a = {'0': 0, '1': 1}
for i in range(2, 1000):
    a[str(i)] = i
    del a[str(i - 2)]
    assert a[str(i - 1)] == i - 1

a = {'0': 0, '1': 1}
b = ['0', '1']

# dict delete test
data = []
j = 6
for i in range(65535):
    j = ((j*5+1) % 65535)
    data.append(str(j))

for i in range(len(data)):
    z = data[i]
    a[z] = i
    b.append(z)
    if i % 3 == 0:
        y = b.pop()
        del a[y]

# namedict delete test
class A: pass
a = A()
b = ['0', '1']

for i in range(len(data)):
    z = data[i]
    setattr(a, str(z), i)
    b.append(z)
    if i % 3 == 0:
        y = b.pop()
        delattr(a, y)

a = {1: 2, 3: 4}
assert a.pop(1) == 2
try:
    a.pop(1)
    exit(1)
except KeyError:
    pass

assert a.pop(1, None) is None

try:
    a.pop(1, 2, 3)
    exit(1)
except TypeError:
    pass

a = {1: 2, 3: 4}
a['a'] = a
assert repr(a) == "{1: 2, 3: 4, 'a': {...}}"
