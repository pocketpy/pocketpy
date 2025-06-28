#pragma once
#include "pocketpy/export.h"
PK_API void wait_for_debugger(const char* hostname, unsigned short port);
PK_API void c11_dap_send_exited_event(int exitCode);