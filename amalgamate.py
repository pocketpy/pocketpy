import os

os.system("python3 preprocess.py")

with open("src/opcodes.h", "rt", encoding='utf-8') as f:
	OPCODES_TEXT = f.read()

pipeline = [
	["common.h", "memory.h", "vector.h", "str.h", "tuplelist.h", "namedict.h", "error.h", "lexer.h"],
	["obj.h", "codeobject.h", "frame.h"],
	["gc.h", "vm.h", "ceval.h", "expr.h", "compiler.h", "repl.h"],
	["_generated.h", "iter.h", "base64.h", "cffi.h", "linalg.h", "easing.h", "requests.h", "io.h", "pocketpy.h"]
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
	time.sleep(0.6)
os.mkdir("amalgamated")

def remove_copied_include(text):
	text = text.replace("#pragma once", "")
	text = re.sub(
		r'#include\s+"(.+)"\s*',
		lambda m: "" if m.group(1) in copied else m.group(0),
		text
	)
	text = text.replace('#include "opcodes.h"', OPCODES_TEXT)
	return text

for seq in pipeline:
	for j in seq:
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

shutil.copy("src/main.cpp", "amalgamated/main.cpp")

if sys.platform == 'linux':
	ok = os.system("clang++ -o pocketpy amalgamated/main.cpp --std=c++17 -stdlib=libc++")
	if ok == 0:
		print("Test build success!")
		os.remove("pocketpy")

print("amalgamated/pocketpy.h")

# my custom things...
if os.path.exists("/mnt/e/PainterEngine/project/pocketpy.h"):
	shutil.copy("amalgamated/pocketpy.h", "/mnt/e/PainterEngine/project/pocketpy.h")
	shutil.copy("src/easing.pyi", "/mnt/e/PainterEngine/game/easing.pyi")
	shutil.copy("src/linalg.pyi", "/mnt/e/PainterEngine/game/linalg.pyi")


