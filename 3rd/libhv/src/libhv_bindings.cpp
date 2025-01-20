#include "libhv_bindings.hpp"
#include "base/herr.h"

extern "C" void pk__add_module_libhv() {
    py_GlobalRef mod = py_newmodule("libhv");

    libhv_register_HttpRequest(mod);
    libhv_register_HttpClient(mod);
    libhv_register_HttpServer(mod);
    libhv_register_WebSocketClient(mod);

    py_bindfunc(mod, "strerror", [](int argc, py_Ref argv) {
        PY_CHECK_ARGC(1);
        PY_CHECK_ARG_TYPE(0, tp_int);
        int code = py_toint(py_arg(0));
        const char* msg = hv_strerror(code);
        py_newstr(py_retval(), msg);
        return true;
    });
}
