#pragma once

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    //define something for Windows (32-bit and 64-bit, this part is common)
    #define PK_EXPORT __declspec(dllexport)
    #define PK_SYS_PLATFORM     0
#elif __EMSCRIPTEN__
    #define PK_EXPORT
    #define PK_SYS_PLATFORM     1
#elif __APPLE__
    #include <TargetConditionals.h>
    #if TARGET_IPHONE_SIMULATOR
        // iOS, tvOS, or watchOS Simulator
        #define PK_SYS_PLATFORM     2
    #elif TARGET_OS_IPHONE
        // iOS, tvOS, or watchOS device
        #define PK_SYS_PLATFORM     2
    #elif TARGET_OS_MAC
        #define PK_SYS_PLATFORM     3
    #else
    #   error "Unknown Apple platform"
    #endif
    #define PK_EXPORT __attribute__((visibility("default")))
#elif __ANDROID__
    #define PK_EXPORT __attribute__((visibility("default")))
    #define PK_SYS_PLATFORM     4
#elif __linux__
    #define PK_EXPORT __attribute__((visibility("default")))
    #define PK_SYS_PLATFORM     5
#else
    #define PK_EXPORT
    #define PK_SYS_PLATFORM     6
#endif

#if PK_SYS_PLATFORM == 0 || PK_SYS_PLATFORM == 3 || PK_SYS_PLATFORM == 5
    #define PK_IS_DESKTOP_PLATFORM 1
#else
    #define PK_IS_DESKTOP_PLATFORM 0
#endif
