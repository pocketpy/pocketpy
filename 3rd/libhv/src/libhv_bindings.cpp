#include "libhv_bindings.hpp"

extern "C" void pk__add_module_libhv() {
    py_GlobalRef mod = py_newmodule("libhv");

    libhv_register_HttpClient(mod);
    libhv_register_HttpServer(mod);
    libhv_register_WebSocketClient(mod);
    libhv_register_WebSocketServer(mod);
}
