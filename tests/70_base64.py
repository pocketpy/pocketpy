a = '测试  123'
a = a.encode()

import base64

b = base64.b64encode(a)
c = base64.b64decode(b)

assert a == c
