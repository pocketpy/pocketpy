#include "libhv_bindings.hpp"
#include "http/client/WebSocketClient.h"

py_Type libhv_register_WebSocketClient(py_GlobalRef mod) {
    py_Type type = py_newtype("WebSocketClient", tp_object, mod, NULL);
    return type;
}