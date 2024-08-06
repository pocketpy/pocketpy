set -e

python prebuild.py

rm -rf web/lib
mkdir web/lib

SRC=$(find src/ -name "*.c")

emcc $SRC -Iinclude/ -s -Os -sEXPORTED_FUNCTIONS=_py_initialize,_py_finalize,_py_exec,_py_replinput -sEXPORTED_RUNTIME_METHODS=ccall -o web/lib/pocketpy.js
