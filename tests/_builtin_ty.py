##############################################
##String
##############################################

a = ''
b = 'test'
c ='test'
assert len(a) == 0
assert len(b) == 4
assert b == c

assert ''.lower() == '' and ''.upper() == ''
assert 'already+lower '.lower() == 'already+lower '
assert 'ALREADY+UPPER '.upper() == 'ALREADY+UPPER '
assert 'tEST+InG'.lower() == 'test+ing'
assert 'tEST+InG'.upper() == 'TEST+ING'

s = "football"
q = "abcd"
r = "zoo"
str = "this is string example....wow!!!"
assert s[0] == 'f'
assert s[1:4] == 'oot'
assert s[:-1] == 'footbal'
assert s[:10] == 'football'
assert s[-3] == 'a'
assert str[-5:] == 'ow!!!'
assert str[3:-3] == 's is string example....wow'
assert s > q;assert s < r
assert s.replace("foo","ball") == "balltball"
assert s.startswith('f') == True;assert s.endswith('o') == False
assert str.startswith('this') == True;


assert str.split('w') == ['this is string example....', 'o', '!!!']
assert "a,b,c".split(',') == ['a', 'b', 'c']
assert 'a,'.split(',') == ['a', '']
assert 'foo!!bar!!baz'.split('!!') == ['foo', 'bar', 'baz']

str = "*****this is **string** example....wow!!!*****"
s = "123abcrunoob321"
# assert str.strip( '*' ) == "this is **string** example....wow!!!"
# assert s.strip( '12' ) == "3abcrunoob3"

assert str.strip( '*' ) == "this is **string** example....wow!!!"
assert s.strip( '12' ) == "3abcrunoob3"

s = ' asd\n  asd \n'
assert s.strip() == 'asd\n  asd'

s1 = "-"
s2 = ""
seq = ["r","u","n","o","o","b"]
assert s1.join( seq ) == "r-u-n-o-o-b"
assert s2.join( seq ) == "runoob"

def test(*seq):
    return s1.join(seq)
assert test("r", "u", "n", "o", "o", "b") == "r-u-n-o-o-b"


##num = 6
##assert str(num) == '6'   TypeError: 'str' object is not callable


##############################################
##Lists
##############################################

l = [1,2,3,4]
assert l[2] == 3
assert l[-1] == 4
assert l[:32] == [1,2,3,4]
assert l[32:] == []
assert l[1:4] == [2,3,4]
assert l[-1:-3] == []
assert l[-3:-1] == [2,3]


l1 = [1];l2 = l1;l1.append(2);l3 = [1,1,2]
assert l2[1] == 2
assert l1 == l2
assert l1*3 == [1,2,1,2,1,2]
assert l3.count(1) == 2

member = ['Tom', 'Sunny', 'Honer', 'Lily']
teacher = [1,2,3]
assert len(member + teacher) == 7
assert member[0] == 'Tom'
assert member[-2] == 'Honer'
assert member[0:3] == ['Tom', 'Sunny', 'Honer']

member.remove('Sunny')
assert member == ['Tom', 'Honer', 'Lily']
member.pop()
assert member == ['Tom', 'Honer']
del member[0]
assert member == ['Honer']
member.append('Jack')
assert member == ['Honer','Jack']
member.extend(teacher)
assert member == ['Honer','Jack',1,2,3]
member.insert(1,'Tom')
assert member == ['Honer','Tom','Jack',1,2,3]
member.clear()
assert member == []
member = teacher.copy()
assert member == [1,2,3]

l = []
l.insert(0, 'l')
l.insert(1, 'l')
l.insert(0, 'h')
l.insert(3, 'o')
l.insert(1, 'e')
assert l == ['h', 'e', 'l', 'l', 'o']
assert l[-2] == 'l'

##############################################
##tuple
##############################################

tup = ('Google', 'Runoob', 'Taobao', 'Wiki', 'Weibo','Weixin')
a,b = 1,2
assert a == 1
assert b == 2
a,b = b,a
assert a == 2
assert b == 1
assert len(tup) == 6

##############################################
##dict
##############################################
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
tinydict.update(tinydict2)
updated_dict = {'Name': 'circle', 'Age': 7, 'Sex': 'female'}
for k,v in tinydict.items():
    assert updated_dict[k] == v

dishes = {'eggs': 2, 'sausage': 1, 'bacon': 1, 'spam': 500}
keys = dishes.keys()
values = dishes.values()
assert sorted(keys) == sorted(['eggs', 'sausage', 'bacon', 'spam'])
assert sorted(values) == sorted([2, 1, 1, 500])

d={1:"a",2:"b",3:"c"}
result=[]
for kv in d.items():
    k = kv[0]; v=kv[1]
    result.append(k)
    result.append(v)
assert result == [1, 'a', 2, 'b', 3, 'c']

a = [1,2,3,-1]
assert sorted(a) == [-1,1,2,3]
assert sorted(a, lambda x:-x) == [3,2,1,-1]
assert sorted(a, None, True) == [3,2,1,-1]

assert abs(0) == 0
assert abs(1.0) == 1.0
assert abs(-1.0) == 1.0
assert abs(1) == 1
assert abs(-1) == 1