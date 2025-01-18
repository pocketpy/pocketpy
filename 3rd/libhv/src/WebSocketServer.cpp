#include "libhv_bindings.hpp"
#include "http/server/WebSocketServer.h"

py_Type libhv_register_WebSocketServer(py_GlobalRef mod) {
    py_Type type = py_newtype("WebSocketServer", tp_object, mod, NULL);
    return type;
}
