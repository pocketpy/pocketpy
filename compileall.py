import sys
import os
import subprocess

if len(sys.argv) != 4:
    print('Usage: python compileall.py <pocketpy_executable> <source_dir> <output_dir>')
    exit(1)

pkpy_exe = sys.argv[1]
source_dir = sys.argv[2]
output_dir = sys.argv[3]

def do_compile(src_path, dst_path):
    if not os.path.isfile(src_path):
        raise FileNotFoundError(f"Source file not found: {src_path}")
    result = subprocess.run([pkpy_exe, '--compile', src_path, dst_path])
    if result.returncode != 0:
        print(src_path)
        exit(1)

if os.path.isfile(source_dir):
    if output_dir.endswith('.py'):
        output_dir += 'c'
    do_compile(source_dir, output_dir)
    exit(0)

for root, _, files in os.walk(source_dir):
    for file in files:
        if not file.endswith('.py'):
            continue
        src_path = os.path.join(root, file)
        dst_path = os.path.join(
            output_dir,
            os.path.relpath(root, source_dir),
            file + 'c'
        )
        os.makedirs(os.path.dirname(dst_path), exist_ok=True)
        do_compile(src_path, dst_path)
