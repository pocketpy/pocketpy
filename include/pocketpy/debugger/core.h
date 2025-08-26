#pragma once

#include "pocketpy/common/sstream.h"
#include "pocketpy/common/str.h"
#include "pocketpy/common/vector.h"
#include "pocketpy/pocketpy.h"

#if PK_ENABLE_OS

typedef enum { C11_STEP_IN, C11_STEP_OVER, C11_STEP_OUT, C11_STEP_CONTINUE } C11_STEP_MODE;
typedef enum { C11_DEBUGGER_NOSTOP, C11_DEBUGGER_STEP, C11_DEBUGGER_EXCEPTION, C11_DEBUGGER_BP} C11_STOP_REASON;
typedef enum {
    C11_DEBUGGER_SUCCESS = 0,
    C11_DEBUGGER_EXIT = 1,
    C11_DEBUGGER_UNKNOW_ERROR = 3,
    C11_DEBUGGER_FILEPATH_ERROR = 7
} C11_DEBUGGER_STATUS;

void c11_debugger_init(void);
void c11_debugger_set_step_mode(C11_STEP_MODE mode);
void c11_debugger_exception_on_trace(py_Ref exc);
C11_DEBUGGER_STATUS c11_debugger_on_trace(py_Frame* frame, enum py_TraceEvent event);
const char* c11_debugger_excinfo(const char ** message);
void c11_debugger_frames(c11_sbuf* buffer);
void c11_debugger_scopes(int frameid, c11_sbuf* buffer);
bool c11_debugger_unfold_var(int var_id, c11_sbuf* buffer);
int c11_debugger_setbreakpoint(const char* filename, int lineno);
int c11_debugger_reset_breakpoints_by_source(const char* sourcesname);
C11_STOP_REASON c11_debugger_should_pause(void);
int c11_debugger_should_keep_pause(void);

#endif // PK_ENABLE_OS