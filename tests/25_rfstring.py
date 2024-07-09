a = r'1\232\\\13'

assert a == '1\\232\\\\\\13'

b = r'测\试'
assert len(b) == 3
assert b == '测\\试'

s = '''asdasd
asds1321321321测试测试
'''

assert s == 'asdasd\nasds1321321321测试测试\n'

s = r'''asdasd
asds1321321321测试\测试'''

assert s == 'asdasd\nasds1321321321测试\\测试'

t = 4
assert f'123{t}56789' == '123456789'

b = 123
s = f'''->->{s}<-<-
{b}
'''

assert s == '->->asdasd\nasds1321321321测试\\测试<-<-\n123\n'

x = 1, 2, 3
assert f"""
a = {{{x[0]}, {x[1]}, {x[2]}}}""" == '\na = {1, 2, 3}'

assert r''' ' ''' == " ' "

a = 10
assert f'{a}' == '10'
assert f'{a:>10}' == '        10'
assert f'{a:<10}' == '10        '
assert f'{a:<10.2f}' == '10.00     '
assert f'{a:>10.2f}' == '     10.00'

assert f'{a:^10}' == '    10    '
assert f'{a:^10.2f}' == '  10.00   '

assert f'{a:3d}' == ' 10'
assert f'{a:10d}' == '        10'
assert f'{a:1d}' == '10'
assert f'{a:010}' == '0000000010'
assert f'{a:010d}' == '0000000010'
assert f'{a:010f}' == '010.000000'
assert f'{a:010.2f}' == '0000010.00'
assert f'{a:.2f}' == '10.00'
assert f'{a:.5f}' == '10.00000'

b = '123'
assert f'{b:10}' == '123       '
assert f'{b:>10}' == '       123'
assert f'{b:1}' == '123'
assert f'{b:10s}' == '123       '

obj = object()
obj.b = '123'
assert f'{obj.b:10}' == '123       '
assert f'{obj.b:>10}' == '       123'
assert f'{obj.b:1}' == '123'
assert f'{obj.b:10s}' == '123       '

a = [(1,2), 3, obj]
assert f'{a[0][1]}' == '2'
assert f'abc{a[-1].b:10}==={1234}' == 'abc123       ===1234'

assert f'{"text":10}' == 'text      '
assert f'{"test":*>10}' == '******test'
assert f'{"test":*<10}' == 'test******'
assert f'{"test":*^10}' == '***test***'
assert f'{"test":*^11}' == '***test****'
assert f'{12345:0>10}' == '0000012345'

assert f'{obj.b!r:10}' == "'123'     "
assert f'{obj.b!r:*>10}' == "*****'123'"
assert f'{obj.b!r:1}' == "'123'"
assert f'{obj.b!r:10s}' == "'123'     "

assert f'{"text"!r:10}' == "'text'    "
assert f'{"test"!r:*>10}' == "****'test'"
assert f'{"test"!r:*<10}' == "'test'****"
assert f'{"test"!r:*^10}' == "**'test'**"
assert f'{"test"!r:*^11}' == "**'test'***"
assert f'{12345!r:0>10}' == "0000012345"

# test {{ and }}
assert f'' == ''
assert f'{{}}' == '{}'
assert f'{{{{}}}}' == '{{}}'
assert f'{{' == '{'
assert f'}}' == '}'
assert f'{{{{' == '{{'
assert f'}}}}' == '}}'
a = 123
assert f'={a}' == '=123'
assert f'{a}=' == '123='
assert f'--{a}--' == '--123--'
assert f'{{a}}' == '{a}'
assert f'{{{a}}}' == '{123}'

assert f'{{=}}{a}' == '{=}123'
assert f'{a}{{=}}' == '123{=}'

# assert f'}123' == '123'     # ignore '}'
# assert f'{{{' == '{'        # ignore '{'

class A:
    def __repr__(self):
        return 'A()'
    def __str__(self):
        return 'A'

a = A()
assert f'{a!r:10}' == 'A()       '
assert f'{a!s:10}' == 'A         '
assert f'{a:10}' == 'A         '

assert f'{A()!r:10}' == 'A()       '
assert f'{A()!s:10}' == 'A         '
assert f'{A():10}' == 'A         '