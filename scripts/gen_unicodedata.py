import unicodedata
from tqdm import trange
from typing import Literal

info = []

for i in trange(0x110000):
    char = chr(i)
    category = unicodedata.category(char)
    east_asian_width = unicodedata.east_asian_width(char)
    info.append((i, category, east_asian_width))

def merge(index: Literal[1, 2], filter):
    # index = 1, category
    # index = 2, east_asian_width
    result: list[tuple[int, int, str]] = []
    last_value = None
    last_start = None
    for i in range(len(info)):
        value = info[i][index]
        if value != last_value:
            if last_value is not None:
                result.append((last_start, i - 1, last_value))
            last_value = value
            last_start = i
    if last_value is not None:
        result.append((last_start, len(info) - 1, last_value))
    return [x for x in result if filter(x[2])]

df_category = merge(1, lambda x: x == 'Lo')
df_east_asian_width = merge(2, lambda x: x != 'N')

def to_c11(ranges, name, with_value=True):
    with open(f'{name}.c', 'wt', encoding='utf-8', newline='\n') as f:
        f.write(f'const static c11_u32_range {name}[] = {{\n')
        for start, end, value in ranges:
            if with_value:
                f.write(f'    {{ {start}, {end}, "{value}\\0" }},\n')
            else:
                f.write(f'    {{ {start}, {end} }},\n')
        f.write(f'}};\n')

to_c11(df_category, 'kLoRanges', with_value=False)
to_c11(df_east_asian_width, 'kEastAsianWidthRanges', with_value=True)


