#pragma once

#include "pocketpy/export.h"
#include <stdbool.h>

// Fine-grained capabilities of the VM for sandboxing.
typedef struct py_Capabilities {
    // FileIO
    bool (*file_open)(const char* path, const char* mode);
    // os
    bool (*os_chdir)(const char* path);
    bool (*os_getcwd)();
    bool (*os_system)(const char* command);
    bool (*os_remove)(const char* path);
    // stdc
    bool stdc_write;    // memset, write_bytes, ...
    bool stdc_read;     // memcmp, read_bytes, ...
    bool stdc_malloc;   // malloc
    bool stdc_free;     // free
} py_Capabilities;

/// Setup the capabilities for the current VM.
PK_API py_Capabilities* py_capabilities();
/// Raise a `KeyboardInterrupt` to interrupt the current execution.
PK_API void py_interrupt();
