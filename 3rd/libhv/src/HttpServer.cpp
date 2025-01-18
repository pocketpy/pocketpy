#include "libhv_bindings.hpp"
#include "http/server/HttpServer.h"

py_Type libhv_register_HttpServer(py_GlobalRef mod){
    py_Type type = py_newtype("HttpServer", tp_object, mod, NULL);
    return type;
}