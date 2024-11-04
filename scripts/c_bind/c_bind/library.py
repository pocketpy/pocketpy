from .schema import *
from .writer import Writer
from .enum import gen_enum
from .struct import gen_struct
from .function import gen_function

from typing import TYPE_CHECKING
if TYPE_CHECKING:
    from .meta import Header

class Library:
    def __init__(self, name: str) -> None:
        self.name = name
        # ['defines', 'structs', 'aliases', 'enums', 'callbacks', 'functions']
        self.structs = []   # type: list[Struct]
        self.aliases = []   # type: list[Alias]
        self.enums = []     # type: list[Enum]
        self.functions = [] # type: list[Function]
        self.callbacks = set() # type: set[str]

    def set_includes(self, includes: list[str]):
        self.user_includes.extend(includes)

    def build(self, *, glue_dir='.', stub_dir='.', includes: list[str] = None):
        self.remove_unsupported()

        w, pyi_w = Writer(), Writer()

        pyi_w.write('from linalg import vec2, vec3, vec2i, vec3i, mat3x3')
        pyi_w.write('from typing import overload')
        pyi_w.write('intptr = int')
        pyi_w.write('')

        w.write('#include "pocketpy.h"')
        w.write(f'#include "string.h"')
        
        if includes:
            for include in includes:
                w.write(f'#include "{include}"')

        w.write('')
        w.write('#define ADD_ENUM(name) py_newint(py_emplacedict(mod, py_name(#name)), name)')
        w.write('')
        w.write('static bool struct__address__(int argc, py_Ref argv) {')
        w.indent()
        w.write('PY_CHECK_ARGC(1);')
        w.write('void* ud = py_touserdata(argv);')
        w.write('py_newint(py_retval(), (py_i64)ud);')
        w.write('return true;')
        w.dedent()
        w.write('}')
        w.write('')

        for alias in self.aliases:
            w.write(f'#define tp_user_{alias.name} tp_user_{alias.type}')
        w.write('')

        reg_exprs = [
            gen_struct(w, pyi_w, struct)
            for struct in self.structs
        ]

        w.write('/* functions */')
        for function in self.functions:
            gen_function(w, pyi_w, function)
        
        w.write(f'void py__add_module_{self.name}() {{')
        w.indent()

        w.write(f'py_GlobalRef mod = py_newmodule("{self.name}");')

        w.write('/* structs */')
        for reg_expr in reg_exprs:
            w.write(reg_expr)

        w.write('/* aliases */')
        pyi_w.write('# aliases')
        for alias in self.aliases:
            w.write(f'py_setdict(mod, py_name("{alias.name}"), py_getdict(mod, py_name("{alias.type}")));')
            pyi_w.write(f'{alias.name} = {alias.type}')

        w.write('/* functions */')
        for function in self.functions:
            w.write(f'py_bindfunc(mod, "{function.name}", &cfunc__{function.name});')

        w.write('/* enums */')
        pyi_w.write('# enums')
        for enum in self.enums:
            gen_enum(w, pyi_w, enum)
        w.dedent()
        w.write('}')
        with open(f'{glue_dir}/{self.name}.c', 'w') as f:
            f.write(str(w))
        with open(f'{stub_dir}/{self.name}.pyi', 'w') as f:
            f.write(str(pyi_w))

    def remove_unsupported(self):
        functions = []
        for f in self.functions:
            if f.params and f.params[-1].type == '...':
                print('[WARN]', f.signature(), 'is variadic')
                continue
            for p in f.params:
                if p.type in self.callbacks:
                    print('[WARN]', f.signature(), 'has callback param')
                    break
            else:
                functions.append(f)
        self.functions.clear()
        self.functions.extend(functions)
                
    @staticmethod
    def from_raylib(data: dict):
        self = Library('raylib')
        for struct in data['structs']:
            self.structs.append(Struct(
                name=struct['name'],
                desc=struct['description'],
                fields=[StructField(
                    type=field['type'],
                    name=field['name'],
                    desc=field['description']
                ) for field in struct['fields']]
            ))
        for alias in data['aliases']:
            self.aliases.append(Alias(
                type=alias['type'],
                name=alias['name'],
                desc=alias['description']
            ))
        for enum in data['enums']:
            self.enums.append(Enum(
                name=enum['name'],
                desc=enum['description'],
                values=[EnumValue(
                    name=value['name'],
                    value=value['value'],
                    desc=value['description']
                ) for value in enum['values']]
            ))
        for function in data['functions']:
            self.functions.append(Function(
                name=function['name'],
                desc=function['description'],
                params=[FunctionParam(
                    type=param['type'],
                    name=param['name']
                ) for param in function['params']
                ] if 'params' in function else [],
                ret_type=function['returnType']
            ))
        for callback in data['callbacks']:
            self.callbacks.add(callback['name'])
        return self
    
    @staticmethod
    def from_header(name: str, header: 'Header'):
        from c_bind.meta import schema
        self = Library(name)
        for type in header.types:
            if isinstance(type, schema.NamedFields):
                if type.is_opaque():
                    continue
                else:
                    self.structs.append(Struct(
                        name=type.name,
                        fields=[StructField(
                            type=field_type,
                            name=field_name
                        ) for field_name, field_type in type.fields.items()]
                    ))
            elif isinstance(type, schema.Enum):
                self.enums.append(Enum(
                    name=type.name,
                    values=[EnumValue(
                        name=value,
                        value=None
                    ) for value in type.values]
                ))
        for k, v in header.type_aliases.items():
            self.aliases.append(Alias(
                name=k,
                type=v
            ))

        for function in header.functions:
            self.functions.append(Function(
                name=function.name,
                params=[FunctionParam(
                    type=param,
                    name=f'_{i}'
                ) for i, param in enumerate(function.args)],
                ret_type=function.ret
            ))
        return self