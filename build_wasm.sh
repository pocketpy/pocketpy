rm -rf build/wasm/
mkdir -p build/wasm/
emcc src/main.cpp -fexceptions -o build/wasm/index.html
