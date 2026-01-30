# https://github.com/pocketpy/pocketpy/issues/456

module_code = '''
CONSTANT = 42

def hello(name):
    return "Hello, " + name
'''

namespace = {}

exec(module_code, namespace)

assert namespace['CONSTANT'] == 42
assert namespace['hello']('world') == "Hello, world"
# print("Constant:", namespace['CONSTANT'])
# print("Function result:", namespace['hello']('world'))
