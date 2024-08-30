#pragma once
// clang-format off

#define PK_VERSION				"2.0.0"
#define PK_VERSION_MAJOR            2
#define PK_VERSION_MINOR            0
#define PK_VERSION_PATCH            0

/*************** feature settings ***************/

// Whether to compile os-related modules or not
#ifndef PK_ENABLE_OS                // can be overridden by cmake
#define PK_ENABLE_OS                1
#endif

// GC min threshold
#ifndef PK_GC_MIN_THRESHOLD         // can be overridden by cmake
#define PK_GC_MIN_THRESHOLD         16384
#endif

/*************** debug settings ***************/
// Do not edit the following settings unless you know what you are doing
#define PK_DEBUG_CEVAL_STEP         0
#define PK_DEBUG_MEMORY_POOL        0
#define PK_DEBUG_NO_AUTO_GC         0
#define PK_DEBUG_GC_STATS           0
#define PK_DEBUG_COMPILER           0

/*************** internal settings ***************/

// This is the maximum size of the value stack in py_TValue units
// The actual size in bytes equals `sizeof(py_TValue) * PK_VM_STACK_SIZE`
#define PK_VM_STACK_SIZE            16384

// This is the maximum number of local variables in a function
// (not recommended to change this)
#define PK_MAX_CO_VARNAMES          64

#ifdef _WIN32
    #define PK_PLATFORM_SEP '\\'
#else
    #define PK_PLATFORM_SEP '/'
#endif