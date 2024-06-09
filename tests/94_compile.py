assert eval("1+2") == 3

code = compile("1+2", "<eval>", "eval")
# print(code)
assert eval(code) == 3
