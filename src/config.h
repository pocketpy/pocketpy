#pragma once

#ifdef PK_USER_CONFIG_H

#include "user_config.h"

#else

/*************** feature settings ***************/

// Whether to compile os-related modules
#define PK_ENABLE_OS                1
// Enable this if you are working with multi-threading (experimental)
// This triggers necessary locks to make the VM thread-safe
#define PK_ENABLE_THREAD            0

// Whether to use `std::function` to do bindings
// By default, functions to be binded must be a C function pointer without capture
// However, someone thinks it's not convenient.
// By setting this to 1, capturing lambdas can be binded,
// but it's slower and may cause severe "code bloat", also needs more time to compile.
#define PK_ENABLE_STD_FUNCTION      0

/*************** internal settings ***************/

// This is the maximum size of the value stack in void* units
// The actual size in bytes equals `sizeof(void*) * PK_VM_STACK_SIZE`
#define PK_VM_STACK_SIZE            32768

// This is the maximum number of arguments in a function declaration
// including positional arguments, keyword-only arguments, and varargs
// (not recommended to change this)
#define PK_MAX_CO_VARNAMES          255

// Hash table load factor (smaller ones mean less collision but more memory)
// For class instance
inline const float kInstAttrLoadFactor = 0.67f;
// For class itself
inline const float kTypeAttrLoadFactor = 0.5f;

#ifdef _WIN32
    inline const char kPlatformSep = '\\';
#else
    inline const char kPlatformSep = '/';
#endif

/*************** debug settings ***************/

// Enable this may help you find bugs
#define DEBUG_EXTRA_CHECK           0

// Do not edit the following settings unless you know what you are doing
#define DEBUG_NO_BUILTIN_MODULES    0
#define DEBUG_DIS_EXEC              0
#define DEBUG_CEVAL_STEP            0
#define DEBUG_FULL_EXCEPTION        0
#define DEBUG_MEMORY_POOL           0
#define DEBUG_NO_MEMORY_POOL        0
#define DEBUG_NO_AUTO_GC            0
#define DEBUG_GC_STATS              0

#endif