with open("src/opcodes.h", "rt", encoding='utf-8') as f:
	OPCODES_TEXT = f.read()

pipeline = [
	["hash_table8.hpp", "common.h", "memory.h", "str.h", "safestl.h", "builtins.h", "error.h"],
	["obj.h", "iter.h", "parser.h", "ref.h", "codeobject.h"],
	["vm.h", "compiler.h", "repl.h"],
	["pocketpy.h"]
]

copied = set()

text = ""

import re
import shutil
import os
import time

if os.path.exists("amalgamated"):
	shutil.rmtree("amalgamated")
	if os.name == "nt":
		# On Windows, making a directory too soon after shutil.rmtree can fail
		# with PermissionError. Sleep for 1 second as a simple hedge against
		# this.
		# See https://stackoverflow.com/a/60181105/569183
		time.sleep(1)
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
os.system("g++ -o pocketpy amalgamated/main.cpp --std=c++17")
os.system("rm pocketpy")

os.system("cp amalgamated/pocketpy.h plugins/flutter/src/pocketpy.h")
os.system("cp amalgamated/pocketpy.h plugins/macos/pocketpy/pocketpy.h")

if os.path.exists("plugins/unity/PocketPyUnityPlugin"):
	unity_ios_header = 'plugins/unity/PocketPyUnityPlugin/Assets/PocketPy/Plugins/iOS/pocketpy.h'
	os.system(f'cp amalgamated/pocketpy.h "{unity_ios_header}"')

if os.path.exists("plugins/godot/godot-cpp/pocketpy"):
	os.system("cp amalgamated/pocketpy.h plugins/godot/godot-cpp/pocketpy/src/pocketpy.h")