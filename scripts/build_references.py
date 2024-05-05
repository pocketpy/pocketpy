import re

filepath = 'include/pocketpy/vm.h'

with open(filepath, 'r', encoding='utf-8') as f:
    lines = f.readlines()

REGION_PATTERN = re.compile(r'#if PK_REGION\("(.+)"\)')

current_region = None
output = []

def parse_line(line: str):
    output.append(line)

for line in lines:
    if current_region:
        if line.startswith('#endif'):
            current_region = None
            output.append('```\n\n')
        else:
            parse_line(line.strip(' '))
    else:
        m = REGION_PATTERN.match(line)
        if m:
            current_region = m.group(1)
            output.append(f'### {current_region}\n')
            output.append('```cpp\n')

with open('docs/references.md', 'w', encoding='utf-8') as f:
    f.write('''---
label: References
icon: code
order: 2
---
            
This page contains all useful methods of `VM` class.

''')
    content = ''.join(output)
    # replace {...} to  ; (multi-line match)
    content = re.sub(r'\{[^}]+?\}', r';', content, flags=re.DOTALL)
    f.write(content)
