#pragma once

/*************** feature settings ***************/

// Whether to compile os-related modules or not
#ifndef PK_ENABLE_OS                // can be overridden by cmake
#define PK_ENABLE_OS                0
#endif

// Enable this if you are working with multi-threading (experimental)
// This triggers necessary locks to make the VM thread-safe
#ifndef PK_ENABLE_THREAD            // can be overridden by cmake
#define PK_ENABLE_THREAD            0
#endif

// Enable `line_profiler` module and `breakpoint()` function
#ifndef PK_ENABLE_PROFILER          // can be overridden by cmake
#define PK_ENABLE_PROFILER          0
#endif

// GC min threshold
#ifndef PK_GC_MIN_THRESHOLD         // can be overridden by cmake
#define PK_GC_MIN_THRESHOLD         32768
#endif

// Whether to use `std::function` to do bindings or not
// By default, functions to be binded must be a C function pointer without capture
// However, someone thinks it's not convenient.
// By setting this to 1, capturing lambdas can be binded,
// but it's slower and may cause severe "code bloat", also needs more time to compile.
#define PK_ENABLE_STD_FUNCTION      0

/*************** debug settings ***************/

// Enable this may help you find bugs
#define PK_DEBUG_EXTRA_CHECK        0

// Do not edit the following settings unless you know what you are doing
#define PK_DEBUG_NO_BUILTINS        0
#define PK_DEBUG_DIS_EXEC           0
#define PK_DEBUG_CEVAL_STEP         0
#define PK_DEBUG_MEMORY_POOL        0
#define PK_DEBUG_NO_MEMORY_POOL     0
#define PK_DEBUG_NO_AUTO_GC         0
#define PK_DEBUG_GC_STATS           0

#ifndef PK_DEBUG_PRECOMPILED_EXEC
#define PK_DEBUG_PRECOMPILED_EXEC   0
#endif

/*************** internal settings ***************/

// This is the maximum size of the value stack in void* units
// The actual size in bytes equals `sizeof(void*) * PK_VM_STACK_SIZE`
#define PK_VM_STACK_SIZE            32768

// This is the maximum number of local variables in a function
// (not recommended to change this / it should be less than 200)
#define PK_MAX_CO_VARNAMES          64

// Hash table load factor (smaller ones mean less collision but more memory)
// For class instance
#define PK_INST_ATTR_LOAD_FACTOR    0.67
// For class itself
#define PK_TYPE_ATTR_LOAD_FACTOR    0.5

#ifdef _WIN32
    #define PK_PLATFORM_SEP '\\'
#else
    #define PK_PLATFORM_SEP '/'
#endif

#ifdef _MSC_VER
#pragma warning (disable:4267)
#pragma warning (disable:4100)
#pragma warning (disable:4244)
#pragma warning (disable:4996)
#endif

#ifdef _MSC_VER
#define PK_UNREACHABLE()			__assume(0);
#else
#define PK_UNREACHABLE()			__builtin_unreachable();
#endif
