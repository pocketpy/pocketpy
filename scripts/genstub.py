import os

with open('include/pocketpy/pocketpy_c.h') as f:
    lines = f.readlines()

a = []
for line in lines:
    if line.startswith("PK_EXPORT"):
        _, ret, *body = line.split()
    else:
        continue
    body = ' '.join(body)
    assert body.endswith(";")
    body = body[:-1]

    if '(pkpy_vm*' in body:
        body = body.replace('(pkpy_vm*', '(pkpy_vm* vm')

    if ret == 'void':
        mock_string = ''
    else:
        mock_string = ' '*4 + ret + ' returnValue;\n    return returnValue;'

    a.append(
        ret + ' ' + body + ' {\n' + mock_string + '\n}\n'
    )

# use LF line endings instead of CRLF
with open('src2/pocketpy_c.c', 'wt', encoding='utf-8', newline='\n') as f:
    f.write('''
#include "pocketpy_c.h"

#ifdef _WIN32
#pragma warning(disable: 4700)
#endif
            
''')
    f.write('\n'.join(a))
