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

assert r''' ' ''' == " ' "

a = 10
assert f'{a}' == '10'
assert f'{a:>10}' == '        10'
assert f'{a:<10}' == '10        '
assert f'{a:<10.2f}' == '10.00     '
assert f'{a:>10.2f}' == '     10.00'
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