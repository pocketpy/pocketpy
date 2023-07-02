import os
import sys
import shutil

assert __name__ == "__main__"

os.system("python3 prebuild.py")

src_file_list = []
for file in os.listdir("src"):
    if file.endswith(".cpp"):
        src_file_list.append("src/" + file)

main_src_arg = " ".join(src_file_list+["src2/main.cpp"])

print(main_src_arg)

linux_common = " -Wfatal-errors --std=c++17 -O2 -Wall -fno-rtti -stdlib=libc++ -Iinclude/ "
linux_cmd = "clang++ -o pocketpy " + main_src_arg + linux_common

if "web" in sys.argv:
    os.system(r'''
    rm -rf web/lib/
mkdir -p web/lib/
em++ ''' + main_src_arg + '''-Iinclude/ -fno-rtti -fexceptions -O3 -sEXPORTED_FUNCTIONS=_pkpy_new_repl,_pkpy_repl_input,_pkpy_new_vm -sEXPORTED_RUNTIME_METHODS=ccall -o web/lib/pocketpy.js
''')
else:
    os.system(linux_cmd)