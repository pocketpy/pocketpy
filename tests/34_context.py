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

with A(123) as a:
    assert path == ['enter']
    -> end
    path.append('in')

== end ==
assert path == ['enter']

