import os

os.system("python3 prebuild.py")

with open("include/pocketpy/opcodes.h", "rt", encoding='utf-8') as f:
	OPCODES_TEXT = '\n' + f.read() + '\n'

pipeline = [
	["config.h", "common.h", "memory.h", "vector.h", "str.h", "tuplelist.h", "namedict.h", "error.h", "lexer.h"],
	["obj.h", "dict.h", "codeobject.h", "frame.h"],
	["gc.h", "vm.h", "ceval.h", "expr.h", "compiler.h", "repl.h"],
	["_generated.h", "cffi.h", "iter.h", "base64.h", "random.h", "re.h", "linalg.h", "easing.h", "io.h"],
	["export.h", "pocketpy.h"]
]

copied = set()
text = ""

import re
import shutil
import os
import sys
import time

if os.path.exists("amalgamated"):
	shutil.rmtree("amalgamated")
	time.sleep(0.5)
os.mkdir("amalgamated")

def remove_copied_include(text):
	text = text.replace("#pragma once", "")

	def _replace(m):
		key = m.group(1)
		if key.startswith("pocketpy/"):
			key = key[9:]
		if key == "user_config.h":
			return m.group(0)
		if key == "opcodes.h":
			return OPCODES_TEXT
		assert key in copied, f"include {key} not found"
		return ""

	text = re.sub(
		r'#include\s+"(.+)"\s*',
		_replace,
		text
	)
	return text

for seq in pipeline:
	for j in seq:
		print(j)
		with open("include/pocketpy/"+j, "rt", encoding='utf-8') as f:
			text += remove_copied_include(f.read()) + '\n'
			copied.add(j)
		j = j.replace(".h", ".cpp")
		if os.path.exists("src/"+j):
			with open("src/"+j, "rt", encoding='utf-8') as f:
				text += remove_copied_include(f.read()) + '\n'
				copied.add(j)

with open("amalgamated/pocketpy.h", "wt", encoding='utf-8') as f:
	final_text = \
r'''/*
 *  Copyright (c) 2023 blueloveTH
 *  Distributed Under The MIT License
 *  https://github.com/blueloveTH/pocketpy
 */

#ifndef POCKETPY_H
#define POCKETPY_H
''' + text + '\n#endif // POCKETPY_H'
	f.write(final_text)

shutil.copy("src2/main.cpp", "amalgamated/main.cpp")
with open("amalgamated/main.cpp", "rt", encoding='utf-8') as f:
	text = f.read()
text = text.replace('#include "pocketpy/pocketpy.h"', '#include "pocketpy.h"')
with open("amalgamated/main.cpp", "wt", encoding='utf-8') as f:
	f.write(text)

if sys.platform == 'linux':
	ok = os.system("clang++ -o pocketpy amalgamated/main.cpp --std=c++17 -stdlib=libc++")
	if ok == 0:
		print("Test build success!")
		os.remove("pocketpy")

print("amalgamated/pocketpy.h")

content = []
for i in ["include/pocketpy/export.h", "c_bindings/pocketpy_c.h", "c_bindings/pocketpy_c.cpp"]:
	with open(i, "rt", encoding='utf-8') as g:
		content.append(g.read())

with open("amalgamated/pocketpy.cpp", "wt", encoding='utf-8') as f:
	content = '\n\n'.join(content)
	content = content.replace('#include "export.h"', '')
	content = content.replace('#include "pocketpy_c.h"', '')
	f.write(content)


shutil.copy("amalgamated/pocketpy.h", "plugins/flutter/src/pocketpy.h")
shutil.copy("amalgamated/pocketpy.cpp", "plugins/flutter/src/pocketpy.cpp")

shutil.copy("amalgamated/pocketpy.h", "plugins/macos/pocketpy/pocketpy.h")
shutil.copy("amalgamated/pocketpy.cpp", "plugins/macos/pocketpy/pocketpy.cpp")

# unity plugin
unity_ios_root = 'plugins/unity/PocketPyUnityPlugin/Assets/PocketPython/Plugins/iOS'
if os.path.exists(unity_ios_root):
	shutil.copy("amalgamated/pocketpy.h", unity_ios_root)
	shutil.copy("amalgamated/pocketpy.cpp", unity_ios_root)


