def f(n):
    for i in range(n):
        yield i

x = 0
for j in f(5):
    x += j

assert x == 10

a = [i for i in f(6)]

assert a == [0,1,2,3,4,5]