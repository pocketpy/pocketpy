tup = ('Google', 'Runoob', 'Taobao', 'Wiki', 'Weibo','Weixin')
a,b = 1,2
assert a == 1
assert b == 2
a,b = b,a
assert a == 2
assert b == 1
assert len(tup) == 6

# unpack expression
a = 1, 2, 3
b = *a, 4, 5
assert b == (1, 2, 3, 4, 5)

a = tuple([])
b = *a, 1, 2, 3, *a, *a
assert b == (1, 2, 3)