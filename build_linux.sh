echo '#include "pocketpy.h"' > src/tmp.cpp
clang++ -fPIC -shared -o pocketpy.so src/tmp.cpp --std=c++17 -O2 -Wall -Wno-sign-compare -Wno-unused-variable -fno-rtti
rm src/tmp.cpp