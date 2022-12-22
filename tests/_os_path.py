import os

# test os.path.join

f = os.path.join

assert f('/', 'a') == '/a'
assert f('/', 'a/', 'b/') == '/a/b/'
assert f('c', 'd') == 'c/d'
assert f('C:\\', 'a') == 'C:/a'
assert f('C:\\', 'a\\', 'b\\') == 'C:/a/b/'
assert f('c\\', 'd/') == 'c/d/'