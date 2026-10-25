set -e

python amalgamate.py

rm -rf build
mkdir build
cd build

FLAGS="-DPK_BUILD_STATIC_LIB=ON \
    -DPK_ENABLE_OS=OFF \
    -DCMAKE_BUILD_TYPE=Release \
    "

cmake -G Xcode $FLAGS ..
cmake --build . --config Release

cd ../

python scripts/merge_built_libraries.py build
