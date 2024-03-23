rm -rf build
mkdir build
cd build

FLAGS="-DCMAKE_TOOLCHAIN_FILE=~/ios-cmake/ios.toolchain.cmake -DPK_BUILD_STATIC_LIB=ON -DDEPLOYMENT_TARGET=13.0"

cmake -B os64 -G Xcode $FLAGS -DPLATFORM=OS64 ..
cmake --build os64 --config Release

cmake -B simulator64 -G Xcode $FLAGS -DPLATFORM=SIMULATOR64 ..
cmake --build simulator64 --config Release

cmake -B simulatorarm64 -G Xcode $FLAGS -DPLATFORM=SIMULATORARM64 ..
cmake --build simulatorarm64 --config Release

xcodebuild -create-xcframework \
    -framework os64/Release-iphoneos/pocketpy.framework \
    -framework simulator64/Release-iphonesimulator/pocketpy.framework \
    -framework simulatorarm64/Release-iphonesimulator/pocketpy.framework \
    -output pocketpy.xcframework


