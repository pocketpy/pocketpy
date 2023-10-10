if (Test-Path build) {
    Remove-Item -Recurse -Force build
}

New-Item -ItemType Directory -Path build
Push-Location build

cmake ..
cmake --build . --config Release

Copy-Item "Release\main.exe" -Destination ".." # Note: NTFS uses backslash (\) instead of slashes (*nix, /) 
Copy-Item "Release\pocketpy.dll" -Destination ".."
