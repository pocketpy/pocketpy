if (Test-Path build) {
    Remove-Item -Recurse -Force build
}
mkdir build
cd build
cmake ..
cmake --build . --config Release
cp Release/main.exe ../
cp Release/pocketpy.dll ../