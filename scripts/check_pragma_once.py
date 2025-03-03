import os
import sys

def check_pragma_once_in_dir(path):
    for root, dirs, files in os.walk(path):
        if 'include/pocketpy/xmacros' in root or 'include/pybind11' in root:
            continue
        for file in files:
            if file.endswith(".c") or file.endswith(".h"):
                with open(os.path.join(root, file), "r", encoding='utf-8') as f:
                    print(f"==> {os.path.join(root, file)}")
                    text = f.read()
                    assert text.count("#pragma once") == 1, "#pragma once should appear exactly once"

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python scripts/check_pragma_once.py <path>")
        sys.exit(1)

    check_pragma_once_in_dir(sys.argv[1])