#define PK_SHARED_MODULE
#include "pocketpy.h"

using namespace pkpy;

extern "C" {
    PK_EXPORT
    PyObject* platform_module__init__(VM* vm, const char* version){
        PyObject* mod = vm->new_module("test");
        vm->_stdout(vm, "Hello from dylib!\n");

        vm->bind(mod, "hello()", [](VM* vm, ArgsView args){
            vm->_stdout(vm, "Hello from dylib!\n");
            return vm->None;
        });
        return mod;
    }

#if _WIN32
BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,  // handle to DLL module
    DWORD fdwReason,     // reason for calling function
    LPVOID lpvReserved )  // reserved
{
    return TRUE;  // Successful DLL_PROCESS_ATTACH.
}
#endif

}