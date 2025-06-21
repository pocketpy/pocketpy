#include "pocketpy/export.h"
#include "pocketpy/pocketpy.h"
PK_API void wait_for_debugger();
void c11_dap_tracefunc(py_Frame* frame, enum py_TraceEvent event);
void c11_dap_configureDebugger();