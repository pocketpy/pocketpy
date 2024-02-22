import os
import sys
import shutil

assert os.system("python prebuild.py") == 0

if not os.path.exists("build"):
    os.mkdir("build")

os.chdir("build")

code = os.system("cmake .. -DPK_USE_CJSON=ON -DPK_ENABLE_OS=ON -DCMAKE_BUILD_TYPE=Release")
assert code == 0
code = os.system("cmake --build . --config Release")
assert code == 0

if sys.platform == "win32":
    shutil.copy("Release/main.exe", "../main.exe")
    shutil.copy("Release/pocketpy.dll", "../pocketpy.dll")
elif sys.platform == "darwin":
    shutil.copy("main", "../main")
    shutil.copy("libpocketpy.dylib", "../libpocketpy.dylib")
else:
    shutil.copy("main", "../main")
    shutil.copy("libpocketpy.so", "../libpocketpy.so")
