#pragma once

#include "cffi.h"

namespace pkpy{
    unsigned char* _default_import_handler(const char*, int, int*);
    void add_module_os(VM* vm);
    void add_module_io(VM* vm);
}
