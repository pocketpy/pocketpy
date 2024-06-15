python prebuild.py

rm -rf web/lib
mkdir web/lib

SRC_C=$(find src/ -name "*.c")
SRC_CPP=$(find src/ -name "*.cpp")
SRC="$SRC_C $SRC_CPP"

em++ $SRC -Iinclude/ -fexceptions -frtti -s -Os -sEXPORTED_FUNCTIONS=_pkpy_new_repl,_pkpy_repl_input,_pkpy_new_vm -sEXPORTED_RUNTIME_METHODS=ccall -o web/lib/pocketpy.js
