#pragma once

#include "pocketpy.h"

extern "C" void pk__add_module_libhv();

py_Type libhv_register_HttpClient(py_GlobalRef mod);
py_Type libhv_register_HttpServer(py_GlobalRef mod);
py_Type libhv_register_WebSocketClient(py_GlobalRef mod);
py_Type libhv_register_WebSocketServer(py_GlobalRef mod);
