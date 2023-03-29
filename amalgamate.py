import os

os.system("python3 preprocess.py")

with open("src/opcodes.h", "rt", encoding='utf-8') as f:
	OPCODES_TEXT = f.read()

pipeline = [
	["common.h", "memory.h", "str.h", "tuplelist.h", "namedict.h", "error.h"],
	["obj.h", "parser.h", "codeobject.h", "frame.h"],
	["gc.h", "vm.h", "ref.h", "ceval.h", "compiler.h", "repl.h"],
	["iter.h", "cffi.h", "io.h", "_generated.h", "pocketpy.h"]
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
	ok = os.system("g++ -o pocketpy amalgamated/main.cpp --std=c++17")
	if ok == 0:
		print("Test build success!")
		os.remove("pocketpy")

# plugins sync
shutil.copy("amalgamated/pocketpy.h", "plugins/flutter/src/pocketpy.h")
shutil.copy("amalgamated/pocketpy.h", "plugins/macos/pocketpy/pocketpy.h")

if os.path.exists("plugins/unity/PocketPyUnityPlugin"):
	unity_ios_header = 'plugins/unity/PocketPyUnityPlugin/Assets/PocketPy/Plugins/iOS/pocketpy.h'
	shutil.copy("amalgamated/pocketpy.h", unity_ios_header)

if os.path.exists("plugins/godot/godot-cpp/pocketpy"):
	shutil.copy("amalgamated/pocketpy.h", "plugins/godot/godot-cpp/pocketpy/src/pocketpy.h")

print("amalgamated/pocketpy.h")