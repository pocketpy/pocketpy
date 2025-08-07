import os
import sys
import shutil

assert os.system("python prebuild.py") == 0

if not os.path.exists("build"):
    os.mkdir("build")

# python cmake_build.py [Debug|Release|RelWithDebInfo] ...

if len(sys.argv) > 1:
    config = sys.argv[1]
else:
    config = 'Release'

extra_flags = " ".join(sys.argv[2:])

assert config in ['Debug', 'Release', 'RelWithDebInfo']

os.chdir("build")

code = os.system(f"cmake .. -DPK_ENABLE_MIMALLOC=ON -DPK_ENABLE_DETERMINISM=ON -DCMAKE_BUILD_TYPE={config} {extra_flags}")
assert code == 0
code = os.system(f"cmake --build . --config {config} -j 4")
assert code == 0

if sys.platform == "win32":
    shutil.copy(f"{config}/main.exe", "../main.exe")
    dll_path = f"{config}/pocketpy.dll"
    if os.path.exists(dll_path):
        shutil.copy(dll_path, "../pocketpy.dll")
elif sys.platform == "darwin":
    shutil.copy("main", "../main")
    dll_path = "libpocketpy.dylib"
    if os.path.exists(dll_path):
        shutil.copy(dll_path, "../libpocketpy.dylib")
else:
    shutil.copy("main", "../main")
    dll_path = "libpocketpy.so"
    if os.path.exists(dll_path):
        shutil.copy(dll_path, "../libpocketpy.so")
