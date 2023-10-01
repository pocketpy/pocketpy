#pragma once

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    //define something for Windows (32-bit and 64-bit, this part is common)
    #define PK_EXPORT __declspec(dllexport)
    #ifdef PK_USE_DYLIB
        #define PK_SUPPORT_DYLIB    1
    #else
        #define PK_SUPPORT_DYLIB    0
    #endif
    #define PK_SYS_PLATFORM     "win32"
#elif __EMSCRIPTEN__
    #include <emscripten.h>
    #define PK_EXPORT EMSCRIPTEN_KEEPALIVE
    #define PK_SUPPORT_DYLIB    0
    #define PK_SYS_PLATFORM     "emscripten"
#elif __APPLE__
    #include <TargetConditionals.h>
    #if TARGET_IPHONE_SIMULATOR
        // iOS, tvOS, or watchOS Simulator
        #define PK_SYS_PLATFORM     "ios"
        #define PK_SUPPORT_DYLIB    4
    #elif TARGET_OS_IPHONE
        // iOS, tvOS, or watchOS device
        #define PK_SYS_PLATFORM     "ios"
        #define PK_SUPPORT_DYLIB    4
    #elif TARGET_OS_MAC
        #define PK_SYS_PLATFORM     "darwin"
        #ifdef PK_USE_DYLIB
            #include <dlfcn.h>
            #define PK_SUPPORT_DYLIB    2
        #else
            #define PK_SUPPORT_DYLIB    0
        #endif
    #else
    #   error "Unknown Apple platform"
    #endif
    #define PK_EXPORT __attribute__((visibility("default")))
#elif __ANDROID__
    #ifdef PK_USE_DYLIB
        #include <dlfcn.h>
        #define PK_SUPPORT_DYLIB    3
    #else
        #define PK_SUPPORT_DYLIB    0
    #endif
    #define PK_EXPORT __attribute__((visibility("default")))
    #define PK_SYS_PLATFORM     "android"
#elif __linux__
    #ifdef PK_USE_DYLIB
        #include <dlfcn.h>
        #define PK_SUPPORT_DYLIB    2
    #else
        #define PK_SUPPORT_DYLIB    0
    #endif
    #define PK_EXPORT __attribute__((visibility("default")))
    #define PK_SYS_PLATFORM     "linux"
#else
    #define PK_EXPORT
    #define PK_SUPPORT_DYLIB    0
    #define PK_SYS_PLATFORM     "unknown"
#endif