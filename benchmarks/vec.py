import sys

class vec2:
    def __init__(self, x, y):
        self.x = x
        self.y = y

    def __add__(self, other):
        return vec2(self.x + other.x, self.y + other.y)

    def __eq__(self, other):
        return self.x == other.x and self.y == other.y
    
    def __ne__(self, other):
        return not self == other

x = vec2(0, 0)
for i in range(10000000):
    x += vec2(1, 1)

assert x == vec2(10000000, 10000000)
