#include "pocketpy/objects/base.h"
#include "pocketpy/pocketpy.h"
#include "pocketpy/interpreter/vm.h"

#if PK_ENABLE_OS

#include <errno.h>

#if PY_SYS_PLATFORM == 0
#include <direct.h>
#include <io.h>

int platform_chdir(const char* path) { return _chdir(path); }

bool platform_getcwd(char* buf, size_t size) { return _getcwd(buf, size) != NULL; }

bool platform_path_exists(const char* path) { return _access(path, 0) == 0; }

#elif PY_SYS_PLATFORM == 3 || PY_SYS_PLATFORM == 5
#include <unistd.h>

int platform_chdir(const char* path) { return chdir(path); }

bool platform_getcwd(char* buf, size_t size) { return getcwd(buf, size) != NULL; }

bool platform_path_exists(const char* path) { return access(path, F_OK) == 0; }
#else

int platform_chdir(const char* path) { return -1; }

bool platform_getcwd(char* buf, size_t size) { return false; }

bool platform_path_exists(const char* path) { return false; }
#endif

static bool os_chdir(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_str);
    const char* path = py_tostr(py_arg(0));
    int code = platform_chdir(path);
    if(code != 0) {
        const char* msg = strerror(errno);
        return OSError("[Errno %d] %s: '%s'", errno, msg, path);
    }
    py_newnone(py_retval());
    return true;
}

static bool os_getcwd(int argc, py_Ref argv) {
    char buf[1024];
    if(!platform_getcwd(buf, sizeof(buf))) return OSError("getcwd() failed");
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
    return OSError("system() is not supported on this platform");
#endif
}

static bool os_remove(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_str);
    const char* path = py_tostr(py_arg(0));
    int code = remove(path);
    if(code != 0) {
        const char* msg = strerror(errno);
        return OSError("[Errno %d] %s: '%s'", errno, msg, path);
    }
    py_newnone(py_retval());
    return true;
}

static bool os_path_exists(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_str);
    const char* path = py_tostr(py_arg(0));
    py_newbool(py_retval(), platform_path_exists(path));
    return true;
}

void pk__add_module_os() {
    py_Ref mod = py_newmodule("os");
    py_bindfunc(mod, "chdir", os_chdir);
    py_bindfunc(mod, "getcwd", os_getcwd);
    py_bindfunc(mod, "system", os_system);
    py_bindfunc(mod, "remove", os_remove);

    py_ItemRef path_object = py_emplacedict(mod, py_name("path"));
    py_newobject(path_object, tp_object, -1, 0);
    py_bindfunc(path_object, "exists", os_path_exists);

    py_newdict(py_emplacedict(mod, py_name("environ")));
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
        return OSError("[Errno %d] %s: '%s'", errno, msg, ud->path);
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
    py_newnone(py_retval());
    return true;
}

static bool io_FileIO_read(int argc, py_Ref argv) {
    io_FileIO* ud = py_touserdata(py_arg(0));
    bool is_binary = ud->mode[strlen(ud->mode) - 1] == 'b';
    int size;
    if(argc == 1) {
        long current = ftell(ud->file);
        fseek(ud->file, 0, SEEK_END);
        size = ftell(ud->file);
        fseek(ud->file, current, SEEK_SET);
    } else if(argc == 2) {
        PY_CHECK_ARG_TYPE(1, tp_int);
        size = py_toint(py_arg(1));
    } else {
        return TypeError("read() takes at most 2 arguments (%d given)", argc);
    }
    if(is_binary) {
        void* dst = py_newbytes(py_retval(), size);
        int actual_size = fread(dst, 1, size, ud->file);
        py_bytes_resize(py_retval(), actual_size);
    } else {
        void* dst = PK_MALLOC(size);
        int actual_size = fread(dst, 1, size, ud->file);
        py_newstrv(py_retval(), (c11_sv){dst, actual_size});
        PK_FREE(dst);
    }
    return true;
}

static bool io_FileIO_tell(int argc, py_Ref argv) {
    io_FileIO* ud = py_touserdata(py_arg(0));
    py_newint(py_retval(), ftell(ud->file));
    return true;
}

static bool io_FileIO_seek(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    PY_CHECK_ARG_TYPE(1, tp_int);
    PY_CHECK_ARG_TYPE(2, tp_int);
    io_FileIO* ud = py_touserdata(py_arg(0));
    long cookie = py_toint(py_arg(1));
    int whence = py_toint(py_arg(2));
    py_newint(py_retval(), fseek(ud->file, cookie, whence));
    return true;
}

static bool io_FileIO_close(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    io_FileIO* ud = py_touserdata(py_arg(0));
    if(ud->file != NULL) {
        fclose(ud->file);
        ud->file = NULL;
    }
    py_newnone(py_retval());
    return true;
}

static bool io_FileIO_write(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    io_FileIO* ud = py_touserdata(py_arg(0));
    size_t written_size;
    if(ud->mode[strlen(ud->mode) - 1] == 'b') {
        PY_CHECK_ARG_TYPE(1, tp_bytes);
        int filesize;
        unsigned char* data = py_tobytes(py_arg(1), &filesize);
        written_size = fwrite(data, 1, filesize, ud->file);
    } else {
        PY_CHECK_ARG_TYPE(1, tp_str);
        c11_sv sv = py_tosv(py_arg(1));
        written_size = fwrite(sv.data, 1, sv.size, ud->file);
    }
    py_newint(py_retval(), written_size);
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
    py_bindmethod(FileIO, "close", io_FileIO_close);
    py_bindmethod(FileIO, "tell", io_FileIO_tell);
    py_bindmethod(FileIO, "seek", io_FileIO_seek);

    py_newint(py_emplacedict(mod, py_name("SEEK_SET")), SEEK_SET);
    py_newint(py_emplacedict(mod, py_name("SEEK_CUR")), SEEK_CUR);
    py_newint(py_emplacedict(mod, py_name("SEEK_END")), SEEK_END);

    py_setdict(pk_current_vm->builtins, py_name("open"), py_tpobject(FileIO));
}

#else

void pk__add_module_os() {}

void pk__add_module_io() {}

#endif

static bool sys_setrecursionlimit(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_int);
    int limit = py_toint(py_arg(0));
    if(limit <= pk_current_vm->recursion_depth) return ValueError("the limit is too low");
    pk_current_vm->max_recursion_depth = limit;
    py_newnone(py_retval());
    return true;
}

static bool sys_getrecursionlimit(int argc, py_Ref argv) {
    PY_CHECK_ARGC(0);
    py_newint(py_retval(), pk_current_vm->max_recursion_depth);
    return true;
}

void pk__add_module_sys() {
    py_Ref mod = py_newmodule("sys");
    py_newstr(py_emplacedict(mod, py_name("platform")), PY_SYS_PLATFORM_STRING);
    py_newstr(py_emplacedict(mod, py_name("version")), PK_VERSION);
    py_newlist(py_emplacedict(mod, py_name("argv")));

    py_bindfunc(mod, "setrecursionlimit", sys_setrecursionlimit);
    py_bindfunc(mod, "getrecursionlimit", sys_getrecursionlimit);
}
