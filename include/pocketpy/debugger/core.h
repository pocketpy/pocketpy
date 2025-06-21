#pragma once

#include "pocketpy/common/vector.h"
#include "pocketpy/pocketpy.h"


typedef struct c11_debugger_frame {
    int id;                   // Unique frame identifier
    const char* module_name;  // Module where this frame resides
    const char* file_name;    // Source file path
    int lineno;              // Current line number in source
} c11_debugger_frame;



// A single variable or composite namespace entry
// Composite variables (e.g., objects, dicts) can have children
typedef struct c11_debugger_variable {
    int var_ref;
    const char* name;
    const char* value;
    const char* type;
} c11_debugger_variable;

// Initialize the debugger (must be called before other APIs)
PK_API void c11_debugger_init(void);

// Trace event callback from the interpreter
PK_API void c11_debugger_on_trace(py_Frame* frame, enum py_TraceEvent event);

// Control execution
PK_API void c11_debugger_stepin(void);
PK_API void c11_debugger_stepout(void);
PK_API void c11_debugger_stepover(void);
PK_API void c11_debugger_continue(void);

// Frame navigation
// Retrieve the call stack (older frames follow .next). Returns head frame, ending with NULL.
PK_API c11_vector c11_debugger_frames();

// Switch active frame by id
PK_API void c11_debugger_switch_frame(int frame_id);
c11_vector c11_debugger_scopes(int frameid);

// Variable inspection and manipulation
// Get a specific variable by name within the current namespace; returns NULL if not found
c11_vector c11_debugger_get_children(int var_id);
// Set a breakpoint at file and line
PK_API void c11_debugger_setbreakpoint(const char* filename, int lineno);

// Query whether execution should pause (e.g. hit breakpoint)
// This function has side effects, so it should only be called when necessary.
PK_API int c11_debugger_should_pause(void);
PK_API int c11_debugger_should_keep_pause(void);












