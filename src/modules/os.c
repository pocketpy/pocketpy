#include "pocketpy/objects/base.h"
#include "pocketpy/pocketpy.h"
#include "pocketpy/interpreter/vm.h"
#include <errno.h>

#if PY_SYS_PLATFORM == 0
#include <direct.h>

int platform_chdir(const char* path) { return _chdir(path); }

bool platform_getcwd(char* buf, size_t size) { return _getcwd(buf, size) != NULL; }

#elif PY_SYS_PLATFORM == 3 || PY_SYS_PLATFORM == 5
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
    if(code != 0) return py_exception(tp_OSError, "chdir() failed: %d", code);
    py_newnone(py_retval());
    return true;
}

static bool os_getcwd(int argc, py_Ref argv) {
    char buf[1024];
    if(!platform_getcwd(buf, sizeof(buf))) return py_exception(tp_OSError, "getcwd() failed");
    py_newstr(py_retval(), buf);
    return true;
}

static bool os_system(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_str);
#if PK_IS_DESKTOP_PLATFORM
    const char* cmd = py_tostr(py_arg(0));
    int code = system(cmd);
    py_newint(py_retval(), code);
    return true;
#else
    return py_exception(tp_OSError, "system() is not supported on this platform");
#endif
}

void pk__add_module_os() {
    py_Ref mod = py_newmodule("os");
    py_bindfunc(mod, "chdir", os_chdir);
    py_bindfunc(mod, "getcwd", os_getcwd);
    py_bindfunc(mod, "system", os_system);
}

void pk__add_module_sys() {
    py_Ref mod = py_newmodule("sys");
    py_newstr(py_emplacedict(mod, py_name("platform")), PY_SYS_PLATFORM_STRING);
    py_newstr(py_emplacedict(mod, py_name("version")), PK_VERSION);
    py_newlist(py_emplacedict(mod, py_name("argv")));
}

typedef struct {
    const char* path;
    const char* mode;
    FILE* file;
} io_FileIO;

static bool io_FileIO__new__(int argc, py_Ref argv) {
    // __new__(cls, file, mode)
    PY_CHECK_ARGC(3);
    PY_CHECK_ARG_TYPE(1, tp_str);
    PY_CHECK_ARG_TYPE(2, tp_str);
    py_Type cls = py_totype(argv);
    io_FileIO* ud = py_newobject(py_retval(), cls, 0, sizeof(io_FileIO));
    ud->path = py_tostr(py_arg(1));
    ud->mode = py_tostr(py_arg(2));
    ud->file = fopen(ud->path, ud->mode);
    if(ud->file == NULL) {
        const char* msg = strerror(errno);
        return IOError("[Errno %d] %s: %s", errno, msg, ud->path);
    }
    return true;
}

static bool io_FileIO__enter__(int argc, py_Ref argv) {
    py_assign(py_retval(), py_arg(0));
    return true;
}

static bool io_FileIO__exit__(int argc, py_Ref argv) {
    io_FileIO* ud = py_touserdata(py_arg(0));
    if(ud->file != NULL) {
        fclose(ud->file);
        ud->file = NULL;
    }
    return true;
}

static bool io_FileIO_read(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    io_FileIO* ud = py_touserdata(py_arg(0));
    fseek(ud->file, 0, SEEK_END);
    int filesize = ftell(ud->file);
    fseek(ud->file, 0, SEEK_SET);
    unsigned char* data = py_newbytes(py_retval(), filesize);
    fread(data, 1, filesize, ud->file);
    return true;
}

static bool io_FileIO_write(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(1, tp_bytes);
    io_FileIO* ud = py_touserdata(py_arg(0));
    int filesize;
    unsigned char* data = py_tobytes(py_arg(1), &filesize);
    fwrite(data, 1, filesize, ud->file);
    return true;
}

void pk__add_module_io() {
    py_Ref mod = py_newmodule("io");

    py_Type FileIO = pk_newtype("FileIO", tp_object, mod, NULL, false, true);

    py_bindmagic(FileIO, __new__, io_FileIO__new__);
    py_bindmagic(FileIO, __enter__, io_FileIO__enter__);
    py_bindmagic(FileIO, __exit__, io_FileIO__exit__);
    py_bindmethod(FileIO, "read", io_FileIO_read);
    py_bindmethod(FileIO, "write", io_FileIO_write);

    py_setdict(&pk_current_vm->builtins, py_name("open"), py_tpobject(FileIO));
}
