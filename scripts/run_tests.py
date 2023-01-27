import os
import sys

def test_file(filepath):
    if sys.platform == 'win32':
        return os.system("pocketpy.exe " + filepath) == 0
    else:
        return os.system("./pocketpy " + filepath) == 0

def test_dir(path):
    has_error = False
    for filename in os.listdir(path):
        if filename.endswith('.py'):
            if(filename == '1.py'):
                continue
            filepath = os.path.join(path, filename)
            code = test_file(filepath)
            if not code:
                print("[x] " + filepath)
                has_error = True
            else:
                print("[v] " + filepath)
    return not has_error

if __name__ == '__main__':
    ok = test_dir('tests')
    if ok:
        print("ALL TESTS PASSED")