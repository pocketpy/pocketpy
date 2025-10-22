set -e

python amalgamate.py

rm -rf build
mkdir build
cd build

FLAGS="-DPK_BUILD_STATIC_LIB=ON \
    -DPK_ENABLE_OS=OFF \
    -DPK_ENABLE_DETERMINISM=ON \
    -DPK_BUILD_MODULE_LZ4=ON \
    -DPK_BUILD_MODULE_CUTE_PNG=ON \
    "

cmake -G Xcode $FLAGS ..
cmake --build . --config Release

cd ../

python scripts/merge_built_libraries.py build
