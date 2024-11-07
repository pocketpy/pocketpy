#include "pocketpy/pocketpy.h"

#if PK_IS_DESKTOP_PLATFORM && PK_ENABLE_OS

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#else
#include <dlfcn.h>
#endif

typedef bool (*py_module_initialize_t)() PY_RAISE PY_RETURN;

int load_module_from_dll_desktop_only(const char* path) PY_RAISE PY_RETURN {
    const char* f_init_name = "py_module_initialize";
#ifdef _WIN32
    void* dll = LoadLibraryA(path);
    if(dll == NULL) return 0;
    py_module_initialize_t f_init = (py_module_initialize_t)GetProcAddress(dll, f_init_name);
#else
    void* dll = dlopen(path, RTLD_LAZY);
    if(dll == NULL) return 0;
    py_module_initialize_t f_init = (py_module_initialize_t)dlsym(dll, f_init_name);
#endif
    if(f_init == NULL) {
        RuntimeError("%s() not found in '%s'", f_init_name, path);
        return -1;
    }
    bool success = f_init();
    if(!success) return -1;
    return 1;
}

#else

int load_module_from_dll_desktop_only(const char* path) PY_RAISE PY_RETURN {
    return 0;
}

#endif