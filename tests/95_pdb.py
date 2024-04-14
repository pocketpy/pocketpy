a = 1
b = 2

print(a, b)


def f(a, b):
    breakpoint()
    b = a + b
    print(a, b)
    return b

f(1, 2)
