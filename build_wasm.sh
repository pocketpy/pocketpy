rm -rf web/lib/
mkdir -p web/lib/
em++ src/main.cpp -fno-rtti -fexceptions -sEXIT_RUNTIME -O3 -sEXPORTED_FUNCTIONS=_repl_input,_repl_start -sEXPORTED_RUNTIME_METHODS=ccall -o web/lib/pocketpy.js