# https://gist.github.com/dean0x7d/df5ce97e4a1a05be4d56d1378726ff92

a = 1
my_locals = {"b": 2}

# With user-defined locals:
exec("""
import sys
assert "sys" in locals()
assert "sys" not in globals()
assert "a" not in locals()
assert "a" in globals()
# print(a)  # checks `locals()` first, fails, but finds it in `globals()`
assert (a == 1), a
assert "b" in locals()
assert "b" not in globals()
# print(b)
assert (b == 2), b
def main():
    assert "sys" not in locals()   # not the same `locals()` as the outer scope
    assert "sys" not in globals()  # and `sys` isn't in `globals()`, same as before
    assert "b" not in locals() # again, not the same `locals()` as the outer scope
main()
""", globals(), my_locals)

assert "sys" in my_locals  # side effect
assert "sys" not in globals()


# With default locals:
exec("""
import sys
assert locals() == {}
assert "sys" in globals()
def main():
    assert "sys" not in locals()  # not the same locals as the outer scope
    assert "sys" in globals()     # but now be can access `sys` via `globals()`
main()
""", globals())

assert "sys" in globals()