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
