mkdir build
cd build
cmake ..
cmake --build . --config Release
cp Release/main.exe ../
cp Release/pocketpy.dll ../