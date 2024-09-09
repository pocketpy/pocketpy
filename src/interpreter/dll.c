#include "pocketpy/pocketpy.h"

#if PK_IS_DESKTOP_PLATFORM

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

typedef bool (*py_module_initialize_t)() PY_RAISE PY_RETURN;

int load_module_from_dll_desktop_only(const char* path) PY_RAISE PY_RETURN {
    const char* f_init_name = "py_module_initialize";
#ifdef _WIN32
    HMODULE dll = LoadLibraryA(path);
    if(dll == NULL) return 0;
    py_module_initialize_t f_init = (py_module_initialize_t)GetProcAddress(dll, init_func);
#else
    void* dll = dlopen(path, RTLD_LAZY);
    if(dll == NULL) return 0;
    py_module_initialize_t f_init = (py_module_initialize_t)dlsym(dll, f_init_name);
#endif
    if(f_init == NULL) return 0;
    bool success = f_init();
    if(!success) return -1;
    return 1;
}

#else

int load_module_from_dll_desktop_only(const char* path) PY_RAISE PY_RETURN {
    return 0;
}

#endif