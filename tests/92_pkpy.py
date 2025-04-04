from pkpy import is_user_defined_type

class A:
    pass

assert is_user_defined_type(A)
assert not is_user_defined_type(int)
assert not is_user_defined_type(dict)