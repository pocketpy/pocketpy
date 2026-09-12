#pragma once

#include "pocketpy/pocketpy.h"

#if PK_ENABLE_OS

void dap_waitforattach(const char* hostname, unsigned short port);
int dap_status();
void dap_exceptionbreakpoint(py_Ref exc);
void dap_exit(int code);

#endif  // PK_ENABLE_OS
