import os
import subprocess

def get_all_files(root: str):
    for path, _, files in os.walk(root):
        for file in files:
            fullpath = os.path.join(path, file)
            # ignore some files
            if fullpath.startswith('include/pybind11'):
                continue
            if file.startswith('_'):
                continue
            if not file.endswith('.cpp') and not file.endswith('.h') and not file.endswith('.hpp'):
                continue
            yield fullpath

if __name__ == '__main__':
    files = list(get_all_files('src'))
    files.extend(get_all_files('src2'))
    files.extend(get_all_files('include'))
    subprocess.run(['clang-format-15', '-i'] + files, check=True)
