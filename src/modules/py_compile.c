#include "pocketpy/pocketpy.h"
#include "pocketpy/interpreter/vm.h"
#include <errno.h>

static bool py_compile_compile(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(0, tp_str);
    PY_CHECK_ARG_TYPE(1, tp_str);
    const char* src_path = py_tostr(py_arg(0));
    const char* dst_path = py_tostr(py_arg(1));
    // read
    FILE* fp = fopen(src_path, "rb");
    if(fp == NULL) {
        const char* msg = strerror(errno);
        return OSError("[Errno %d] %s: '%s'", errno, msg, src_path);
    }
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char* buffer = PK_MALLOC(size + 1);
    size = fread(buffer, 1, size, fp);
    buffer[size] = 0;
    fclose(fp);
    // compile
    bool ok = py_compile(buffer, src_path, EXEC_MODE, false);
    PK_FREE(buffer);
    if(!ok) return false;
    // dump
    py_assign(py_pushtmp(), py_retval());
    int bc_size;
    void* bc_data = CodeObject__dumps(py_touserdata(py_peek(-1)), &bc_size);
    py_pop();
    // write
    fp = fopen(dst_path, "wb");
    if(fp == NULL) {
        PK_FREE(bc_data);
        const char* msg = strerror(errno);
        return OSError("[Errno %d] %s: '%s'", errno, msg, dst_path);
    }
    fwrite(bc_data, 1, bc_size, fp);
    fclose(fp);
    PK_FREE(bc_data);
    py_newnone(py_retval());
    return true;
}

void pk__add_module_py_compile() {
    py_Ref mod = py_newmodule("py_compile");

    py_bindfunc(mod, "compile", py_compile_compile);
}