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
keys = dishes.keys()
values = dishes.values()
assert sorted(keys) == sorted(['eggs', 'sausage', 'bacon', 'spam'])
assert sorted(values) == sorted([2, 1, 1, 500])

d={1:"a",2:"b",3:"c"}
result=[]
for k,v in d.items():
    result.append(k)
    result.append(v)
assert len(result) == 6
assert set(result) == set([1, 'a', 2, 'b', 3, 'c'])

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