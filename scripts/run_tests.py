import os
import sys
import time
import subprocess


def test_file(filepath, cpython=False):
    if cpython:
        return os.system("python " + filepath) == 0
    if sys.platform == 'win32':
        code = os.system("main.exe " + filepath)
    else:
        code = os.system("./main " + filepath)
    if code != 0:
        print('Return code:', code)
    return code == 0

def test_dir(path):
    print("Testing directory:", path)
    for filename in sorted(os.listdir(path)):
        if not filename.endswith('.py'):
            continue
        filepath = os.path.join(path, filename)
        print("> " + filepath, flush=True)

        if path == 'benchmarks/':
            _0 = time.time()
            if not test_file(filepath, cpython=True):
                print('cpython run failed')
                continue
            _1 = time.time()
            if not test_file(filepath): exit(1)
            _2 = time.time()
            print(f'  cpython:  {_1 - _0:.6f}s (100%)')
            print(f'  pocketpy: {_2 - _1:.6f}s ({(_2 - _1) / (_1 - _0) * 100:.2f}%)')
        else:
            if not test_file(filepath):
                print('-' * 50)
                print("TEST FAILED!")
                exit(1)
                # print("TEST FAILED! Press any key to continue...")
                # input()

print('CPython:', str(sys.version).replace('\n', ''))
print('System:', '64-bit' if sys.maxsize > 2**32 else '32-bit')

def test_repl():
    print("[REPL Test Enabled]")
    if sys.platform in ['linux', 'darwin']:
        cmd = './main'
    else:
        cmd = None

    if cmd is not None:
        res = subprocess.run([cmd], encoding='utf-8', input=r'''
def add(a, b):
    return a + b

class A:
    def __init__(self, x):
        self.x = x
    def get(self):
        return self.x

print('ans_1:', add(1, 2))
print('ans_2:', A('abc').get())
exit()
''', capture_output=True, check=True)
        res.check_returncode()
        # assert 'ans_1: 3' in res.stdout, res.stdout
        if 'ans_1: 3' not in res.stdout:
            print(res.stdout)
            exit(1)
        # assert 'ans_2: abc' in res.stdout, res.stdout
        if 'ans_2: abc' not in res.stdout:
            print(res.stdout)
            exit(1)


if len(sys.argv) == 2:
    assert 'benchmark' in sys.argv[1]
    test_dir('benchmarks/')
else:
    test_dir('tests/')
    test_repl()


print("ALL TESTS PASSED")
