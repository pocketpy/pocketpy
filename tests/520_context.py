path = []

class A:
    def __init__(self, x):
        self.x = x
        self.path = []

    def __enter__(self):
        path.append('enter')
        return self.x
    
    def __exit__(self, *args):
        path.append('exit')
    

with A(123):
    assert path == ['enter']
assert path == ['enter', 'exit']

path.clear()

with A(123) as a:
    assert path == ['enter']
    assert a == 123
    path.append('in')
assert path == ['enter', 'in', 'exit']

path.clear()

# Test that __exit__ is called even when an exception occurs
class B:
    def __init__(self):
        self.path = []
    
    def __enter__(self):
        path.append('enter')
        return self
    
    def __exit__(self, exc_type, exc_val, exc_tb):
        path.append('exit')
        if exc_type is not None:
            path.append('exception')
        return False  # propagate exception

try:
    with B():
        path.append('before_raise')
        raise ValueError('test')
        path.append('after_raise')  # should not be reached
except ValueError:
    pass

assert path == ['enter', 'before_raise', 'exit', 'exception'], f"Expected ['enter', 'before_raise', 'exit', 'exception'], got {path}"


