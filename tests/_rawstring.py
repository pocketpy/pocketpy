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

s = f'''->->{s}<-<-
{123}
'''

assert s == '->->asdasd\nasds1321321321测试\\测试<-<-\n123\n'

assert r''' ' ''' == " ' "