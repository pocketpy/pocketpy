from pycparser import c_ast

class Pointer:
    def __init__(self, base: str, level: int):
        super().__init__(f'{base}' + '*' * level)
        self.base = base
        self.level = level

class NamedFields:
    def __init__(self, name: str, fields: dict[str, str] | None):
        self.name = name
        self.fields = fields

    def is_opaque(self):
        return self.fields is None
    
    def __repr__(self):
        cls = type(self).__name__
        return f"{cls}('{self.name}', {self.fields!r})"
    
class Struct(NamedFields): pass
class Union(NamedFields): pass
    
class Enum:
    def __init__(self, name: str):
        self.name = name
        self.values = [] # type: list[str]

    def __repr__(self):
        return f"Enum('{self.name}', values={self.values!r})"

class Function:
    def __init__(self, name: str, ret: str):
        self.name = name
        self.args = [] # type: list[str]
        self.ret = ret

    def __repr__(self):
        return f"Function('{self.name}', args={self.args!r}, ret={self.ret!r})"