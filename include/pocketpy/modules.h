#pragma once

#include "bindings.h"

namespace pkpy{

void add_module_operator(VM* vm);
void add_module_time(VM* vm);
void add_module_sys(VM* vm);
void add_module_json(VM* vm);
void add_module_math(VM* vm);
void add_module_traceback(VM* vm);
void add_module_dis(VM* vm);
void add_module_gc(VM* vm);
void add_module_line_profiler(VM* vm);
void add_module_enum(VM* vm);
void add_module___builtins(VM* vm);

}   // namespace pkpy