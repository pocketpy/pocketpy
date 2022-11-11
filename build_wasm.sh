rm -rf web/lib/
mkdir -p web/lib/
emcc src/main.cpp -fexceptions -sEXIT_RUNTIME -O2 -sEXPORTED_FUNCTIONS=_repl_input,_repl_start -sEXPORTED_RUNTIME_METHODS=ccall -o web/lib/pocketpy.js