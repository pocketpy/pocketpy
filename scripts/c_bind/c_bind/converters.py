from .writer import Writer
from .types import C_INT_TYPES, C_FLOAT_TYPES, C_BOOL_TYPES, C_STRING_TYPES, LINALG_TYPES

class Converter:
    def __init__(self, T: str):
        self.T = T
    def c2py(self, w: Writer, out: str, expr: str):
        raise NotImplementedError
    def py2c(self, w: Writer, out: str, expr: str):
        raise NotImplementedError
    @property
    def py_T(self) -> str:
        raise NotImplementedError
    
    def is_const(self):
        return self.T.startswith('const ') or '[' in self.T
    
class _SimpleConverter(Converter):
    def c2py(self, w: Writer, out: str, expr: str):
        w.write(f'py_new{self.py_T}({out}, {expr});')
    def py2c(self, w: Writer, out: str, expr: str):
        w.write(f'if(!py_check{self.py_T}({expr})) return false;')
        w.write(f'{out} = py_to{self.py_T}({expr});')
    
class IntConverter(_SimpleConverter):
    @property
    def py_T(self) -> str:
        return 'int'
    
class BoolConverter(_SimpleConverter):
    @property
    def py_T(self) -> str:
        return 'bool'
    
class StringConverter(_SimpleConverter):
    @property
    def py_T(self) -> str:
        return 'str'
    
class FloatConverter(Converter):
    def c2py(self, w: Writer, out: str, expr: str):
        w.write(f'py_newfloat({out}, {expr});')
    def py2c(self, w: Writer, out: str, expr: str):
        if self.T == 'float':
            w.write(f'if(!py_castfloat32({expr}, &{out})) return false;')
        else:
            w.write(f'if(!py_castfloat({expr}, &{out})) return false;')

    @property
    def py_T(self) -> str:
        return 'float'

class PointerConverter(Converter):
    def c2py(self, w: Writer, out: str, expr: str):
        w.write(f'py_newint({out}, (py_i64){expr});')
    def py2c(self, w: Writer, out: str, expr: str):
        w.write(f'if(!py_checkint({expr})) return false;')
        w.write(f'{out} = ({self.T})py_toint({expr});')

    @property
    def py_T(self) -> str:
        return 'intptr'

class StructConverter(Converter):
    def __init__(self, T: str, type_index: str | None):
        super().__init__(T)
        if type_index is None:
            type_index = f'tp_user_{T}'
        self.type_index = type_index
    def c2py(self, w: Writer, out: str, expr: str):
        w.write('do {')
        w.indent()
        w.write(f'{self.T}* ud = py_newobject({out}, {self.type_index}, 0, sizeof({self.T}));')
        w.write(f'*ud = {expr};')
        w.dedent()
        w.write('} while(0);')
    def py2c(self, w: Writer, out: str, expr: str):
        w.write('do {')
        w.indent()
        w.write(f'if(!py_checktype({expr}, {self.type_index})) return false;')
        w.write(f'{out} = *({self.T}*)py_touserdata({expr});')
        w.dedent()
        w.write('} while(0);')

    @property
    def py_T(self) -> str:
        return self.T
    
class EnumConverter(Converter):
    def __init__(self, T: str):
        super().__init__(T)
    def c2py(self, w: Writer, out: str, expr: str):
        w.write(f'py_newint({out}, (py_i64){expr});')
    def py2c(self, w: Writer, out: str, expr: str):
        w.write(f'if(!py_checkint({expr})) return false;')
        w.write(f'{out} = ({self.T})py_toint({expr});')

    @property
    def py_T(self) -> str:
        return 'int'
    
class VoidConverter(Converter):
    def c2py(self, w: Writer, out: str, expr: str):
        w.write(f'py_newnone({out});')
    def py2c(self, w: Writer, out: str, expr: str):
        # raise NotImplementedError
        w.write(f'? // VoidConverter.py2c is not implemented')
    
    @property
    def py_T(self) -> str:
        return 'None'
    
class BuiltinVectorConverter(Converter):
    def __init__(self, T: str, py_builtin_T: str):
        super().__init__(T)
        self.py_builtin_T = py_builtin_T

    def c2py(self, w: Writer, out: str, expr: str):
        w.write(f'py_new{self.py_builtin_T}({out}, *(c11_{self.py_T}*)(&{expr}));')

    def py2c(self, w: Writer, out: str, expr: str):
        w.write('do {')
        w.indent()
        w.write(f'if(!py_checktype({expr}, tp_{self.py_T})) return false;')
        w.write(f'c11_{self.py_T} tmp = py_to{self.py_builtin_T}({expr});')
        # w.write(f'memcpy(&{out}, &tmp, sizeof(c11_{self.py_T}));')
        w.write(f'{out} = *({self.T}*)(&tmp);')
        w.dedent()
        w.write('} while(0);')
    
    @property
    def py_T(self) -> str:
        return self.py_builtin_T


_CONVERTERS: dict[str, Converter] = {}

for t in C_INT_TYPES:
    _CONVERTERS[t] = IntConverter(t)
for t in C_FLOAT_TYPES:
    _CONVERTERS[t] = FloatConverter(t)
for t in C_BOOL_TYPES:
    _CONVERTERS[t] = BoolConverter(t)
for t in C_STRING_TYPES:
    _CONVERTERS[t] = StringConverter(t)
for t in LINALG_TYPES:
    _CONVERTERS[t] = BuiltinVectorConverter(f'c11_{t}', t)

_CONVERTERS['void'] = VoidConverter('void')
_CONVERTERS['c11_array2d'] = StructConverter('c11_array2d', 'tp_array2d')

def set_linalg_converter(T: str, py_T: str):
    assert py_T in LINALG_TYPES
    _CONVERTERS[T] = BuiltinVectorConverter(T, py_T)

def set_enum_converters(enums: list[str]):
    for T in enums:
        _CONVERTERS[T] = EnumConverter(T)

def get_converter(T: str) -> Converter:
    if T in _CONVERTERS:
        return _CONVERTERS[T]
    if T.endswith('*'):
        return PointerConverter(T)
    if '[' in T:
        return PointerConverter(T)
    cvt = _CONVERTERS.get(T)
    if cvt is None:
        return StructConverter(T, None)
