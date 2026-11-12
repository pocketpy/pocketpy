import os
import sys
import time
import subprocess
import ctypes
from pathlib import Path
from tempfile import TemporaryDirectory

pkpy_exe = 'main.exe' if sys.platform == 'win32' else './main'

def test_file(filepath, cpython=False):
    if cpython:
        return os.system(f'"{sys.executable}" {filepath}') == 0
    code = os.system(pkpy_exe + ' ' + filepath)
    if code != 0:
        print('Return code:', code)
    return code == 0

def test_dir(path):
    print("Testing directory:", path)
    for filename in sorted(os.listdir(path)):
        if not filename.endswith('.py') and not filename.endswith('.pyc'):
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

def test_windows_monotonic():
    # Read the same raw counter as pocketpy regardless of Python's clock implementation.
    kernel32 = ctypes.WinDLL('kernel32', use_last_error=True)

    def read_counter(query):
        value = ctypes.c_longlong()
        if not query(ctypes.byref(value)):
            raise ctypes.WinError(ctypes.get_last_error())
        return value.value

    frequency = read_counter(kernel32.QueryPerformanceFrequency)
    with TemporaryDirectory() as directory:
        script = Path(directory) / 'monotonic.py'
        script.write_text('import time\nprint(time.monotonic_ns())\n', encoding='utf-8')
        before = read_counter(kernel32.QueryPerformanceCounter)
        actual = int(subprocess.check_output([pkpy_exe, str(script)]))
        after = read_counter(kernel32.QueryPerformanceCounter)
    before_ns = before * 1000000000 // frequency
    after_ns = after * 1000000000 // frequency
    assert before_ns - 1 <= actual <= after_ns + 1, (before_ns, actual, after_ns)

code = os.system(f'"{sys.executable}" compileall.py {pkpy_exe} tests tmp/tests')
assert code == 0

if len(sys.argv) == 2:
    assert 'benchmark' in sys.argv[1]
    test_dir('benchmarks/')
else:
    test_dir('tests/')
    test_repl()
    if sys.platform == 'win32':
        test_windows_monotonic()

    if os.path.exists('tmp/tests'):
        print('-' * 50)
        time.sleep(3)
        test_dir('tmp/tests/')
    


print("ALL TESTS PASSED")
