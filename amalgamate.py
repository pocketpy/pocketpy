import re
import shutil
import os
import sys
import time
from typing import List, Dict

# Run the prebuild script before starting the amalgamation process
assert os.system("python prebuild.py") == 0

# Define the root directory containing headers
ROOT = 'include/pocketpy'

# List of public headers that will be included in the final amalgamated header
PUBLIC_HEADERS = ['config.h', 'export.h', 'linalg.h', 'pocketpy.h']

# Copyright notice to be added at the beginning of amalgamated files
COPYRIGHT = '''/*
 *  Copyright (c) 2025 blueloveTH
 *  Distributed Under The MIT License
 *  https://github.com/pocketpy/pocketpy
 */
 '''

def read_file(path):
    """Read the content of a file with UTF-8 encoding."""
    with open(path, 'rt', encoding='utf-8') as f:
        return f.read()

def write_file(path, content):
    """Write content to a file with UTF-8 encoding and Unix line endings."""
    with open(path, 'wt', encoding='utf-8', newline='\n') as f:
        f.write(content)

# Remove existing amalgamated directory if it exists and create a new one
if os.path.exists('amalgamated'):
    shutil.rmtree('amalgamated')
    time.sleep(0.5)  # Wait briefly to ensure directory is fully removed
os.mkdir('amalgamated')

class Header:
    """
    Class to represent and process a header file.
    
    Attributes:
        path: Relative path to the header file
        content: Processed content of the header file
        dependencies: List of other headers this header depends on
    """
    path: str
    content: str
    dependencies: List[str]
    
    def __init__(self, path: str):
        """
        Initialize a Header object.
        
        Args:
            path: Relative path to the header file
        """
        self.path = path
        self.dependencies = []
        self.content = read_file(f'{ROOT}/{path}')
        
        # Process raw content and extract dependencies
        self.content = self.content.replace('#pragma once', '')  # Remove pragma once directives
        
        def _replace(m):
            """
            Process #include directives in the header.
            
            Returns:
                - For xmacros: the content of the included file
                - For public headers: empty string (removed)
                - For other headers: empty string and adds the header to dependencies
            """
            path = m.group(1)
            if path.startswith('xmacros/'):
                return read_file(f'{ROOT}/{path}') + '\n'  # Inline xmacros directly
            if path in PUBLIC_HEADERS:
                return ''  # Remove includes of public headers
            if path != self.path:
                self.dependencies.append(path)  # Add to dependencies
            return ''  # Remove include directive
        
        # Process all include directives
        self.content = re.sub(
            r'#include\s+"pocketpy/(.+)"\s*',
            _replace,
            self.content
        )
    
    def __repr__(self):
        """Return string representation of the Header object."""
        return f'Header({self.path!r}, dependencies={self.dependencies})'
    
    def text(self):
        """Return the processed header content with a comment indicating its source."""
        return f'// {self.path}\n{self.content}\n'

# Dictionary to store all header objects
headers: Dict[str, Header] = {}

# Process all header files in the ROOT directory
for entry in os.listdir(ROOT):
    if os.path.isdir(f'{ROOT}/{entry}'):
        if entry == 'xmacros' or entry in PUBLIC_HEADERS:
            continue  # Skip xmacros directory and public headers
        files = os.listdir(f'{ROOT}/{entry}')
        for file in sorted(files):
            assert file.endswith('.h')  # Ensure we're only processing header files
            if entry in PUBLIC_HEADERS:
                continue
            headers[f'{entry}/{file}'] = Header(f'{entry}/{file}')

def merge_c_files():
    """
    Merge all C source files into a single file.
    
    Uses a topological sort to ensure headers are included in the correct order
    based on their dependencies.
    
    Returns:
        String containing the amalgamated C source
    """
    c_files = [COPYRIGHT, '\n', '#include "pocketpy.h"', '\n']  # Start with copyright and main include
    
    # Merge internal headers in dependency order
    internal_h = []
    while True:
        # Find a header with no dependencies
        for h in headers.values():
            if not h.dependencies:
                break
        else:
            # If no headers without dependencies were found and headers still remain,
            # there must be a circular dependency
            if headers:
                print(headers)
                raise RuntimeError("Circular dependencies detected")
            break  # No headers left, we're done
        
        # Add this header to the amalgamated file
        internal_h.append(h.text())
        del headers[h.path]
        
        # Remove this header from the dependencies of other headers
        for h2 in headers.values():
            h2.dependencies = [d for d in h2.dependencies if d != h.path]
    
    c_files.extend(internal_h)
    
    def _replace(m):
        """Process include directives in C files."""
        path = m.group(1)
        if path.startswith('xmacros/'):
            return read_file(f'{ROOT}/{path}') + '\n'  # Inline xmacros
        return ''  # Remove other includes
    
    # Process all C source files
    for root, _, files in os.walk('src/'):
        for file in files:
            if file.endswith('.c'):
                path = os.path.join(root, file)
                c_files.append(f'// {path}\n')  # Add comment with original file path
                content = read_file(path)
                
                # Process include directives
                content = re.sub(
                    r'#include\s+"pocketpy/(.+)"\s*',
                    _replace,
                    content,
                )
                c_files.append(content)
                c_files.append('\n')
    
    return ''.join(c_files)

def merge_h_files():
    """
    Merge all public header files into a single header file.
    
    Returns:
        String containing the amalgamated header
    """
    h_files = [COPYRIGHT, '#pragma once']  # Start with copyright and pragma once
    
    def _replace(m):
        """Process include directives in header files."""
        path = m.group(1)
        if path.startswith('xmacros/'):
            return read_file(f'{ROOT}/{path}') + '\n'  # Inline xmacros
        return ''  # Remove other includes
    
    # Process all public headers
    for path in PUBLIC_HEADERS:
        content = read_file(f'{ROOT}/{path}')
        content = content.replace('#pragma once', '')  # Remove pragma once directives
        
        # Process include directives
        content = re.sub(
                    r'#include\s+"pocketpy/(.+)"\s*',
                    _replace,
                    content,
                )
        h_files.append(content)
    
    return '\n'.join(h_files)

# Generate amalgamated files
write_file('amalgamated/pocketpy.c', merge_c_files())
write_file('amalgamated/pocketpy.h', merge_h_files())

# Copy the main.c file to the amalgamated directory
shutil.copy("src2/main.c", "amalgamated/main.c")

# Test compile on Linux or macOS
if sys.platform in ['linux', 'darwin']:
    ok = os.system("gcc -o main amalgamated/pocketpy.c amalgamated/main.c -O1 --std=c11 -lm -ldl")
    if ok == 0:
        print("Test build success!")

print("amalgamated/pocketpy.h")

# Copy the amalgamated files to the Flutter plugin directory
shutil.copy("amalgamated/pocketpy.h", "plugins/flutter/pocketpy/src/pocketpy.h")
shutil.copy("amalgamated/pocketpy.c", "plugins/flutter/pocketpy/src/pocketpy.c")
