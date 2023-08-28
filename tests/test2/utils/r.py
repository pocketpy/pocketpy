value = '123'

try:
    from test2.a import g
except ImportError:
    # circular import
    pass