from dataclasses import dataclass

@dataclass
class StructField:
    type: str
    name: str
    desc: str = None

@dataclass
class EnumValue:
    name: str
    value: int | None
    desc: str = None

@dataclass
class Struct:
    name: str
    desc: str = None
    fields: list[StructField] = None

@dataclass
class Alias:
    type: str
    name: str
    desc: str = None

@dataclass
class Enum:
    name: str
    values: list[EnumValue]
    desc: str = None

@dataclass
class FunctionParam:
    type: str
    name: str

@dataclass
class Function:
    name: str
    params: list[FunctionParam]
    ret_type: str
    desc: str = None

    def signature(self) -> str:
        return f'{self.ret_type} {self.name}({", ".join([f"{param.type} {param.name}" for param in self.params])})'

