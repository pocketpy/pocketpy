import pcpp
import pycparser
from c_bind import Library, set_linalg_converter, set_enum_converters
from c_bind.meta import Header
import os

path = '../3rd/box2d/include/box2d/box2d.h'
code = pcpp.CmdPreprocessor([None, path, '-o', 'tmp.h', '--line-directive', '-I', 'libc_include', '-I', '../3rd/box2d/include'])

ast = pycparser.parse_file('tmp.h')
os.remove('tmp.h')

header = Header()
header.build(ast)

header.remove_types({'b2Timer', 'b2DebugDraw'})
header.remove_functions({'b2CreateTimer', 'b2Hash', 'b2DefaultDebugDraw'})

lib = Library.from_header('box2d', header)

set_linalg_converter('b2Vec2', 'vec2')
set_linalg_converter('b2Vec3', 'vec3')

set_enum_converters([enum.name for enum in lib.enums])

lib.build(
    includes=['box2d/box2d.h'],
    glue_dir='../src',
    stub_dir='../include/typings'
)
