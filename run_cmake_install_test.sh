rm -rf install
mkdir -p install/build
cd install/build
cmake ../../ -DCMAKE_INSTALL_PREFIX=../pocketpy -DPK_USE_CJSON=ON -DPK_ENABLE_OS=ON
cmake --build . --target install --config Release -j 8

if [ -e "../pocketpy/lib/libpocketpy.so" ]; then
    echo "Library installed successfully."
else
    echo "Library installation failed."
    exit 1
fi

if [ -e "../pocketpy/include/pocketpy.h" ]; then
    echo "Header files installed successfully."
else
    echo "Header files installation failed."
    exit 1
fi

if [ -e "../pocketpy/share/pocketpy/pocketpy-config.cmake" ]; then
    echo "Config file installed successfully."
else
    echo "Config file installation failed."
    exit 1
fi

echo "CMake install verified successfully."
