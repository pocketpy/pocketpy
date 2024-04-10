class A:
    def f(self, a, b, c):
        pass

a = A()
for i in range(10000000):
    a.f(1, 2, 3)
    a.f(1, 2, 3)
    a.f(1, 2, 3)
    a.f(1, 2, 3)

