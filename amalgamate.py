import os

assert os.system("python prebuild.py") == 0

with open("include/pocketpy/opcodes.h", "rt", encoding='utf-8') as f:
	OPCODES_TEXT = '\n' + f.read() + '\n'

pipeline = [
	["config.h", "export.h", "_generated.h", "common.h", "memory.h", "vector.h", "str.h", "tuplelist.h", "namedict.h", "error.h"],
	["obj.h", "dict.h", "codeobject.h", "frame.h", "profiler.h"],
	["gc.h", "vm.h", "ceval.h", "lexer.h", "expr.h", "compiler.h", "repl.h"],
	["cffi.h", "bindings.h", "iter.h", "base64.h", "csv.h", "collections.h", "array2d.h", "dataclasses.h", "random.h", "linalg.h", "easing.h", "io.h", "modules.h"],
	["pocketpy.h", "pocketpy_c.h"]
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
		if key in ["user_config.h", "cJSONw.hpp"]:
			return m.group(0)
		if "opcodes.h" in key:
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

# use LF line endings instead of CRLF
with open("amalgamated/pocketpy.h", "wt", encoding='utf-8', newline='\n') as f:
	final_text = \
r'''/*
 *  Copyright (c) 2024 blueloveTH
 *  Distributed Under The MIT License
 *  https://github.com/pocketpy/pocketpy
 */

#ifndef POCKETPY_H
#define POCKETPY_H
''' + text + '\n#endif // POCKETPY_H'
	f.write(final_text)

shutil.copy("src2/main.cpp", "amalgamated/main.cpp")
with open("amalgamated/main.cpp", "rt", encoding='utf-8') as f:
	text = f.read()
text = text.replace('#include "pocketpy/pocketpy.h"', '#include "pocketpy.h"')
with open("amalgamated/main.cpp", "wt", encoding='utf-8', newline='\n') as f:
	f.write(text)

if sys.platform in ['linux', 'darwin']:
	ok = os.system("clang++ -o main amalgamated/main.cpp -O1 --std=c++17 -frtti -stdlib=libc++")
	if ok == 0:
		print("Test build success!")

print("amalgamated/pocketpy.h")

def sync(path):
	shutil.copy("amalgamated/pocketpy.h", os.path.join(path, "pocketpy.h"))
	with open(os.path.join(path, "pocketpy.cpp"), "wt", encoding='utf-8', newline='\n') as f:
		f.write("#include \"pocketpy.h\"\n")

sync("plugins/macos/pocketpy")
