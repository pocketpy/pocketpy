#include "pocketpy/pocketpy.h"

#if defined(_WIN32) && defined(PK_MODULE_WIN32)

#include <windows.h>
#include <conio.h>

static bool win32__kbhit(int argc, py_Ref argv) {
    PY_CHECK_ARGC(0);
    int ret = _kbhit();
    py_newint(py_retval(), ret);
    return true;
}

static bool win32__getch(int argc, py_Ref argv) {
    PY_CHECK_ARGC(0);
    int ret = _getch();
    py_newint(py_retval(), ret);
    return true;
}

static bool win32_PlaySoundA(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    PY_CHECK_ARG_TYPE(0, tp_str);
    PY_CHECK_ARG_TYPE(1, tp_int);
    PY_CHECK_ARG_TYPE(2, tp_int);
    const char* pszSound = py_tostr(argv);
    py_i64 hmod = py_toint(py_arg(1));
    py_i64 fdwSound = py_toint(py_arg(2));
    int ret = PlaySoundA(pszSound, (HMODULE)hmod, fdwSound);
    py_newbool(py_retval(), ret);
    return true;
}

#endif


void pk__add_module_win32() {
#if defined(_WIN32) && defined(PK_MODULE_WIN32)
    py_Ref mod = py_newmodule("win32");

    py_bindfunc(mod, "_kbhit", win32__kbhit);
    py_bindfunc(mod, "_getch", win32__getch);

    py_bindfunc(mod, "PlaySoundA", win32_PlaySoundA);
#endif
}
