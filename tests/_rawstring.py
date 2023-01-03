a = r'1\232\\\13'

assert a == '1\\232\\\\\\13'

b = r'测\试'
assert len(b) == 3
assert b == '测\\试'