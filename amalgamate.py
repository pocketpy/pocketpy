import re
import shutil
import os
import sys
import time
from typing import List, Dict

assert os.system("python prebuild.py") == 0

ROOT = 'include/pocketpy'
PUBLIC_HEADERS = ['config.h', 'export.h', 'vmath.h', 'pocketpy.h']

COPYRIGHT = '''/*
 *  Copyright (c) 2025 blueloveTH
 *  Distributed Under The MIT License
 *  https://github.com/pocketpy/pocketpy
 */
 '''

def read_file(path):
	with open(path, 'rt', encoding='utf-8') as f:
		return f.read()

def write_file(path, content):
	with open(path, 'wt', encoding='utf-8', newline='\n') as f:
		f.write(content)
	
if os.path.exists('amalgamated'):
	shutil.rmtree('amalgamated')
	time.sleep(0.5)

os.mkdir('amalgamated')

class Header:
	path: str
	content: str		# header source
	dependencies: List[str]

	def __init__(self, path: str):
		self.path = path
		self.dependencies = []
		self.content = read_file(f'{ROOT}/{path}')

		# process raw content and get dependencies
		self.content = self.content.replace('#pragma once', '')
		def _replace(m):
			path = m.group(1)
			if path.startswith('xmacros/'):
				return read_file(f'{ROOT}/{path}') + '\n'
			if path in PUBLIC_HEADERS:
				return ''	# remove include
			if path != self.path:
				self.dependencies.append(path)
			return ''	# remove include
		
		self.content = re.sub(
			r'#include\s+"pocketpy/(.+)"\s*',
			_replace,
			self.content
		)

	def __repr__(self):
		return f'Header({self.path!r}, dependencies={self.dependencies})'
	
	def text(self):
		return f'// {self.path}\n{self.content}\n'


headers: Dict[str, Header] = {}

for entry in os.listdir(ROOT):
	if os.path.isdir(f'{ROOT}/{entry}'):
		if entry == 'xmacros' or entry in PUBLIC_HEADERS:
			continue
		files = os.listdir(f'{ROOT}/{entry}')
		for file in sorted(files):
			assert file.endswith('.h')
			if entry in PUBLIC_HEADERS:
				continue
			headers[f'{entry}/{file}'] = Header(f'{entry}/{file}')

def merge_c_files():
	c_files = [COPYRIGHT, '\n', '#include "pocketpy.h"', '\n']

	# merge internal headers
	internal_h = []
	while True:
		for h in headers.values():
			if not h.dependencies:
				break
		else:
			if headers:
				print(headers)
				raise RuntimeError("Circular dependencies detected")
			break
		# print(h.path)
		internal_h.append(h.text())
		del headers[h.path]
		for h2 in headers.values():
			h2.dependencies = [d for d in h2.dependencies if d != h.path]

	c_files.extend(internal_h)

	def _replace(m):
		path = m.group(1)
		if path.startswith('xmacros/'):
			return read_file(f'{ROOT}/{path}') + '\n'
		return ''	# remove include

	for root, _, files in os.walk('src/'):
		for file in files:
			if file.endswith('.c'):
				path = os.path.join(root, file)
				c_files.append(f'// {path}\n')
				content = read_file(path)
				content = re.sub(
					r'#include\s+"pocketpy/(.+)"\s*',
					_replace,
					content,
				)
				c_files.append(content)
				c_files.append('\n')
	return ''.join(c_files)

def merge_h_files():
	h_files = [COPYRIGHT, '#pragma once']

	def _replace(m):
		path = m.group(1)
		if path.startswith('xmacros/'):
			return read_file(f'{ROOT}/{path}') + '\n'
		return ''	# remove include
	
	for path in PUBLIC_HEADERS:
		content = read_file(f'{ROOT}/{path}')
		content = content.replace('#pragma once', '')
		content = re.sub(
					r'#include\s+"pocketpy/(.+)"\s*',
					_replace,
					content,
				)
		h_files.append(content)
	return '\n'.join(h_files)


write_file('amalgamated/pocketpy.c', merge_c_files())
write_file('amalgamated/pocketpy.h', merge_h_files())

shutil.copy("src2/main.c", "amalgamated/main.c")

if sys.platform in ['linux', 'darwin']:
	ok = os.system("gcc -o main amalgamated/pocketpy.c amalgamated/main.c -O1 --std=c11 -lm -ldl -lpthread")
	if ok == 0:
		print("Test build success!")

print("amalgamated/pocketpy.h")

shutil.copy("amalgamated/pocketpy.h", "plugins/flutter/pocketpy/src/pocketpy.h")
shutil.copy("amalgamated/pocketpy.c", "plugins/flutter/pocketpy/src/pocketpy.c")