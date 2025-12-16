from dataclasses import dataclass
from pycparser import c_ast

@dataclass
class StructField:
    type: str
    name: str
    desc: str | None = None

@dataclass
class EnumValue:
    name: str
    value: int | None
    desc: str | None = None

@dataclass
class Struct:
    name: str | None = None
    typedef_name: str | None = None
    desc: str | None = None
    fields: list[StructField] | None = None

    @property
    def code_name(self):
        if self.typedef_name:
            return self.typedef_name
        assert self.name is not None
        return f'struct {self.name}'
    
    @property
    def identifier(self):
        if self.name:
            return self.name
        assert self.typedef_name is not None
        return self.typedef_name

@dataclass
class Enum:
    name: str
    values: list[EnumValue]
    desc: str | None = None

@dataclass
class FunctionParam:
    type: str
    name: str

@dataclass
class Function:
    name: str
    params: list[FunctionParam]
    ret_type: str
    desc: str | None = None

    def signature(self) -> str:
        return f'{self.ret_type} {self.name}({", ".join([f"{param.type} {param.name}" for param in self.params])})'

