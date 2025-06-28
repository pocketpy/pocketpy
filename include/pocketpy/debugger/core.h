#pragma once

#include "pocketpy/common/sstream.h"
#include "pocketpy/common/str.h"
#include "pocketpy/common/vector.h"
#include "pocketpy/pocketpy.h"

enum C11_STEP_MODE { C11_STEP_IN, C11_STEP_OVER, C11_STEP_OUT, C11_STEP_CONTINUE };

void c11_debugger_init(void);
void c11_debugger_set_step_mode(enum C11_STEP_MODE mode);
void c11_debugger_on_trace(py_Frame* frame, enum py_TraceEvent event);
void c11_debugger_frames(c11_sbuf* buffer);
void c11_debugger_scopes(int frameid, c11_sbuf* buffer);
void c11_debugger_set_work_directort(const char* path);
bool c11_debugger_unfold_var(int var_id, c11_sbuf* buffer);
int c11_debugger_setbreakpoint(const char* filename, int lineno);
int c11_debugger_reset_breakpoints_by_source(const char* sourcesname);
int c11_debugger_should_pause(void);
int c11_debugger_should_keep_pause(void);
