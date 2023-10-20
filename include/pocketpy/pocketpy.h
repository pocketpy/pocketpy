#pragma once

#include "compiler.h"
#include "obj.h"
#include "repl.h"
#include "iter.h"
#include "base64.h"
#include "cffi.h"
#include "linalg.h"
#include "easing.h"
#include "io.h"
#include "vm.h"
#include "re.h"
#include "random.h"
#include "bindings.h"
#include "collections.h"

namespace pkpy {

void init_builtins(VM* _vm);

void add_module_timeit(VM* vm);
void add_module_time(VM* vm);
void add_module_sys(VM* vm);
void add_module_json(VM* vm);

void add_module_math(VM* vm);
void add_module_dis(VM* vm);
void add_module_traceback(VM* vm);
void add_module_gc(VM* vm);
}   // namespace pkpy