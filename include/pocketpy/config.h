#pragma once
// clang-format off

#define PK_VERSION				"2.1.0"
#define PK_VERSION_MAJOR            2
#define PK_VERSION_MINOR            1
#define PK_VERSION_PATCH            0

/*************** feature settings ***************/
// Whether to compile os-related modules or not
#ifndef PK_ENABLE_OS                // can be overridden by cmake
#define PK_ENABLE_OS                1
#endif

#ifndef PK_ENABLE_WATCHDOG          // can be overridden by cmake
#define PK_ENABLE_WATCHDOG          0                
#endif

// GC min threshold
#ifndef PK_GC_MIN_THRESHOLD         // can be overridden by cmake
    #define PK_GC_MIN_THRESHOLD     32768
#endif

// Memory allocation functions
#ifndef PK_MALLOC
#define PK_MALLOC(size)             malloc(size)
#define PK_REALLOC(ptr, size)       realloc(ptr, size)
#define PK_FREE(ptr)                free(ptr)
#endif

// This is the maximum size of the value stack in py_TValue units
// The actual size in bytes equals `sizeof(py_TValue) * PK_VM_STACK_SIZE`
#ifndef PK_VM_STACK_SIZE            // can be overridden by cmake
    #define PK_VM_STACK_SIZE        16384
#endif

// This is the maximum number of local variables in a function
// (not recommended to change this)
#ifndef PK_MAX_CO_VARNAMES          // can be overridden by cmake
#define PK_MAX_CO_VARNAMES          64
#endif

/*************** internal settings ***************/
// This is the maximum character length of a module path
#define PK_MAX_MODULE_PATH_LEN      63

// This is some math constants
#define PK_M_PI                     3.1415926535897932384
#define PK_M_E                      2.7182818284590452354
#define PK_M_DEG2RAD                0.017453292519943295
#define PK_M_RAD2DEG                57.29577951308232

#ifdef _WIN32
    #define PK_PLATFORM_SEP '\\'
#else
    #define PK_PLATFORM_SEP '/'
#endif
