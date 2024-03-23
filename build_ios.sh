rm -rf build
mkdir build
cd build

FLAGS="-DCMAKE_TOOLCHAIN_FILE=~/ios-cmake/ios.toolchain.cmake -DPK_BUILD_STATIC_LIB=ON -DDEPLOYMENT_TARGET=13.0"

cmake -B os64 -G Xcode  -DPLATFORM=OS64 $FLAGS ..
cmake --build os64 --config Release

cmake -B simulator64 -G Xcode  -DPLATFORM=SIMULATOR64 $FLAGS ..
cmake --build simulator64 --config Release

xcodebuild -create-xcframework \
    -framework os64/Release-iphoneos/pocketpy.framework \
    -framework simulator64/Release-iphonesimulator/pocketpy.framework \
    -output pocketpy.xcframework


