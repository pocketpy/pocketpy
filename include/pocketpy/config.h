#pragma once
// clang-format off

#define PK_VERSION				"2.2.1"
#define PK_VERSION_MAJOR            2
#define PK_VERSION_MINOR            2
#define PK_VERSION_PATCH            1

/*************** feature settings ***************/
#ifndef PK_ENABLE_OS                // can be overridden by cmake
#define PK_ENABLE_OS                1
#endif

#ifndef PK_ENABLE_THREADS           // must be enabled from cmake
#define PK_ENABLE_THREADS           0
#endif

#ifndef PK_ENABLE_DLL               // must be enabled from cmake
#define PK_ENABLE_DLL               0
#endif

#ifndef PK_ENABLE_DETERMINISM       // must be enabled from cmake
#define PK_ENABLE_DETERMINISM       0
#endif

#ifndef PK_ENABLE_WATCHDOG          // can be overridden by cmake
#define PK_ENABLE_WATCHDOG          0                
#endif

#ifndef PK_ENABLE_CUSTOM_SNAME      // can be overridden by cmake
#define PK_ENABLE_CUSTOM_SNAME      0                
#endif

// GC min threshold
#ifndef PK_GC_MIN_THRESHOLD         // can be overridden by cmake
    #define PK_GC_MIN_THRESHOLD     20000
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

// Hash table load factor (smaller ones mean less collision but more memory)
// For class instance
#define PK_INST_ATTR_LOAD_FACTOR    0.67f
// For class itself
#define PK_TYPE_ATTR_LOAD_FACTOR    0.5f

#ifdef _WIN32
    #define PK_PLATFORM_SEP '\\'
#else
    #define PK_PLATFORM_SEP '/'
#endif

#ifdef __cplusplus
    #ifndef restrict
        #define restrict
    #endif
#endif

#if PK_ENABLE_THREADS
    #define PK_THREAD_LOCAL _Thread_local
#else
    #define PK_THREAD_LOCAL
#endif
