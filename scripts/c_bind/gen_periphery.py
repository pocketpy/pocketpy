import pcpp
import pycparser
from c_bind import Library, set_vmath_converter, set_enum_converters
from c_bind.meta import Header
import os

file_dir = os.path.dirname(os.path.abspath(__file__))

path = '3rd/periphery/c-periphery/src/gpio.h'
code = pcpp.CmdPreprocessor([None, path, '-o', 'tmp.h', '-I', os.path.join(file_dir, 'libc_include')])

ast = pycparser.parse_file('tmp.h')
os.remove('tmp.h')

header = Header()
header.build(ast)

lib = Library.from_header('periphery', header)

set_enum_converters([enum.name for enum in lib.enums])

lib.build(
    includes=['c-periphery/gpio.h'],
    glue_dir='3rd/periphery/src',
    stub_dir='include/typings'
)
