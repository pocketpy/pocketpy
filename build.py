import os
import sys

assert __name__ == "__main__"

os.system("python3 preprocess.py")

def DONE(code=0):
    exit(code)

linux_common = "-Wfatal-errors --std=c++17 -O2 -Wall -Wno-sign-compare -Wno-unused-variable -fno-rtti -stdlib=libc++"
linux_cmd = "clang++ -o pocketpy src/main.cpp " + linux_common
linux_lib_cmd = "clang++ -fPIC -shared -o pocketpy.so src/tmp.cpp " + linux_common

lib_pre_cmd = r'''echo "#include \"pocketpy.h\"" > src/tmp.cpp'''
lib_post_cmd = r'''rm src/tmp.cpp'''

windows_common = "clang-cl.exe -std:c++17 -GR- -EHsc -O2 -Wno-deprecated-declarations"
windows_cmd = windows_common + " -Fe:pocketpy src/main.cpp"
windows_lib_cmd = windows_common + " -LD -Fe:pocketpy src/tmp.cpp"

if sys.argv.__len__() == 1:
    os.system(linux_cmd)
    DONE()

if "windows" in sys.argv:
    if "-lib" in sys.argv:
        os.system(lib_pre_cmd)
        os.system(windows_lib_cmd)
        os.system(lib_post_cmd)
    else:
        os.system(windows_cmd)
    DONE()

if "linux" in sys.argv:
    if "-lib" in sys.argv:
        os.system(lib_pre_cmd)
        os.system(linux_lib_cmd)
        os.system(lib_post_cmd)
    else:
        os.system(linux_cmd)
    DONE()

if "web" in sys.argv:
    os.system(r'''
rm -rf web/lib/
mkdir -p web/lib/
em++ src/main.cpp -fno-rtti -fexceptions -O3 -sEXPORTED_FUNCTIONS=_pkpy_delete,_pkpy_setup_callbacks,_pkpy_new_repl,_pkpy_repl_input,_pkpy_new_vm,_pkpy_vm_add_module,_pkpy_vm_bind,_pkpy_vm_eval,_pkpy_vm_exec,_pkpy_vm_get_global,_pkpy_vm_read_output -sEXPORTED_RUNTIME_METHODS=ccall -o web/lib/pocketpy.js
''')
    DONE()