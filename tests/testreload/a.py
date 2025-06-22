import os

class MyClass:
    value = os.environ['TEST_RELOAD_VALUE']

    def some_func(self):
        return self.value
    
    @staticmethod
    def get_xy():
        g = globals()
        return g.get('x', 0), g.get('y', 0)


if os.environ['SET_X'] == '1':
    x = 1
elif os.environ['SET_Y'] == '1':
    y = 1