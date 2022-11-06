import os
import re

with open("src/opcodes.h", "rt", encoding='utf-8') as f:
    text = f.read()

# opcodes = re.findall(r"OP_(\w+)", text)

# print('\n'.join([f"OPCODE({o})" + o for o in opcodes]))

text = re.sub(r"OP_(\w+)", lambda m: f"OPCODE({m.group(1)})", text)

print(text.replace(',', ''))