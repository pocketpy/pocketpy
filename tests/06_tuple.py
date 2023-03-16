tup = ('Google', 'Runoob', 'Taobao', 'Wiki', 'Weibo','Weixin')
a,b = 1,2
assert a == 1
assert b == 2
a,b = b,a
assert a == 2
assert b == 1
assert len(tup) == 6