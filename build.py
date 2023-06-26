import os
import sys
import shutil

assert __name__ == "__main__"

os.system("python3 preprocess.py")

def DONE(code=0):
    exit(code)

linux_common = "-Wfatal-errors --std=c++17 -O2 -Wall -fno-rtti -stdlib=libc++"
linux_cmd = "clang++ -o pocketpy src/main.cpp " + linux_common
linux_lib_cmd = "clang++ -fPIC -shared -o pocketpy.so src/tmp.cpp " + linux_common

class LibBuildEnv:
    def __enter__(self):
        shutil.copy("c_bindings/pocketpy_c.h", "src/")
        shutil.copy("c_bindings/pocketpy_c.cpp", "src/tmp.cpp")

    def __exit__(self, *args):
        if os.path.exists("src/pocketpy_c.h"):
            os.remove("src/pocketpy_c.h")
        if os.path.exists("src/tmp.cpp"):
            os.remove("src/tmp.cpp")

windows_common = "CL -std:c++17 /utf-8 -GR- -EHsc -O2"
windows_cmd = windows_common + " -Fe:pocketpy src/main.cpp"
windows_lib_cmd = windows_common + " -LD -Fe:pocketpy src/tmp.cpp"

if sys.argv.__len__() == 1:
    os.system(linux_cmd)
    DONE()

if "windows" in sys.argv:
    if "-lib" in sys.argv:
        with LibBuildEnv():
            os.system(windows_lib_cmd)
    else:
        os.system(windows_cmd)
    DONE()

if "linux" in sys.argv:
    if "-lib" in sys.argv:
        with LibBuildEnv():
            os.system(linux_lib_cmd)
    else:
        os.system(linux_cmd)
    DONE()

if "web" in sys.argv:
    os.system(r'''
rm -rf web/lib/
mkdir -p web/lib/
em++ src/main.cpp -fno-rtti -fexceptions -O3 -sEXPORTED_FUNCTIONS=_pkpy_new_repl,_pkpy_repl_input,_pkpy_new_vm -sEXPORTED_RUNTIME_METHODS=ccall -o web/lib/pocketpy.js
''')
    DONE()

print("invalid usage!!")
exit(2)