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
    -DANDROID_STL=c++_shared \
    ../../.. \
    -DPK_BUILD_SHARED_LIB=ON -DPK_USE_CJSON=ON \
    -DCMAKE_BUILD_TYPE=Release

cmake --build . --config Release
