code = compile("1+2", "<eval>", "eval")
assert eval(code) == 3

src = """
def f(a, b):
    return g(a, b)

def g(a, b):
    c = f(a, b)
    d = g(a, b)
    return c + d
"""

code = compile(src, "<12>", "exec")
exec(code)
f(1, 2)
