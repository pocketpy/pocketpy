rm -rf build
mkdir build
cd build

FLAGS="-DCMAKE_TOOLCHAIN_FILE=3rd/ios.toolchain.cmake -DPK_BUILD_STATIC_LIB=ON -DDEPLOYMENT_TARGET=13.0"

cmake -B os64 -G Xcode $FLAGS -DPLATFORM=OS64 ..
cmake --build os64 --config Release

cmake -B simulatorarm64 -G Xcode $FLAGS -DPLATFORM=SIMULATORARM64 ..
cmake --build simulatorarm64 --config Release

xcodebuild -create-xcframework \
    -library os64/Release-iphoneos/libpocketpy.a -headers ../include \
    -library simulatorarm64/Release-iphonesimulator/libpocketpy.a -headers ../include \
    -output pocketpy.xcframework


