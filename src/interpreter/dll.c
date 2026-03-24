#include "pocketpy/pocketpy.h"

#if PK_IS_DESKTOP_PLATFORM && PK_ENABLE_OS && PK_ENABLE_DLL

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

#else
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#endif

typedef bool (*py_module_initialize_t)() PY_RAISE PY_RETURN;

int load_module_from_dll_desktop_only(const char* path) PY_RAISE PY_RETURN {
    const char* f_init_name = "py_module_initialize";
#ifdef _WIN32
    void* dll = LoadLibraryA(path);
    if(dll == NULL) return 0;
    py_module_initialize_t f_init = (py_module_initialize_t)GetProcAddress(dll, f_init_name);
#else
    void* dll = NULL;
    // On Linux, dlopen doesn't automatically add .so suffix like Windows does with .dll
    // Also, CMake typically generates libXxx.so instead of Xxx.so
    // Try: path.so, libpath.so, then the original path
    char* path_with_so = NULL;
    char* path_with_lib = NULL;
    size_t path_len = strlen(path);
    
    // Try path.so
    path_with_so = py_malloc(path_len + 4); // .so + null terminator
    if(path_with_so != NULL) {
        strcpy(path_with_so, path);
        strcat(path_with_so, ".so");
        dll = dlopen(path_with_so, RTLD_LAZY);
        py_free(path_with_so);
    }
    
    // Try libpath.so if path.so didn't work
    if(dll == NULL) {
        path_with_lib = py_malloc(path_len + 7); // lib + .so + null terminator
        if(path_with_lib != NULL) {
            strcpy(path_with_lib, "lib");
            strcat(path_with_lib, path);
            strcat(path_with_lib, ".so");
            dll = dlopen(path_with_lib, RTLD_LAZY);
            py_free(path_with_lib);
        }
    }
    
    // Fallback to original path
    if(dll == NULL) {
        dll = dlopen(path, RTLD_LAZY);
    }
    
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