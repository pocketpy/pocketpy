set -e

# if no $1 default arm64-v8a
if [ -z $1 ]; then
    $1=arm64-v8a
fi

mkdir -p build/android/$1
cd build/android/$1

cmake \
    -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=$1 \
    -DANDROID_PLATFORM=android-22 \
    ../../.. \
    -DPK_BUILD_SHARED_LIB=ON \
    -DCMAKE_BUILD_TYPE=Release \
    -DPK_BUILD_MODULE_LZ4=ON \
    -DPK_BUILD_MODULE_LIBHV=ON \
    -DPK_BUILD_CUTE_PNG=ON

cmake --build . --config Release
