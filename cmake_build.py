import subprocess
import sys
import shutil
import os

subprocess.run([sys.executable, "prebuild.py"], check=True)

if not os.path.exists("build"):
    os.mkdir("build")

# python cmake_build.py [Debug|Release|RelWithDebInfo] ...

if len(sys.argv) > 1:
    config = sys.argv[1]
else:
    config = 'Release'

extra_flags = sys.argv[2:]

if config not in ['Debug', 'Release', 'RelWithDebInfo']:
    raise ValueError(f"Invalid config: {config!r}. Must be one of Debug, Release, RelWithDebInfo")

os.chdir("build")

subprocess.run(
    ["cmake", "..", "-DPK_ENABLE_MIMALLOC=ON", "-DPK_ENABLE_DETERMINISM=ON",
     f"-DCMAKE_BUILD_TYPE={config}"] + extra_flags,
    check=True,
)
subprocess.run(
    ["cmake", "--build", ".", "--config", config, "-j", "4"],
    check=True,
)

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
