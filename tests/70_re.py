# import re

# # test match, search, sub, split

# m = re.search('测试','123测试测试')
# assert m.span() == (3,5)
# assert m.group(0) == '测试'

# assert re.match('测试','123测试测试') is None
# assert re.sub('测试','xxx','123测试12321测试') == '123xxx12321xxx'

# # this is different from cpython, the last empty string is not included
# assert re.split('测试','测试123测试12321测试') == ['', '123', '12321']

# assert re.split(',','123,456,789,10') == ['123', '456', '789', '10']
# assert re.split(',',',123,456,789,10') == ['', '123', '456', '789', '10']
# assert re.split(',','123,456,789,10,') == ['123', '456', '789', '10']

# assert re.match('1','1') is not None