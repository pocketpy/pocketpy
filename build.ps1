mkdir -p output/windows/x86_64
cd dylib
xmake f -p windows -a x64
xmake
cp build/windows/x64/release/test.dll ../output/windows/x86_64
cd ..
mkdir -p output/windows/x86_64
mkdir build
cd build
cmake ..
cmake --build . --config Release
cp Release/main.exe ../output/windows/x86_64
cp Release/pocketpy.dll ../output/windows/x86_64
cp Release/main.exe ../
cp Release/pocketpy.dll ../