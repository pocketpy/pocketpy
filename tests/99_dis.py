from dis import dis

def f(a):
    for i in range(100000):
        a.append(i)
    a = 0.5
    a = True
    a = '123'
    a = 123, 456
    a = [1, 2, 3]
    return a, a, a

def g(a):
    return f([1,2,3] + a)

assert dis(g) is None
