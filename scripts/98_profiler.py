from pkpy import *
import time

def costly_func(n: int):
    time.sleep(n)

x = 1
y = 2
costly_func(2)

time.sleep(1)
