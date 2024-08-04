#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/common/sstream.h"
#include "pocketpy/interpreter/vm.h"

#if _WIN32
#include <direct.h>

int platform_chdir(const char* path) { return _chdir(path); }

bool platform_getcwd(char* buf, size_t size) { return _getcwd(buf, size) != NULL; }

#elif __linux__
#include <unistd.h>

int platform_chdir(const char* path) { return chdir(path); }

bool platform_getcwd(char* buf, size_t size) { return getcwd(buf, size) != NULL; }
#else

int platform_chdir(const char* path) { return -1; }

bool platform_getcwd(char* buf, size_t size) { return false; }

#endif

static bool os_chdir(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_str);
    const char* path = py_tostr(py_arg(0));
    int code = platform_chdir(path);
    if(code != 0) return OSError("chdir() failed: %d", code);
    return true;
}

static bool os_getcwd(int argc, py_Ref argv) {
    char buf[1024];
    if(!platform_getcwd(buf, sizeof(buf))) return OSError("getcwd() failed");
    py_newstr(py_retval(), buf);
    return true;
}

void pk__add_module_os() {
    py_Ref mod = py_newmodule("os");
    py_bindfunc(mod, "chdir", os_chdir);
    py_bindfunc(mod, "getcwd", os_getcwd);
}