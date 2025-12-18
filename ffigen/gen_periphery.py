import pcpp
import pycparser
from ffigen.library import Library
from ffigen.converters import set_vmath_converter, set_enum_converter
from ffigen.meta import Header
import os

file_dir = os.path.dirname(os.path.abspath(__file__))

path = '3rd/periphery/include/periphery.h'
code = pcpp.CmdPreprocessor([None, path, '-o', 'tmp.h', '-I', os.path.join(file_dir, 'libc_include')])

mapping = {
    'enum serial_parity parity': 'serial_parity_t parity',
}
# remap tmp.h
with open('tmp.h', 'r') as f:
    content = f.read()
for k, v in mapping.items():
    content = content.replace(k, v)
with open('tmp.h', 'w') as f:
    f.write(content)

ast = pycparser.parse_file('tmp.h')
os.remove('tmp.h')

header = Header()
header.build(ast)

lib = Library.from_header('periphery', header)

lib.build(
    includes=['periphery.h'],
    glue_dir='3rd/periphery/src',
    stub_dir='include/typings',
)
