# test type hints

def f(x: int) -> int:
    return x + 1

def g(x: int, y: int) -> int:
    return x + y

def h(x: int, y):
    return x + y

def i(x, y: int):
    return x + y

# test type hints with default values

def f(x: int = 1) -> int:
    return x + 1

def g(x: int = 1, y: int = 2) -> int:
    return x + y

def h(x: int = 1, y = 2):
    return x + y

def i(x = 1, y: int = 2):
    return x + y

# test type hints with *args

def f(x: int, *args) -> int:
    return x + len(args)

def g(x: int, y: int, *args) -> int:
    return x + y + len(args)

def h(x: int, y, *args):
    return x + y + len(args)

def i(x, y: int, *args):
    return x + y + len(args)

def j(x, y: int, *args: str) -> int:
    return x + y + len(args)

x: int = 1
y: 'str' = '2'

x: 'list[int]' = [1, 2, 3]
y: 'list[str]' = ['1', '2', '3']

def g(x: 'list[int]', y: 'list[str]') -> 'list[int]':
    return x + y

def z(x: float):
    x: int = 1
    y: 'str' = '2'