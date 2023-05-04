import os
import sys

assert __name__ == "__main__"

os.system("python3 preprocess.py")

def DONE(code=0):
    exit(code)

linux_common = "-Wfatal-errors --std=c++17 -O2 -Wall -Wno-sign-compare -Wno-unused-variable -fno-rtti -stdlib=libc++"
linux_cmd = "clang++ -o pocketpy src/main.cpp " + linux_common
linux_lib_cmd = "clang++ -fPIC -shared -o pocketpy.so src/tmp.cpp " + linux_common


def lib_pre_build():
    with open("src/tmp.cpp", "w", encoding='utf-8') as f:
        f.write('#include "pocketpy.h"')

def lib_post_build():
    os.remove("src/tmp.cpp")

windows_common = "CL -std:c++17 /utf-8 -GR- -EHsc -O2"
windows_cmd = windows_common + " -Fe:pocketpy src/main.cpp"
windows_lib_cmd = windows_common + " -LD -Fe:pocketpy src/tmp.cpp"

if sys.argv.__len__() == 1:
    os.system(linux_cmd)
    DONE()

if "windows" in sys.argv:
    if "-lib" in sys.argv:
        lib_pre_build()
        os.system(windows_lib_cmd)
        lib_post_build()
    else:
        os.system(windows_cmd)
    DONE()

if "linux" in sys.argv:
    if "-lib" in sys.argv:
        lib_pre_build()
        os.system(linux_lib_cmd)
        lib_post_build()
    else:
        os.system(linux_cmd)
    DONE()

if "web" in sys.argv:
    os.system(r'''
rm -rf web/lib/
mkdir -p web/lib/
em++ src/main.cpp -fno-rtti -fexceptions -O3 -sEXPORTED_FUNCTIONS=_pkpy_delete,_pkpy_new_repl,_pkpy_repl_input,_pkpy_new_vm,_pkpy_vm_add_module,_pkpy_vm_eval,_pkpy_vm_exec,_pkpy_vm_get_global -sEXPORTED_RUNTIME_METHODS=ccall -o web/lib/pocketpy.js
''')
    DONE()

