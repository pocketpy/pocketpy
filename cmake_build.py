import os
import sys
import shutil

if not os.path.exists("build"):
    os.mkdir("build")

os.chdir("build")

os.system(r"""
cmake .. -DPK_USE_CJSON=ON -DPK_USE_BOX2D=ON
cmake --build . --config Release
""")

if sys.platform == "win32":
    shutil.copy("Release/main.exe", "../main.exe")
    shutil.copy("Release/pocketpy.dll", "../pocketpy.dll")
elif sys.platform == "darwin":
    shutil.copy("main", "../main")
    shutil.copy("libpocketpy.dylib", "../libpocketpy.dylib")
else:
    shutil.copy("main", "../main")
    shutil.copy("libpocketpy.so", "../libpocketpy.so")
