from .writer import Writer
from .schema import Enum

def gen_enum(w: Writer, pyi_w: Writer, enum: Enum):
    for value in enum.values:
        w.write(f'ADD_ENUM({value.name});')

        if value.value is not None:
            pyi_w.write(f'{value.name}: int = {value.value}')
        else:
            pyi_w.write(f'{value.name}: int')
        