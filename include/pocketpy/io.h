#pragma once

#include "cffi.h"

namespace pkpy{
    Bytes _default_import_handler(const Str& name);
    void add_module_os(VM* vm);
    void add_module_io(VM* vm);
}

#if PK_ENABLE_OS

#include <filesystem>
#include <cstdio>

namespace pkpy{

struct FileIO {
    PY_CLASS(FileIO, io, FileIO)

    Str file;
    Str mode;
    FILE* fp;

    bool is_text() const { return mode != "rb" && mode != "wb" && mode != "ab"; }
    FileIO(VM* vm, std::string file, std::string mode);
    void close();
    static void _register(VM* vm, PyObject* mod, PyObject* type);
};

#endif

} // namespace pkpy