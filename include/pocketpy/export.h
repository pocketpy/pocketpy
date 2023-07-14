#pragma once

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    //define something for Windows (32-bit and 64-bit, this part is common)
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif

    #ifndef NOMINMAX
    #define NOMINMAX
    #endif

    #include <Windows.h>

    #define PK_EXPORT __declspec(dllexport)
    #define PK_SUPPORT_DYLIB    1
#elif __EMSCRIPTEN__
    #include <emscripten.h>
    #define PK_EXPORT EMSCRIPTEN_KEEPALIVE
    #define PK_SUPPORT_DYLIB    0
#elif __APPLE__
    #include <TargetConditionals.h>
    #include <dlfcn.h>
    #define PK_SUPPORT_DYLIB    2
    #if TARGET_IPHONE_SIMULATOR
         // iOS, tvOS, or watchOS Simulator
    #elif TARGET_OS_MACCATALYST
         // Mac's Catalyst (ports iOS API into Mac, like UIKit).
    #elif TARGET_OS_IPHONE
        // iOS, tvOS, or watchOS device
    #elif TARGET_OS_MAC
        // Other kinds of Apple platforms
    #else
    #   error "Unknown Apple platform"
    #endif
    #define PK_EXPORT __attribute__((visibility("default")))
#elif __linux__
    #include <dlfcn.h>
    #define PK_SUPPORT_DYLIB    2
    #define PK_EXPORT __attribute__((visibility("default")))
#else
    #define PK_EXPORT
    #define PK_SUPPORT_DYLIB    0
#endif