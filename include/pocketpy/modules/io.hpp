#pragma once

#include "pocketpy/common/types.hpp"

namespace pkpy {
unsigned char* _default_import_handler(const char*, int*);
void add_module_os(VM* vm);
void add_module_io(VM* vm);
}  // namespace pkpy
