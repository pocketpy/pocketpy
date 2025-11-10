set -e

python amalgamate.py

rm -rf build
mkdir build
cd build

FLAGS="-DCMAKE_TOOLCHAIN_FILE=3rd/ios.toolchain.cmake \
    -DDEPLOYMENT_TARGET=13.0 \
    -DPK_BUILD_STATIC_LIB=ON \
    -DPK_ENABLE_OS=OFF \
    -DPK_ENABLE_DETERMINISM=ON \
    -DPK_BUILD_MODULE_LZ4=ON \
    -DPK_BUILD_MODULE_CUTE_PNG=ON \
    -DPK_BUILD_MODULE_MSGPACK=ON \
    -DCMAKE_BUILD_TYPE=Release \
    "

cmake -B os64 -G Xcode $FLAGS -DPLATFORM=OS64 ..
cmake --build os64 --config Release

cmake -B simulatorarm64 -G Xcode $FLAGS -DPLATFORM=SIMULATORARM64 ..
cmake --build simulatorarm64 --config Release

cd ../

HEADERS="amalgamated/pocketpy.h"

python scripts/merge_built_libraries.py build/os64
python scripts/merge_built_libraries.py build/simulatorarm64

xcodebuild -create-xcframework \
    -library build/os64/libpocketpy.a -headers $HEADERS \
    -library build/simulatorarm64/libpocketpy.a -headers $HEADERS \
    -output build/pocketpy.xcframework
