def f(n):
    for i in range(n):
        yield i

a = [i for i in f(6)]

assert a == [0,1,2,3,4,5]