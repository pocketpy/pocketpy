mkdir -p output/windows/x86_64
mkdir build
cd build
cmake ..
cmake --build . --config Release
cp Release/main.exe ../output/windows/x86_64
cp Release/pocketpy.dll ../output/windows/x86_64
cp Release/main.exe ../
cp Release/pocketpy.dll ../