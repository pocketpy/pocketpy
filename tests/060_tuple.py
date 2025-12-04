tup = ('Google', 'Runoob', 'Taobao', 'Wiki', 'Weibo','Weixin')
a,b = 1,2
assert a == 1
assert b == 2
a,b = b,a
assert a == 2
assert b == 1
assert len(tup) == 6

# empty tuple
a = tuple([])
assert len(a) == 0

assert (1,) == tuple([1])
assert (1,2,) == tuple([1,2])

a = 1,
assert a == (1,)

l = (1,2,3,4)
assert l[2] == 3
assert l[-1] == 4
assert l[:32] == (1,2,3,4)
assert l[32:] == tuple([])
assert l[1:4] == (2,3,4)
assert l[-1:-3] == tuple([])
assert l[-3:-1] == (2,3)

# test repr
assert repr((1,)) == '(1,)'
assert repr((1,2,)) == '(1, 2)'
assert repr((1,2,(3,4))) == '(1, 2, (3, 4))'
assert repr(tuple()) == '()'

# test in and not in
assert 1 in (1, 2, 3)
assert 4 not in (1, 2, 3)

# test < and == and !=
assert (1,2) == (1,2)
assert (2,1) == (2,1)
assert (1,) == (1,)
assert (1,2) != (1,3)
assert (1,2) != (1,2,3)
assert (1,2) != (1,)

assert (1,2) < (1,3)
assert (1,2) < (2,1)
assert (1,2) < (2,2)
assert (1,2) < (1,2,3)
assert (1,2) < (1,2,1)
assert (1,2) < (1,2,2)


