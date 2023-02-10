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
        if not filename.endswith('.py'):
            continue
        filepath = os.path.join(path, filename)
        print("> " + filepath)
        code = test_file(filepath)
        if not code:
            has_error = True
            exit(1)
    return not has_error

if __name__ == '__main__':
    if len(sys.argv) > 1:
        d = sys.argv[1]
    else:
        d = 'tests/'
    print("Testing directory:", d)
    ok = test_dir(d)
    if ok:
        print("ALL TESTS PASSED")