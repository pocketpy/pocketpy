set -e

# Use python3 if available, otherwise fall back to python
PYTHON=$(command -v python3 >/dev/null 2>&1 && echo python3 || echo python)

$PYTHON prebuild.py

rm -rf web/lib
mkdir web/lib

SRC=$(find src/ -name "*.c")

emcc $SRC -Iinclude/ -s -Os \
    -sEXPORTED_FUNCTIONS=_py_initialize,_py_exec,_py_finalize,_py_printexc,_py_clearexc \
    -sEXPORTED_RUNTIME_METHODS=ccall \
    -sALLOW_MEMORY_GROWTH=1 \
    -o web/lib/pocketpy.js
