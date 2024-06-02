import re
import shutil
import os
import sys
import time
from typing import List, Dict

assert os.system("python prebuild.py") == 0

with open("include/pocketpy/opcodes.h", "rt", encoding='utf-8') as f:
	OPCODES_TEXT = '\n' + f.read() + '\n'

class Header:
	path: str
	content: str		# header+source (if exists)
	dependencies: List[str]

	def __init__(self, path: str):
		self.path = path
		self.dependencies = []

		# get raw content
		with open(f'include/pocketpy/{path}', 'rt', encoding='utf-8') as f:
			self.content = f.read()
		src_path = path.replace('.hpp', '.cpp').replace('.h', '.cpp')
		if os.path.exists(f'src/{src_path}'):
			with open(f'src/{src_path}', 'rt', encoding='utf-8') as f:
				self.content += f'\n\n/* {src_path} */\n\n'
				self.content += f.read()

		# process raw content and get dependencies
		self.content = self.content.replace('#pragma once', '')
		def _replace(m):
			path = m.group(1)
			if path == 'opcodes.h':
				return OPCODES_TEXT
			if path != self.path:
				self.dependencies.append(path)
			return ''
		
		self.content = re.sub(
			r'#include\s+"pocketpy/(.+)"\s*',
			_replace,
			self.content
		)

	def __repr__(self):
		return f'Header({self.path!r}, dependencies={self.dependencies})'


headers: Dict[str, Header] = {}

for path in ['pocketpy.hpp', 'pocketpy_c.h']:
	headers[path] = Header(path)

directories = ['common', 'objects', 'interpreter', 'compiler', 'modules', 'tools']
for directory in directories:
	files = os.listdir(f'include/pocketpy/{directory}')
	for file in sorted(files):
		assert file.endswith('.h') or file.endswith('.hpp')
		headers[f'{directory}/{file}'] = Header(f'{directory}/{file}')

text = '''#pragma once

/*
 *  Copyright (c) 2024 blueloveTH
 *  Distributed Under The MIT License
 *  https://github.com/pocketpy/pocketpy
 */'''

while True:
	for h in headers.values():
		if not h.dependencies:
			break
	else:
		if headers:
			print(headers)
			raise Exception("Circular dependencies detected")
		break
	print(h.path)
	text += h.content
	del headers[h.path]
	for h2 in headers.values():
		h2.dependencies = [d for d in h2.dependencies if d != h.path]

if os.path.exists("amalgamated"):
	shutil.rmtree("amalgamated")
	time.sleep(0.5)
os.mkdir("amalgamated")

# use LF line endings instead of CRLF
with open("amalgamated/pocketpy.h", "wt", encoding='utf-8', newline='\n') as f:
	f.write(text)

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
