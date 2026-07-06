from inspect import isgeneratorfunction

def f(a, b):
    return a + b

assert not isgeneratorfunction(f)

def g(a, b):
    yield a
    yield b

assert isgeneratorfunction(g)

class A:
    @staticmethod
    def non_gen(a, b):
        return a + b
    @staticmethod
    def gen(a, b):
        yield a
        yield b
    @classmethod
    def non_gen_class(cls, a, b):
        return a + b
    @classmethod
    def gen_class(cls, a, b):
        yield a
        yield b
    def not_gen_instance(self, a, b):
        return a + b
    def gen_instance(self, a, b):
        yield a
        yield b

a = A()
assert not isgeneratorfunction(a.non_gen)
assert isgeneratorfunction(a.gen)
assert not isgeneratorfunction(A.non_gen)
assert isgeneratorfunction(A.gen)

assert not isgeneratorfunction(a.non_gen_class)
assert isgeneratorfunction(a.gen_class)
assert not isgeneratorfunction(A.non_gen_class)
assert isgeneratorfunction(A.gen_class)

assert not isgeneratorfunction(a.not_gen_instance)
assert isgeneratorfunction(a.gen_instance)
assert not isgeneratorfunction(A.not_gen_instance)
assert isgeneratorfunction(A.gen_instance)

# ---------------- inspect.signature ----------------
from inspect import signature, Parameter

# simple positional-or-keyword parameters
def f1(a, b):
    return a + b

sig = signature(f1)
assert str(sig) == '(a, b)'
params = sig.parameters
assert list(params) == ['a', 'b']
assert params['a'].name == 'a'
assert params['a'].kind == Parameter.POSITIONAL_OR_KEYWORD
assert params['a'].default is Parameter.empty
assert params['b'].kind == Parameter.POSITIONAL_OR_KEYWORD

# no parameters
def f2():
    pass

sig = signature(f2)
assert str(sig) == '()'
assert len(sig.parameters) == 0

# default values
def f3(a, b=2, c='x'):
    pass

sig = signature(f3)
assert str(sig) == "(a, b=2, c='x')"
params = sig.parameters
assert params['a'].default is Parameter.empty
assert params['b'].default == 2
assert params['c'].default == 'x'

# *args and **kwargs
def f4(a, *args, **kwargs):
    pass

sig = signature(f4)
assert str(sig) == '(a, *args, **kwargs)'
params = sig.parameters
assert list(params) == ['a', 'args', 'kwargs']
assert params['a'].kind == Parameter.POSITIONAL_OR_KEYWORD
assert params['args'].kind == Parameter.VAR_POSITIONAL
assert params['args'].default is Parameter.empty
assert params['kwargs'].kind == Parameter.VAR_KEYWORD
assert params['kwargs'].default is Parameter.empty

# keyword-only parameters (defaults after *args)
def f5(a, *c, b=1, **d):
    pass

sig = signature(f5)
assert str(sig) == '(a, *c, b=1, **d)'
assert list(sig.parameters) == ['a', 'c', 'b', 'd']
assert sig.parameters['b'].kind == Parameter.KEYWORD_ONLY
assert sig.parameters['b'].default == 1

# lambda
sig = signature(lambda x, y=3: x + y)
assert str(sig) == '(x, y=3)'
assert list(sig.parameters) == ['x', 'y']
assert sig.parameters['y'].default == 3

# generator function
sig = signature(g)
assert str(sig) == '(a, b)'

# methods: bound methods drop `self`, unbound functions keep it
class B:
    def m(self, x, y=1):
        pass
    @staticmethod
    def sm(x, y):
        pass
    @classmethod
    def cm(cls, x):
        pass

b = B()
assert str(signature(B.m)) == '(self, x, y=1)'
assert str(signature(b.m)) == '(x, y=1)'
assert list(signature(b.m).parameters) == ['x', 'y']
assert str(signature(B.sm)) == '(x, y)'
assert str(signature(b.sm)) == '(x, y)'
assert str(signature(B.cm)) == '(x)'
assert str(signature(b.cm)) == '(x)'

# calling signature() on a class inspects __init__ (without `self`)
class C:
    def __init__(self, a, b=5):
        pass

sig = signature(C)
assert str(sig) == '(a, b=5)'
assert list(sig.parameters) == ['a', 'b']

# decorated function (plain wrapper)
def deco(fn):
    def wrapper(*args, **kwargs):
        return fn(*args, **kwargs)
    return wrapper

@deco
def f6(a, b):
    pass

assert str(signature(f6)) == '(*args, **kwargs)'

# Parameter equality / repr basics
p = signature(f3).parameters['b']
assert p.name == 'b'
assert p.default == 2
assert p.kind == Parameter.POSITIONAL_OR_KEYWORD

# kinds are distinct
kinds = [
    Parameter.POSITIONAL_OR_KEYWORD,
    Parameter.VAR_POSITIONAL,
    Parameter.KEYWORD_ONLY,
    Parameter.VAR_KEYWORD,
]
assert len(set(kinds)) == 4

# non-callable raises TypeError
try:
    signature(42)
    print('failed to raise TypeError')
    exit(1)
except TypeError:
    pass

# ---------------- __annotations__ ----------------
def h1(a: int, *args: int, b: str = 'x', c: float = 1.5, **kwargs: str) -> bool:
    pass

assert h1.__annotations__ == {
    'a': 'int',
    'args': 'int',
    'b': 'str',
    'c': 'float',
    'kwargs': 'str',
    'return': 'bool',
}

def h2(a, b: int, c=1):
    pass

assert h2.__annotations__ == {'b': 'int'}

# complex annotation expressions are preserved as written
def h3(p: list[int], q: dict[str, int]) -> 'A | None':
    pass

assert h3.__annotations__ == {'p': 'list[int]', 'q': 'dict[str, int]', 'return': "'A | None'"}

assert (lambda x: x).__annotations__ == {}
