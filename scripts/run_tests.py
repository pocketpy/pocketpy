import os
import sys
import time

def test_file(filepath, cpython=False):
    if cpython:
        return os.system("python3 " + filepath) == 0
    if sys.platform == 'win32':
        return os.system("pocketpy.exe " + filepath) == 0
    else:
        return os.system("./pocketpy " + filepath) == 0

def test_dir(path):
    print("Testing directory:", path)
    for filename in sorted(os.listdir(path)):
        if not filename.endswith('.py'):
            continue
        filepath = os.path.join(path, filename)
        print("> " + filepath, flush=True)

        if path == 'benchmarks/':
            _0 = time.time()
            if not test_file(filepath, cpython=True): exit(1)
            _1 = time.time()
            if not test_file(filepath): exit(1)
            _2 = time.time()
            print(f'  cpython:  {_1 - _0:.6f}s (100%)')
            print(f'  pocketpy: {_2 - _1:.6f}s ({(_2 - _1) / (_1 - _0) * 100:.2f}%)')
        else:
            if not test_file(filepath):
                print('-' * 50)
                print("TEST FAILED! Press any key to continue...")
                input()


if len(sys.argv) == 2:
    assert 'benchmark' in sys.argv[1]
    d = 'benchmarks/'
else:
    d = 'tests/'
test_dir(d)
print("ALL TESTS PASSED")