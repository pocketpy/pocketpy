import re
from dataclasses import dataclass

NEWLINE = '\n'

@dataclass
class Function:
    name: str
    args: str
    ret: str
    comment: str
    is_py_raise: bool
    is_py_return: bool

    def signature(self):
        tmp = f"PK_EXPORT {self.ret} {self.name}{self.args}"
        return tmp + ';'
    
    def badges(self):
        res = []
        if self.is_py_raise:
            res.append('[!badge text="raise" variant="danger"]')
        if self.is_py_return:
            res.append('[!badge text="return"]')
        return ' '.join(res)

    def markdown(self):
        lines = [
            f"### {self.name}" + f" {self.badges()}",
            f"```c",
            self.comment,
            f"{self.signature()}",
            f"```",
        ]
        return '\n'.join(lines)

with open('include/pocketpy/pocketpy.h') as f:
    header = f.read()

matches = re.finditer(r"((?:/// [^\n]+[\n])*?)PK_EXPORT\s+(\w+)\s+(\w+)(\(.*?\))\s*(PY_RAISE)?\s*(PY_RETURN)?\s*;", header, re.DOTALL)
#                       ^1 comment                        ^2 ret  ^3    ^4 args    ^5 py_raise?  ^6 py_return?

functions: list[Function] = []
for match in matches:
    functions.append(Function(
        name=match[3],
        args=match[4],
        ret=match[2],
        comment=match[1].strip(),
        is_py_raise=bool(match[5]),
        is_py_return=bool(match[6])
    ))
    # print(functions[-1])


# generate markdown
with open('docs/C-API/functions.md', 'w', newline='\n') as f:
    f.write('\n'.join([
        '---',
        'title: Functions',
        'icon: dot',
        'order: 0',
        '---',
        '\n\n',
    ]))
    for function in functions:
        f.write(function.markdown())
        f.write('\n\n')