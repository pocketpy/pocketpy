#pragma once

#define PK_REGION(name)	1

#define PK_ALWAYS_PASS_BY_POINTER(T) \
	T(const T&) = delete; \
	T& operator=(const T&) = delete; \
	T(T&&) = delete; \
	T& operator=(T&&) = delete;

#define PK_SLICE_LOOP(i, start, stop, step) for(int i=start; step>0?i<stop:i>stop; i+=step)

namespace pkpy{
    
// global constants
inline const char* PK_HEX_TABLE = "0123456789abcdef";

inline const char* kPlatformStrings[] = {
    "win32",        // 0
    "emscripten",   // 1
    "ios",          // 2
    "darwin",       // 3
    "android",      // 4
    "linux",        // 5
    "unknown"       // 6
};

} // namespace pkpy
