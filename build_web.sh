set -e

python prebuild.py

rm -rf web/lib
mkdir web/lib

SRC=$(find src/ -name "*.c")

emcc $SRC -Iinclude/ -s -Os \
    -sEXPORTED_FUNCTIONS=_py_initialize,_py_exec,_py_finalize,_py_printexc,_py_clearexc \
    -sEXPORTED_RUNTIME_METHODS="['ccall','cwrap','_emscripten_stack_get_current']"
    -sALLOW_MEMORY_GROWTH=1 \
    -o web/lib/pocketpy.js
