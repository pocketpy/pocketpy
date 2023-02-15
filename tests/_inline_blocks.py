class A: pass
class B: pass
a = A()
assert type(a) is A

x = 0
if x==0: x=1
assert x==1

def f(x, y): return x+y
assert f(1,2)==3

c = 1
if c==0: x=1
elif c==1: x=2
else: x=3
assert x==2

def f1(x): return x+1
assert f1(1)==2