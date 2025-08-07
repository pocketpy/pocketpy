#pragma once

// clang-format off

#if defined(_WIN32) || defined(_WIN64)
    #ifdef PY_DYNAMIC_MODULE
        #define PK_API __declspec(dllimport)
    #else
        #define PK_API __declspec(dllexport)
    #endif
    #define PK_EXPORT __declspec(dllexport)
    #define PY_SYS_PLATFORM     0
    #define PY_SYS_PLATFORM_STRING "win32"
#elif __EMSCRIPTEN__
    #define PK_API
    #define PK_EXPORT
    #define PY_SYS_PLATFORM     1
    #define PY_SYS_PLATFORM_STRING "emscripten"
#elif __APPLE__
    #include <TargetConditionals.h>
    #if TARGET_IPHONE_SIMULATOR
        // iOS, tvOS, or watchOS Simulator
        #define PY_SYS_PLATFORM     2
        #define PY_SYS_PLATFORM_STRING "ios"
    #elif TARGET_OS_IPHONE
        // iOS, tvOS, or watchOS device
        #define PY_SYS_PLATFORM     2
        #define PY_SYS_PLATFORM_STRING "ios"
    #elif TARGET_OS_MAC
        #define PY_SYS_PLATFORM     3
        #define PY_SYS_PLATFORM_STRING "darwin"
    #else
    #   error "Unknown Apple platform"
    #endif
    #define PK_API __attribute__((visibility("default")))
    #define PK_EXPORT __attribute__((visibility("default")))
#elif __ANDROID__
    #define PK_API __attribute__((visibility("default")))
    #define PK_EXPORT __attribute__((visibility("default")))
    #define PY_SYS_PLATFORM     4
    #define PY_SYS_PLATFORM_STRING "android"
#elif __linux__
    #define PK_API __attribute__((visibility("default")))
    #define PK_EXPORT __attribute__((visibility("default")))
    #define PY_SYS_PLATFORM     5
    #define PY_SYS_PLATFORM_STRING "linux"
#else
    #define PK_API
    #define PY_SYS_PLATFORM     6
    #define PY_SYS_PLATFORM_STRING "unknown"
#endif

#if PY_SYS_PLATFORM == 0 || PY_SYS_PLATFORM == 3 || PY_SYS_PLATFORM == 5
    #define PK_IS_DESKTOP_PLATFORM 1
#else
    #define PK_IS_DESKTOP_PLATFORM 0
#endif

#if defined(__GNUC__) || defined(__clang__)
    #define PK_DEPRECATED __attribute__((deprecated))
#else
    #define PK_DEPRECATED
#endif

#ifdef NDEBUG
    #if defined(__GNUC__)
        #define PK_INLINE __attribute__((always_inline)) inline
    #elif defined(_MSC_VER)
        #define PK_INLINE __forceinline
    #else
        #define PK_INLINE inline
    #endif
#else
    #define PK_INLINE
#endif
