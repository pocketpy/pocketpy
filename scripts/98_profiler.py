import time

def costly_func(n: int):
    time.sleep(n)               # 2s
    time.sleep(1)               # 1s

x = 1
y = 2
costly_func(2)      # 3s

time.sleep(1)       # 1s

def new_func(a, b, c):
    x = a + b
    x = x + c
    return a

def new_func2(a, b, c):
    x = a + b
    x = x + c
    return a
